#include "fileoperationjob.h"

namespace Fm {

FileOperationJob::FileOperationJob():
    hasTotalAmount_{false},
    totalSize_{0},
    totalCount_{0},
    finishedSize_{0},
    finishedCount_{0},
    currentFileSize_{0},
    currentFileFinished_{0} {
}

bool FileOperationJob::totalAmount(uint64_t& fileSize, uint64_t& fileCount) const {
    std::lock_guard<std::mutex> lock{mutex_};
    if(hasTotalAmount_) {
        fileSize = totalSize_;
        fileCount = totalCount_;
    }
    return hasTotalAmount_;
}

bool FileOperationJob::currentFileProgress(FilePath& path, uint64_t& totalSize, uint64_t& finishedSize) const {
    std::lock_guard<std::mutex> lock{mutex_};
    if(currentFile_.isValid()) {
        path = currentFile_;
        totalSize = currentFileSize_;
        finishedSize = currentFileFinished_;
    }
    return currentFile_.isValid();
}

FileOperationJob::FileExistsAction FileOperationJob::askRename(const FileInfo &src, const FileInfo &dest, FilePath &newDest) {
    FileExistsAction action = SKIP;
    Q_EMIT fileExists(src, dest, action, newDest);
    return action;
}

bool FileOperationJob::finishedAmount(uint64_t& finishedSize, uint64_t& finishedCount) const {
    std::lock_guard<std::mutex> lock{mutex_};
    if(hasTotalAmount_) {
        finishedSize = finishedSize_;
        finishedCount = finishedCount_;
    }
    return hasTotalAmount_;
}

void FileOperationJob::setTotalAmount(uint64_t fileSize, uint64_t fileCount) {
    std::lock_guard<std::mutex> locl{mutex_};
    hasTotalAmount_ = true;
    totalSize_ = fileSize;
    totalCount_ = fileCount;
}

void FileOperationJob::setFinishedAmount(uint64_t finishedSize, uint64_t finishedCount) {
    std::lock_guard<std::mutex> locl{mutex_};
    finishedSize_ = finishedSize;
    finishedCount_ = finishedCount;
}

void FileOperationJob::addFinishedAmount(uint64_t finishedSize, uint64_t finishedCount) {
    std::lock_guard<std::mutex> locl{mutex_};
    finishedSize_ += finishedSize;
    finishedCount_ += finishedCount;
}

void FileOperationJob::setCurrentFile(const FilePath& path) {
    std::lock_guard<std::mutex> locl{mutex_};
    currentFile_ = path;
}

void FileOperationJob::setCurrentFileProgress(uint64_t totalSize, uint64_t finishedSize) {
    std::lock_guard<std::mutex> locl{mutex_};
    currentFileSize_ = totalSize;
    currentFileFinished_ = finishedSize;
}

} // namespace Fm
