#include "bookmarks.h"
#include "cstrptr.h"
#include <algorithm>
#include <QTimer>

namespace Fm {

std::weak_ptr<Bookmarks> Bookmarks::globalInstance_;

static inline CStrPtr get_legacy_bookmarks_file(void) {
    return CStrPtr{g_build_filename(g_get_home_dir(), ".gtk-bookmarks", nullptr)};
}

static inline CStrPtr get_new_bookmarks_file(void) {
    return CStrPtr{g_build_filename(g_get_user_config_dir(), "gtk-3.0", "bookmarks", nullptr)};
}

Bookmarks::Bookmarks(QObject* parent):
    QObject(parent),
    idle_handler{false} {

    /* trying the gtk-3.0 first and use it if it exists */
    auto fpath = get_new_bookmarks_file();
    file = FilePath::fromLocalPath(fpath.get());
    load();
    if(items_.empty()) { /* not found, use legacy file */
        fpath = get_legacy_bookmarks_file();
        file = FilePath::fromLocalPath(fpath.get());
        load();
    }
    mon = GObjectPtr<GFileMonitor>{g_file_monitor_file(file.gfile().get(), G_FILE_MONITOR_NONE, nullptr, nullptr), false};
    if(mon) {
        g_signal_connect(mon.get(), "changed", G_CALLBACK(_onFileChanged), this);
    }
}

Bookmarks::~Bookmarks() {
    if(mon) {
        g_signal_handlers_disconnect_by_data(mon.get(), this);
    }
}

const std::shared_ptr<const BookmarkItem>& Bookmarks::insert(const FilePath& path, const QString& name, int pos) {
    const auto insert_pos = (pos < 0 || static_cast<size_t>(pos) > items_.size()) ? items_.cend() : items_.cbegin() + pos;
    auto it = items_.insert(insert_pos, std::make_shared<const BookmarkItem>(path, name));
    queueSave();
    return *it;
}

void Bookmarks::remove(const std::shared_ptr<const BookmarkItem>& item) {
    items_.erase(std::remove(items_.begin(), items_.end(), item), items_.end());
    queueSave();
}

void Bookmarks::reorder(const std::shared_ptr<const BookmarkItem>& item, int pos) {
    auto old_it = std::find(items_.cbegin(), items_.cend(), item);
    if(old_it == items_.cend())
        return;
    std::shared_ptr<const BookmarkItem> newItem = item;
    auto old_pos = old_it - items_.cbegin();
    items_.erase(old_it);
    if(old_pos < pos)
        --pos;
    auto new_it = items_.cbegin() + pos;
    if(new_it > items_.cend())
        new_it = items_.cend();
    items_.insert(new_it, std::move(newItem));
    queueSave();
}

void Bookmarks::rename(const std::shared_ptr<const BookmarkItem>& item, QString new_name) {
    auto it = std::find_if(items_.cbegin(), items_.cend(), [item](const std::shared_ptr<const BookmarkItem>& elem) {
        return elem->path() == item->path();
    });
    if(it != items_.cend()) {
        // create a new item to replace the old one
        // we do not modify the old item directly since this data structure is shared with others
        it = items_.insert(it, std::make_shared<const BookmarkItem>(item->path(), new_name));
        items_.erase(it + 1); // remove the old item
        queueSave();
    }
}

std::shared_ptr<Bookmarks> Bookmarks::globalInstance() {
    auto bookmarks = globalInstance_.lock();
    if(!bookmarks) {
        bookmarks = std::make_shared<Bookmarks>();
        globalInstance_ = bookmarks;
    }
    return bookmarks;
}

void Bookmarks::save() {
    std::string buf;
    // G_LOCK(bookmarks);
    for(auto& item: items_) {
        auto uri = item->path().uri();
        buf += uri.get();
        buf += ' ';
        buf += item->name().toUtf8().constData();
        buf += '\n';
    }
    idle_handler = false;
    // G_UNLOCK(bookmarks);
    GError* err = nullptr;
    if(!g_file_replace_contents(file.gfile().get(), buf.c_str(), buf.length(), nullptr,
                                FALSE, G_FILE_CREATE_NONE, nullptr, nullptr, &err)) {
        g_critical("%s", err->message);
        g_error_free(err);
    }
    /* we changed bookmarks list, let inform who interested in that */
    Q_EMIT changed();
}

void Bookmarks::load() {
    auto fpath = file.localPath();
    FILE* f;
    char buf[1024];
    /* load the file */
    f = fopen(fpath.get(), "r");
    if(f) {
        while(fgets(buf, 1024, f)) {
            // format of each line in the bookmark file:
            // <URI> <name>\n
            char* sep;
            sep = strchr(buf, '\n');
            if(sep) {
                *sep = '\0';
            }

            QString name;
            sep = strchr(buf, ' ');  // find the separator between URI and name
            if(sep) {
                *sep = '\0';
                name = sep + 1;
            }
            auto uri = buf;
            if(uri[0] != '\0') {
                items_.push_back(std::make_shared<BookmarkItem>(FilePath::fromUri(uri), name));
            }
        }
        fclose(f);
    }
}

void Bookmarks::onFileChanged(GFileMonitor* /*mon*/, GFile* /*gf*/, GFile* /*other*/, GFileMonitorEvent /*evt*/) {
    // reload the bookmarks
    items_.clear();
    load();
    Q_EMIT changed();
}


void Bookmarks::queueSave() {
    if(!idle_handler) {
        QTimer::singleShot(0, this, &Bookmarks::save);
        idle_handler = true;
    }
}


} // namespace Fm
