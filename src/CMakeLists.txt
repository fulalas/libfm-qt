set(libfm_core_SRCS
    # core data structures
    core/gobjectptr.h
    core/filepath.cpp
    core/iconinfo.cpp
    core/mimetype.cpp
    core/fileinfo.cpp
    core/folder.cpp
    core/filemonitor.cpp
    # i/o jobs
    core/job.cpp
    core/copyjob.cpp
    core/deletejob.cpp
    core/dirlistjob.cpp
    core/filechangeattrjob.cpp
    core/fileinfojob.cpp
    core/filelinkjob.cpp
    core/fileoperationjob.cpp
    core/filesysteminfojob.cpp
    core/job.cpp
    core/totalsizejob.cpp
    core/trashjob.cpp
    core/untrashjob.cpp
    core/thumbnailjob.cpp
    # extra desktop services
    core/bookmarks.cpp
    core/volumemanager.cpp
    core/userinfocache.cpp
    core/thumbnailer.cpp
    core/terminal.cpp
    # custom actions
    customactions/fileaction.cpp
    customactions/fileactionprofile.cpp
    customactions/fileactioncondition.cpp
)

set(libfm_SRCS
    ${libfm_core_SRCS}
    libfmqt.cpp
    bookmarkaction.cpp
    sidepane.cpp
    icontheme.cpp
    filelauncher.cpp
    foldermodel.cpp
    foldermodelitem.cpp
    cachedfoldermodel.cpp
    proxyfoldermodel.cpp
    folderview.cpp
    folderitemdelegate.cpp
    createnewmenu.cpp
    filemenu.cpp
    foldermenu.cpp
    filepropsdialog.cpp
    applaunchcontext.cpp
    placesview.cpp
    placesmodel.cpp
    placesmodelitem.cpp
    dirtreeview.cpp
    dirtreemodel.cpp
    dirtreemodelitem.cpp
    dnddest.cpp
    mountoperation.cpp
    mountoperationpassworddialog.cpp
    mountoperationquestiondialog.cpp
    fileoperation.cpp
    fileoperationdialog.cpp
    renamedialog.cpp
    pathedit.cpp
    pathbar.cpp
    colorbutton.cpp
    fontbutton.cpp
    browsehistory.cpp
    utilities.cpp
    dndactionmenu.cpp
    editbookmarksdialog.cpp
    execfiledialog.cpp
    appchoosercombobox.cpp
    appmenuview.cpp
    appchooserdialog.cpp
    filesearchdialog.cpp
    fm-search.c # might be moved to libfm later
    xdndworkaround.cpp
)

set(libfm_UIS
    file-props.ui
    file-operation-dialog.ui
    rename-dialog.ui
    mount-operation-password.ui
    edit-bookmarks.ui
    exec-file.ui
    app-chooser-dialog.ui
    filesearch.ui
)

qt5_wrap_ui(libfm_UIS_H ${libfm_UIS})


set(LIBFM_QT_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/libfm-qt")
set(LIBFM_QT_INTREE_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/include")

# add translation for libfm-qt
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS ${UPDATE_TRANSLATIONS}
    SOURCES ${libfm_SRCS} ${libfm_UIS}
    INSTALL_DIR "${LIBFM_QT_DATA_DIR}/translations"
    PULL_TRANSLATIONS ${PULL_TRANSLATIONS}
    CLEAN_TRANSLATIONS ${CLEAN_TRANSLATIONS}
    TRANSLATIONS_REPO ${TRANSLATIONS_REPO}
    TRANSLATIONS_REFSPEC ${TRANSLATIONS_REFSPEC}
)

add_library(${LIBFM_QT_LIBRARY_NAME} SHARED
    ${libfm_SRCS}
    ${libfm_UIS_H}
    ${QM_FILES}
)

install(EXPORT
    "${LIBFM_QT_LIBRARY_NAME}-targets"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${LIBFM_QT_LIBRARY_NAME}"
    COMPONENT Devel
)

target_link_libraries(${LIBFM_QT_LIBRARY_NAME}
    Qt5::Widgets
    Qt5::X11Extras
    ${FM_LIBRARIES}
    ${MENUCACHE_LIBRARIES}
    ${XCB_LIBRARIES}
    ${EXIF_LIBRARIES}
)

# set libtool soname
set_target_properties(${LIBFM_QT_LIBRARY_NAME} PROPERTIES
    VERSION ${LIBFM_QT_LIB_VERSION}
    SOVERSION ${LIBFM_QT_LIB_SOVERSION}
)

target_include_directories(${LIBFM_QT_LIBRARY_NAME}
    PRIVATE "${Qt5Gui_PRIVATE_INCLUDE_DIRS}"
    PUBLIC
        "${FM_INCLUDE_DIRS}"
        "${FM_INCLUDE_DIR}/libfm" # to workaround incorrect #include in fm-actions.
        "${MENUCACHE_INCLUDE_DIRS}"
        "${XCB_INCLUDE_DIRS}"
        "${EXIF_INCLUDE_DIRS}"
    INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${LIBFM_QT_INTREE_INCLUDE_DIR}>"
)

target_compile_definitions(${LIBFM_QT_LIBRARY_NAME}
    PRIVATE "LIBFM_QT_DATA_DIR=\"${LIBFM_QT_DATA_DIR}\""
    PUBLIC "QT_NO_KEYWORDS"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${LIBFM_QT_LIBRARY_NAME}_export.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libfm-qt"
    COMPONENT Devel
)

# install include header files (FIXME: can we make this cleaner? should dir name be versioned?)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libfm-qt"
    COMPONENT Devel
    FILES_MATCHING PATTERN "*.h"
)

generate_export_header(${LIBFM_QT_LIBRARY_NAME}
    EXPORT_MACRO_NAME LIBFM_QT_API
)

# InTree build
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/${LIBFM_QT_LIBRARY_NAME}_export.h
    DESTINATION "${LIBFM_QT_INTREE_INCLUDE_DIR}/libfm-qt"
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION "${LIBFM_QT_INTREE_INCLUDE_DIR}/libfm-qt"
    FILES_MATCHING PATTERN "*.h"
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/fm-qt-config.cmake.in"
    "${CMAKE_BINARY_DIR}/${LIBFM_QT_LIBRARY_NAME}-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${LIBFM_QT_LIBRARY_NAME}"
)

install(FILES
    "${CMAKE_BINARY_DIR}/${LIBFM_QT_LIBRARY_NAME}-config.cmake"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${LIBFM_QT_LIBRARY_NAME}"
    COMPONENT Devel
)

# FIXME: add libtool version to the lib (soname) later.
# FIXME: only export public symbols

install(TARGETS ${LIBFM_QT_LIBRARY_NAME}
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    EXPORT "${LIBFM_QT_LIBRARY_NAME}-targets"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER
    COMPONENT Runtime
)

export(TARGETS ${LIBFM_QT_LIBRARY_NAME}
    FILE "${CMAKE_BINARY_DIR}/${LIBFM_QT_LIBRARY_NAME}-targets.cmake"
    EXPORT_LINK_INTERFACE_LIBRARIES
)

# install a pkgconfig file for libfm-qt
set(REQUIRED_QT "Qt5Widgets >= ${REQUIRED_QT_VERSION} Qt5X11Extras >= ${REQUIRED_QT_VERSION}")
configure_file(libfm-qt.pc.in lib${LIBFM_QT_LIBRARY_NAME}.pc @ONLY)
# FreeBSD loves to install files to different locations
# http://www.freebsd.org/doc/handbook/dirstructure.html
if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lib${LIBFM_QT_LIBRARY_NAME}.pc"
        DESTINATION libdata/pkgconfig
        COMPONENT Devel
    )
else()
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lib${LIBFM_QT_LIBRARY_NAME}.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
        COMPONENT Devel
    )
endif()

# prevent the generated files from being deleted during make cleaner
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)


set(TEST_LIBRARIES
    Qt5::Core
    Qt5::Widgets
    ${FM_LIBRARIES}
    ${LIBFM_QT_LIBRARY_NAME}
)
# some simple test cases
add_executable("test-folder"
    tests/test-folder.cpp
)
target_link_libraries("test-folder" ${TEST_LIBRARIES})

add_executable("test-folderview"
    tests/test-folderview.cpp
)
target_link_libraries("test-folderview" ${TEST_LIBRARIES})

add_executable("test-volumemanager"
    tests/test-volumemanager.cpp
)
target_link_libraries("test-volumemanager" ${TEST_LIBRARIES})

add_executable("test-placesview"
    tests/test-placesview.cpp
)
target_link_libraries("test-placesview" ${TEST_LIBRARIES})

