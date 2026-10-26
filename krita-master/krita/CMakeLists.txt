add_subdirectory( dtd )
add_subdirectory( data )
add_subdirectory( integration )
# Install the application icons following the freedesktop icon theme spec
add_subdirectory( "pics/branding/${BRANDING}" )

if (ANDROID)
    if (QT_MAJOR_VERSION STREQUAL "5")
        set (QtAndroidExtras_INCLUDE_DIRS ${Qt5AndroidExtras_INCLUDE_DIRS})
        include_directories (${QtAndroidExtras_INCLUDE_DIRS})
    endif()
endif()

set(krita_SRCS main.cc)

# Set the application icon on the application
if (NOT APPLE)
	file(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/*-apps-krita.png")
else()
    # XCode below 26 cannot work with .icon format
    if(${XCODE_VERSION} VERSION_GREATER_EQUAL "26.0")
        find_program(XCRUN_EXECUTABLE NAMES xcrun)
        set(KRITA_ASSETS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/krita.icon")
        cmake_path(SET ASSETS_PATH "${CMAKE_CURRENT_BINARY_DIR}/assets")

        set(ASSETS_FILES "${ASSETS_PATH}/Assets.car" "${ASSETS_PATH}/krita.icns")
        add_custom_command(
          OUTPUT ${ASSETS_FILES}
          COMMAND ${CMAKE_COMMAND} ARGS -E make_directory "${ASSETS_PATH}"
          COMMAND ${XCRUN_EXECUTABLE} ARGS actool "${KRITA_ASSETS_ICON}"
            --compile ${ASSETS_PATH}
            --platform macosx --minimum-deployment-target ${MACOS_MINIMUM_VERSION}
            --app-icon krita --output-partial-info-plist "${CMAKE_CURRENT_BINARY_DIR}/krita_icon.plist"
            --output-format human-readable-text
            --standalone-icon-behavior all
          DEPENDS
            "${KRITA_ASSETS_ICON}"
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    else()
        set(ICON_SRCS
             "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/16-apps-krita.png"
             "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/32-apps-krita.png"
             "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/48-apps-krita.png"
             "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/128-apps-krita.png"
             "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/256-apps-krita.png"
             "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/512-apps-krita.png"
             "${CMAKE_CURRENT_SOURCE_DIR}/pics/branding/${BRANDING}/1024-apps-krita.png"
        )
    endif() # XCode version
endif()

if (WIN32)
    # Private headers are needed for main.cc to enable WinTab support
    set_source_files_properties(
        main.cc
        PROPERTIES
        INCLUDE_DIRECTORIES "${Qt${QT_MAJOR_VERSION}Gui_PRIVATE_INCLUDE_DIRS}"
        )
endif()

if (WIN32)
    set(krita_windows_stub_ICONS_SRCS "")
    ecm_add_app_icon(krita_windows_stub_ICONS_SRCS ICONS ${ICON_SRCS} OUTFILE_BASENAME krita)

    # This is used to generate an ICO file for the file icon, We don't actually
    # want to embed this icon into an EXE.
	file(GLOB krafile_ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/*-mimetypes-application-x-krita.png")
    set(krafile_dummy_ICONS_SRCS "")
    ecm_add_app_icon(krafile_dummy_ICONS_SRCS ICONS ${krafile_ICON_SRCS} OUTFILE_BASENAME kritafile)
    add_library(krafile_dummy_obj OBJECT ${krafile_dummy_ICONS_SRCS})

    # These icons are used to replace those in the shell extension package.
    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/krita.ico
            ${CMAKE_CURRENT_BINARY_DIR}/kritafile.ico
        DESTINATION ${CMAKE_INSTALL_PREFIX}/installer
    )
elseif(APPLE)
    if(${XCODE_VERSION} VERSION_GREATER_EQUAL "26.0")
      set_source_files_properties(${ASSETS_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    else()
      ecm_add_app_icon(krita_SRCS OUTFILE_BASENAME krita ICONS ${ICON_SRCS})
    endif()
else()
    ecm_add_app_icon(krita_SRCS ICONS ${ICON_SRCS})
endif()

# Install the mimetype icons
ecm_install_icons(ICONS
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/16-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/22-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/32-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/48-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/64-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/128-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/256-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/512-mimetypes-application-x-krita.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/pics/mimetypes/1024-mimetypes-application-x-krita.png"
    DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)

# Android has a different splash screen.
if(ANDROID)
    set(krita_splash_qrc splash-android.qrc)
else()
    set(krita_splash_qrc splash.qrc)
endif()

# ==============================================================================
# KRITA5.XMLGUI UPDATE
#
# The krita5.xmlgui file needs to have its version number updated when it is
# modified. Failing to do so will cause users who have customized their toolbars
# to not see the changes, since that equips them with a local krita5.xmlgui file
# in their application data directory.
#
# On the other hand, if you update the file version, some customizations may
# get lost. If you have changes that are only relevant for certain operating
# systems, you can choose to only bump that system's version here to limit the
# potential blast radius.
#

# Update the version number here. If you have OS-specific changes, put them
# inside of a condition to only bump the version number on that system. If you
# make a change that applies to all systems, remove any conditions. The version
# you specify should always be one higher than any version number you see here,
# no matter what you're doing. Do not re-use versions.
set(KRITA5_XMLGUI_VERSION 546)

# After updating the version number properly, put the new hash that the CMake
# error lists as the "actual hash" here.
set(krita5_xmlgui_hash_expected "bced997671ef3bc6862acdeb87defd96")

# You can stop reading now, the stuff that follows doesn't need to be modified.
# ==============================================================================

# Read the krita5.xmlgui file, strip all whitespace from it and then make an
# hash from it. This will also strip whitespace inside of the XML and MD5 isn't
# a great hash, but that's fine to catch any changes a developer actually makes,
# we're not trying to build an enterprise file change detection system here.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/krita5.xmlgui" krita5_xmlgui_content)
string(REGEX REPLACE "[ \t\r\n]+" "" krita5_xmlgui_content "${krita5_xmlgui_content}")
string(MD5 krita5_xmlgui_hash_actual "${krita5_xmlgui_content}")
if(NOT krita5_xmlgui_hash_actual STREQUAL krita5_xmlgui_hash_expected)
    message(FATAL_ERROR
        "krita5.xmlgui has changed!\n"
        "Actual hash:   <${krita5_xmlgui_hash_actual}>\n"
        "Expected hash: <${krita5_xmlgui_hash_expected}>\n"
        "You have to update the hash and increase the version number.\n"
        "Look for 'KRITA5.XMLGUI UPDATE' in krita/CMakeLists.txt for instructions."
    )
endif()

# Generate a krita5.xmlgui in the build directory with the specified version
# number and then a krita.qrc that pulls in that file.
set(KRITA5_XMLGUI_PATH "${CMAKE_CURRENT_BINARY_DIR}/krita5.xmlgui")
set(KRITA_QRC_PATH "${CMAKE_CURRENT_BINARY_DIR}/krita.qrc")
set(KRITARC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/kritarc")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/krita5.xmlgui" "${KRITA5_XMLGUI_PATH}" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/krita.qrc" "${KRITA_QRC_PATH}" @ONLY)

# separate listing, both used by Krita and KritaSketch
set(krita_QRCS
    ${KRITA_QRC_PATH}
    ${CMAKE_SOURCE_DIR}/krita/pics/Breeze-dark/breeze-dark-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/Breeze-light/breeze-light-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/layerbox/layerbox-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/layers/layers-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/misc-light/misc-light-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/misc-dark/misc-dark-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/paintops/paintops-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/tools/SVG/16/tools-svg-16-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/tool_transform/tool-transform-icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/svg/svg-icons.qrc
    ${CMAKE_SOURCE_DIR}/libs/flake/flake.qrc
    ${CMAKE_SOURCE_DIR}/libs/widgets/kritawidgets.qrc
    ${CMAKE_SOURCE_DIR}/pics/icons.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/aboutdata/aboutdata.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/shaders/shaders.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/cursors/cursors.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/gamutmasks/gamutmasks.qrc
    ${CMAKE_SOURCE_DIR}/plugins/paintops/defaultpresets/defaultpresets.qrc
    ${CMAKE_SOURCE_DIR}/krita/pics/branding/${BRANDING}/branding.qrc
    ${CMAKE_SOURCE_DIR}/krita/data/splash/${krita_splash_qrc}
    ${CMAKE_SOURCE_DIR}/krita/pics/sponsors/sponsors.qrc
    CACHE INTERNAL "krita_QRCS"
)

if (QT_MAJOR_VERSION STREQUAL "6")
    list(APPEND krita_QRCS ${CMAKE_SOURCE_DIR}/krita/data/mime-database/mime-database.qrc)
endif()

qt_add_resources(krita_SRCS ${krita_QRCS})

if (ANDROID AND QT_MAJOR_VERSION STREQUAL "5")
    kis_add_library(krita SHARED ${krita_SRCS})
    target_link_libraries(krita PRIVATE Qt5::AndroidExtras)
elseif (OHOS AND QT_MAJOR_VERSION STREQUAL "6")
    # HarmonyOS launches native Qt applications through a UIAbility.  Qt's
    # executable helper therefore creates a MODULE library with an exported
    # main() entry point and registers the HAP deployment finalizer.
    qt_add_executable(krita MANUAL_FINALIZATION ${krita_SRCS})
elseif (WIN32)
    # Build the main implementation into a DLL to be called by a stub EXE.
    kis_add_library(krita SHARED ${krita_SRCS})
    set_target_properties(krita PROPERTIES PREFIX "")
else()
    add_executable(krita ${krita_SRCS})
endif()
target_link_libraries(krita
    PRIVATE
                      kritamultiarch
                      kritaui
                      Qt${QT_MAJOR_VERSION}::Core
                      Qt${QT_MAJOR_VERSION}::Gui
                      Qt${QT_MAJOR_VERSION}::Widgets
                      Qt${QT_MAJOR_VERSION}::Xml
                      Qt${QT_MAJOR_VERSION}::Network
                      Qt${QT_MAJOR_VERSION}::PrintSupport
                      Qt${QT_MAJOR_VERSION}::Svg)
if(HAVE_KCRASH)
    target_link_libraries(krita PRIVATE KF${KF_MAJOR}::Crash)
endif()

if (OHOS AND QT_MAJOR_VERSION STREQUAL "6")
    set(KRITA_HARMONYOS_EXTRA_LIBS "" CACHE STRING
        "Additional target libraries to include in the Krita HarmonyOS HAP")
    # harmonydeployqt can discover the libraries linked directly by the main
    # executable, but Krita's internal shared-library dependency graph is much
    # deeper.  In particular, libkrita.so reaches libkritaimpex through
    # libkritaui, and the HarmonyOS dynamic loader aborts before main() when
    # that transitive library is absent from the HAP.  List the core targets
    # explicitly so their TARGET_FILE paths are emitted into the deployment
    # settings and harmonydeployqt can collect the complete ELF closure.
    set(KRITA_HARMONYOS_CORE_LIB_TARGETS
        kritacommand
        kritaflake
        kritaglobal
        kritaimage
        kritaimpex
        kritalibbrush
        kritametadata
        kritapigment
        kritaplugin
        kritapsdutils
        kritaresources
        kritaresourcewidgets
        kritastore
        kritaversion
        kritawidgets
        kritawidgetutils
    )

    # Krita creates most menus, tools, dockers, paint engines, filters and
    # import/export actions from MODULE libraries at runtime.  HarmonyOS does
    # not have an install-time plugin directory for harmonydeployqt to scan,
    # so collect every enabled module target and place it beside libkrita.so.
    # Keeping this automatic is important: an optional plugin enabled by a
    # future configuration must not silently disappear from the application.
    set_property(GLOBAL PROPERTY KRITA_HARMONYOS_MODULE_TARGETS "")
    function(krita_collect_harmonyos_module_targets source_dir)
        get_property(_krita_harmonyos_targets DIRECTORY "${source_dir}" PROPERTY BUILDSYSTEM_TARGETS)
        foreach(_krita_harmonyos_target IN LISTS _krita_harmonyos_targets)
            get_target_property(_krita_harmonyos_target_type ${_krita_harmonyos_target} TYPE)
            if(_krita_harmonyos_target_type STREQUAL "MODULE_LIBRARY")
                set_property(GLOBAL APPEND PROPERTY KRITA_HARMONYOS_MODULE_TARGETS
                    ${_krita_harmonyos_target})
            endif()
        endforeach()

        get_property(_krita_harmonyos_subdirs DIRECTORY "${source_dir}" PROPERTY SUBDIRECTORIES)
        foreach(_krita_harmonyos_subdir IN LISTS _krita_harmonyos_subdirs)
            krita_collect_harmonyos_module_targets("${_krita_harmonyos_subdir}")
        endforeach()
    endfunction()
    krita_collect_harmonyos_module_targets("${CMAKE_SOURCE_DIR}/plugins")
    krita_collect_harmonyos_module_targets("${CMAKE_SOURCE_DIR}/qmlmodules")
    get_property(KRITA_HARMONYOS_MODULE_TARGETS GLOBAL PROPERTY KRITA_HARMONYOS_MODULE_TARGETS)
    add_dependencies(krita ${KRITA_HARMONYOS_MODULE_TARGETS})

    set(KRITA_HARMONYOS_PACKAGED_LIBS ${KRITA_HARMONYOS_EXTRA_LIBS})
    list(APPEND KRITA_HARMONYOS_PACKAGED_LIBS ${KRITA_HARMONYOS_MODULE_TARGETS})
    foreach(_krita_harmonyos_lib IN LISTS KRITA_HARMONYOS_CORE_LIB_TARGETS)
        if(TARGET ${_krita_harmonyos_lib})
            list(APPEND KRITA_HARMONYOS_PACKAGED_LIBS ${_krita_harmonyos_lib})
        else()
            message(FATAL_ERROR
                "Required Krita HarmonyOS library target is missing: ${_krita_harmonyos_lib}")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES KRITA_HARMONYOS_PACKAGED_LIBS)
    set(KRITA_HARMONYOS_BUNDLE_NAME "com.dhuzcs.StellarCanvas" CACHE STRING
        "HarmonyOS application bundle name used for signing and custom UTD identifiers")
    set(KRITA_HARMONYOS_RESOURCE_MAPPINGS
        "${CMAKE_INSTALL_PREFIX}/share=>share" CACHE STRING
        "Resource source directories and optional =>resfile/subdirectory destinations for the HarmonyOS HAP")
    set(KRITA_HARMONYOS_UTD_FILE
        "${CMAKE_CURRENT_BINARY_DIR}/harmonyos/utd.json5")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/harmonyos")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/harmonyos/utd.json5.in"
        "${KRITA_HARMONYOS_UTD_FILE}"
        @ONLY
    )

    set_target_properties(krita PROPERTIES
        # The development certificate in this workspace is provisioned for this
        # bundle name. A release build can override it once a distribution
        # certificate for the final Krita identity is available.
        QT_HARMONYOS_APP_BUNDLE_NAME "${KRITA_HARMONYOS_BUNDLE_NAME}"
        QT_HARMONYOS_APP_VERSION_CODE 1000001
        QT_HARMONYOS_APP_VERSION_NAME "1.0.0"
        QT_HARMONYOS_APP_NAME "StellarCanvas"
        QT_HARMONYOS_APP_VENDOR "Zou Changsheng"
        QT_HARMONYOS_APP_LABEL "StellarCanvas"
        QT_HARMONYOS_APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/harmonyos/stellarcanvas-icon.png"
        QT_HARMONYOS_MODULE_DESCRIPTION "StellarCanvas digital painting application for HarmonyOS"
        QT_HARMONYOS_MODULE_DEVICE_TYPES "tablet;2in1"
        QT_HARMONYOS_ABILITY_ORIENTATION "auto_rotation"
        QT_HARMONYOS_COMPATIBLE_SDK_VERSION "6.1.1(24)"
        QT_HARMONYOS_TARGET_SDK_VERSION "6.1.1(24)"
        QT_HARMONYOS_COMPILE_SDK_VERSION "6.1.1(24)"
        QT_HARMONYOS_EXTRA_LIBS "${KRITA_HARMONYOS_PACKAGED_LIBS}"
        QT_HARMONYOS_EXTRA_RESOURCE_DIRS "${KRITA_HARMONYOS_RESOURCE_MAPPINGS}"
        QT_HARMONYOS_CUSTOM_UTD_FILE "${KRITA_HARMONYOS_UTD_FILE}"
        QT_HARMONYOS_FILE_UTDS
            "${KRITA_HARMONYOS_BUNDLE_NAME}.krita-document;${KRITA_HARMONYOS_BUNDLE_NAME}.krita-archive;${KRITA_HARMONYOS_BUNDLE_NAME}.openraster-image;${KRITA_HARMONYOS_BUNDLE_NAME}.photoshop-document;general.image"
        QT_HARMONYOS_EXCLUDED_PROJECT_LIBRARIES
            "dummytrivialinterface;kritadummyplugin1;kritadummyplugin2;kritadummyplugin2_ver2;kritadummyplugin3"
    )

    qt_add_harmonyos_permission(krita NAME ohos.permission.INTERNET)
    # Keep access to documents selected through the HarmonyOS file picker so
    # that Krita can reopen recent files after the application is restarted.
    qt_add_harmonyos_permission(krita NAME ohos.permission.FILE_ACCESS_PERSIST)
    # WRITE_IMAGEVIDEO is a system_basic, user-granted permission.  It still
    # has to be enabled in the signed HarmonyOS profile after cloud approval.
    qt_add_harmonyos_permission(krita
        NAME ohos.permission.WRITE_IMAGEVIDEO
        REASON "Used to save and update images and videos in the shared media library."
        USED_SCENE_ABILITIES QAbility
        USED_SCENE_WHEN inuse)
    qt_finalize_executable(krita)
endif()

if (APPLE)
    set(KRITA_VERSION_NUMBER_STRING "${KRITA_STABLE_VERSION_MAJOR}.${KRITA_STABLE_VERSION_MINOR}.${KRITA_VERSION_RELEASE}")
    set_target_properties(krita PROPERTIES INSTALL_RPATH "@loader_path/../../../../lib;@loader_path/../lib;
        @loader_path/../Frameworks;@executable_path/../../../../lib;@executable_path/../lib;@executable_path/../Frameworks")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_ICON_FILE "krita")
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING ${KRITA_VERSION_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING ${KRITA_VERSION_NUMBER_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION ${KRITA_VERSION_NUMBER_STRING})
    set_target_properties(krita PROPERTIES MACOSX_BUNDLE_COPYRIGHT "GNU Public License, V2 or, at your option, any later version.")
endif ()

if (NOT ANDROID)
  set(KRITAVERSION_SRCS kritaversion.cpp)
  add_executable(krita_version ${KRITAVERSION_SRCS})
  target_link_libraries(krita_version
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Core
        kritaversion
  )

  if(APPLE)
      set_target_properties(krita_version PROPERTIES INSTALL_RPATH "@loader_path/../Frameworks;@executable_path/../lib;@executable_path/../Frameworks")
      set_property(TARGET krita_version PROPERTY MACOSX_BUNDLE OFF)
  endif ()

  install(TARGETS krita_version ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()


if(APPLE)
    set(KRITA_ASSET_KRZ_ICNS "krita-krz.icns" "krita-kra.icns")
    add_custom_command(
      OUTPUT ${KRITA_ASSET_KRZ_ICNS}
      COMMAND ${CMAKE_COMMAND}
      ARGS -E copy
      "./pics/mimetypes/krz/krita-krz.icns"
      "./pics/mimetypes/krita-kra.icns"
      "${CMAKE_CURRENT_BINARY_DIR}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_source_files_properties(${KRITA_ASSET_KRZ_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    # make krita depend on assets icons
    if(${XCODE_VERSION} VERSION_GREATER_EQUAL "26.0")
      target_sources(krita PRIVATE ${ASSETS_FILES})
    endif()
    target_sources(krita PRIVATE ${KRITA_ASSET_KRZ_ICNS})

    install(TARGETS krita ${INSTALL_TARGETS_DEFAULT_ARGS} BUNDLE DESTINATION bin)
elseif(OHOS)
    # qt_add_executable() creates a MODULE library for a HarmonyOS UIAbility.
    install(TARGETS krita LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR})
else()
    install(TARGETS krita ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()
install(FILES org.kde.krita.desktop DESTINATION ${KDE_INSTALL_APPDIR})

set(actions_files krita.action kritamenu.action)
if (ANDROID)
    # In-app-purchase-related actions are only relevant on Android. They
    # wouldn't show up in the menu anyway because the application doesn't
    # instantiate them, but they'd still appear in the keyboard shortcuts.
    list(APPEND actions_files kritamenu_android.action)
endif()
install(FILES ${actions_files} DESTINATION ${KDE_INSTALL_DATADIR}/krita/actions)

install(FILES  org.kde.krita.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

install(DIRECTORY DESTINATION ${KDE_INSTALL_DATADIR}/krita/shortcuts)

if (WIN32)
    configure_file(versioninfo.rc.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc)
    if (QT_MAJOR_VERSION STREQUAL "6")
        configure_file(krita6.exe.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/krita.exe.manifest)
    else()
        configure_file(krita.exe.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/krita.exe.manifest)
    endif()
    if (MINGW)
        # CMake does not know how to embed manifest on mingw, so we create our own RC file to be included
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/krita.exe.manifest.rc "1 24 \"krita.exe.manifest\"\n")
        set(KRITA_MANIFEST_SRC ${CMAKE_CURRENT_BINARY_DIR}/krita.exe.manifest.rc)
    else()
        set(KRITA_MANIFEST_SRC
            ${CMAKE_CURRENT_BINARY_DIR}/krita.exe.manifest)
    endif()
    add_executable(krita_windows_stub_exe
        windows_stub_main.cpp
        ${krita_windows_stub_ICONS_SRCS}
        ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc
        ${KRITA_MANIFEST_SRC}
    )
    target_link_libraries(krita_windows_stub_exe PRIVATE krita)
    if (QT_MAJOR_VERSION STREQUAL "6")
        target_link_libraries(krita_windows_stub_exe PRIVATE Qt6::EntryPointPrivate)
    else()
        target_link_libraries(krita_windows_stub_exe PRIVATE Qt5::WinMain)
    endif()
    set_target_properties(krita_windows_stub_exe
        PROPERTIES
            OUTPUT_NAME "krita"
    )
    if (MSVC)
        target_link_options(krita_windows_stub_exe PRIVATE "/DEBUG:NONE")
    endif()
    install(TARGETS krita_windows_stub_exe ${INSTALL_TARGETS_DEFAULT_ARGS})

    add_executable(krita_windows_stub_com
        windows_stub_main.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc
        ${KRITA_MANIFEST_SRC}
    )
    target_link_libraries(krita_windows_stub_com PRIVATE krita)
    set_target_properties(krita_windows_stub_com
        PROPERTIES
            WIN32_EXECUTABLE false
            OUTPUT_NAME "krita"
            SUFFIX ".com"
    )
    if (MSVC)
        target_link_options(krita_windows_stub_com PRIVATE "/DEBUG:NONE")
        # If MSVC uses incremental linking, it will race the database
        # for krita.exe with the one for krita.com.
        add_dependencies(krita_windows_stub_com krita_windows_stub_exe)
    endif()
    install(TARGETS krita_windows_stub_com ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (WIN32)
