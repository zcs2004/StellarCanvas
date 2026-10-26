cmake_minimum_required(VERSION 3.21)

project(StellarCanvasQtSmoke VERSION 0.1.0 LANGUAGES CXX)

find_package(Qt6 6.12 REQUIRED COMPONENTS Widgets OpenGLWidgets)
qt_standard_project_setup()

qt_add_executable(stellarcanvas_qt_smoke MANUAL_FINALIZATION
    main.cpp
)

target_compile_features(stellarcanvas_qt_smoke PRIVATE cxx_std_17)
target_link_libraries(stellarcanvas_qt_smoke PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
)

set(STELLARCANVAS_OHOS_DEPS_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../build/ohos-deps"
    CACHE PATH "Prefix containing the HarmonyOS fontconfig/freetype/expat build")
cmake_path(ABSOLUTE_PATH STELLARCANVAS_OHOS_DEPS_ROOT
    NORMALIZE OUTPUT_VARIABLE _ohos_deps_root)
set(_ohos_deps "${_ohos_deps_root}/lib")
set_target_properties(stellarcanvas_qt_smoke PROPERTIES
    QT_HARMONYOS_APP_BUNDLE_NAME "com.dhuzcs.StellarCanvas"
    QT_HARMONYOS_APP_VERSION_CODE 1000001
    QT_HARMONYOS_APP_VERSION_NAME "0.1.1"
    QT_HARMONYOS_APP_NAME "StellarCanvas Qt Smoke"
    QT_HARMONYOS_APP_VENDOR "StellarCanvas"
    QT_HARMONYOS_APP_LABEL "StellarCanvas Qt"
    QT_HARMONYOS_MODULE_DESCRIPTION "Qt Widgets HarmonyOS migration smoke test"
    QT_HARMONYOS_MODULE_DEVICE_TYPES "tablet;2in1"
    QT_HARMONYOS_ABILITY_ORIENTATION "auto_rotation"
    QT_HARMONYOS_COMPATIBLE_SDK_VERSION "6.1.1(24)"
    QT_HARMONYOS_TARGET_SDK_VERSION "6.1.1(24)"
    QT_HARMONYOS_COMPILE_SDK_VERSION "6.1.1(24)"
    QT_HARMONYOS_EXTRA_LIBS
        "${_ohos_deps}/libfontconfig.so.1;${_ohos_deps}/libfreetype.so.6;${_ohos_deps}/libexpat.so.1"
)

qt_finalize_executable(stellarcanvas_qt_smoke)
