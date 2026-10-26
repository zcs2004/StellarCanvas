cmake_minimum_required(VERSION 3.29)

# OpenSSL has an alternate install target called "install_sw" that skips installing the manual (which is fairly large).
# We use this on android and macos/linux.
project(ext_openssl)
include(${CMAKE_SOURCE_DIR}/../cmake/base-dep-options.cmake)

if (CMAKE_SYSTEM_NAME STREQUAL "OHOS")
    if (NOT OHOS_SDK_ROOT AND DEFINED ENV{OHOS_SDK_ROOT})
        set(OHOS_SDK_ROOT "$ENV{OHOS_SDK_ROOT}")
    endif()
    if (NOT OHOS_SDK_ROOT)
        message(FATAL_ERROR "OHOS_SDK_ROOT must point to the HarmonyOS/OpenHarmony SDK")
    endif()

    find_package(Perl REQUIRED)

    # OpenSSL's build system requires a make implementation even when the
    # outer dependency project uses Ninja. Keep this overridable for CI and
    # for non-Windows hosts.
    set(OPENSSL_MAKE_PROGRAM "" CACHE FILEPATH "Make-compatible program used to build OpenSSL for OHOS")
    if (NOT OPENSSL_MAKE_PROGRAM)
        find_program(OPENSSL_MAKE_PROGRAM NAMES gmake make mingw32-make)
    endif()
    if (NOT OPENSSL_MAKE_PROGRAM)
        message(FATAL_ERROR
            "A make-compatible program is required for OpenSSL. "
            "Set OPENSSL_MAKE_PROGRAM to make, gmake, or mingw32-make.")
    endif()

    set(_ohos_llvm_bin "${OHOS_SDK_ROOT}/native/llvm/bin")
    if (CMAKE_HOST_WIN32)
        # The SDK's target-prefixed clang launchers are POSIX shell scripts
        # without a file extension. Native Perl + mingw32-make cannot execute
        # them reliably, and the SDK path commonly contains spaces. Generate
        # small command wrappers in the dependency build directory instead.
        set(_ohos_clang "${_ohos_llvm_bin}/clang.exe")
        set(_ohos_clangxx "${_ohos_llvm_bin}/clang++.exe")
        set(_ohos_ar_binary "${_ohos_llvm_bin}/llvm-ar.exe")
        set(_ohos_ranlib_binary "${_ohos_llvm_bin}/llvm-ranlib.exe")
        set(_ohos_strip_binary "${_ohos_llvm_bin}/llvm-strip.exe")
        set(_ohos_sysroot "${OHOS_SDK_ROOT}/native/sysroot")

        foreach(_tool IN ITEMS _ohos_clang _ohos_clangxx _ohos_ar_binary
                               _ohos_ranlib_binary _ohos_strip_binary)
            if (NOT EXISTS "${${_tool}}")
                message(FATAL_ERROR "Required OHOS LLVM tool was not found: ${${_tool}}")
            endif()
        endforeach()

        set(_ohos_wrapper_dir "${CMAKE_CURRENT_BINARY_DIR}/ohos-openssl-tools")
        file(MAKE_DIRECTORY "${_ohos_wrapper_dir}")
        file(WRITE "${_ohos_wrapper_dir}/cc.cmd"
            "@echo off\r\n\"${_ohos_clang}\" -target aarch64-linux-ohos --sysroot=\"${_ohos_sysroot}\" -D__MUSL__ %*\r\n")
        file(WRITE "${_ohos_wrapper_dir}/cxx.cmd"
            "@echo off\r\n\"${_ohos_clangxx}\" -target aarch64-linux-ohos --sysroot=\"${_ohos_sysroot}\" -D__MUSL__ %*\r\n")
        file(WRITE "${_ohos_wrapper_dir}/ar.cmd"
            "@echo off\r\n\"${_ohos_ar_binary}\" %*\r\n")
        file(WRITE "${_ohos_wrapper_dir}/ranlib.cmd"
            "@echo off\r\n\"${_ohos_ranlib_binary}\" %*\r\n")
        file(WRITE "${_ohos_wrapper_dir}/strip.cmd"
            "@echo off\r\n\"${_ohos_strip_binary}\" %*\r\n")

        set(_ohos_cc "${_ohos_wrapper_dir}/cc.cmd")
        set(_ohos_cxx "${_ohos_wrapper_dir}/cxx.cmd")
        set(_ohos_ar "${_ohos_wrapper_dir}/ar.cmd")
        set(_ohos_ranlib "${_ohos_wrapper_dir}/ranlib.cmd")
        set(_ohos_strip "${_ohos_wrapper_dir}/strip.cmd")
    else()
        set(_ohos_cc "${_ohos_llvm_bin}/aarch64-unknown-linux-ohos-clang")
        set(_ohos_cxx "${_ohos_llvm_bin}/aarch64-unknown-linux-ohos-clang++")
        set(_ohos_ar "${_ohos_llvm_bin}/llvm-ar")
        set(_ohos_ranlib "${_ohos_llvm_bin}/llvm-ranlib")
        set(_ohos_strip "${_ohos_llvm_bin}/llvm-strip")

        foreach(_tool IN ITEMS _ohos_cc _ohos_cxx _ohos_ar _ohos_ranlib _ohos_strip)
            if (NOT EXISTS "${${_tool}}")
                message(FATAL_ERROR "Required OHOS LLVM tool was not found: ${${_tool}}")
            endif()
        endforeach()
    endif()

    # OpenSSL has no dedicated OHOS Configure target. The SDK exposes a
    # Linux-compatible aarch64 Clang/musl ABI, so linux-aarch64 is the correct
    # architecture recipe while the explicit compiler paths select OHOS.
    # Static libraries are intentional: Qt's qopensslbackend plugin absorbs
    # them and the HAP does not need versioned libssl/libcrypto shared objects.
    ExternalProject_Add(ext_openssl
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://openssl-library.org/source/openssl-3.5.9.tar.gz
        URL_HASH SHA256=603f5602e2eef00d77fbd429d34dcd5822bb301757a1bc9cdb24c670f1eb859a

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
            CC=${_ohos_cc}
            CXX=${_ohos_cxx}
            AR=${_ohos_ar}
            RANLIB=${_ohos_ranlib}
            STRIP=${_ohos_strip}
            ${PERL_EXECUTABLE} <SOURCE_DIR>/Configure linux-aarch64
                no-shared no-tests no-apps no-docs no-module -fPIC
                --prefix=${EXTPREFIX} --openssldir=ssl --libdir=lib

        BUILD_COMMAND ${CMAKE_COMMAND} -E env
            CC=${_ohos_cc}
            CXX=${_ohos_cxx}
            AR=${_ohos_ar}
            RANLIB=${_ohos_ranlib}
            STRIP=${_ohos_strip}
            ${OPENSSL_MAKE_PROGRAM} -j${SUBMAKE_JOBS} build_libs

        INSTALL_COMMAND ${CMAKE_COMMAND} -E env
            CC=${_ohos_cc}
            CXX=${_ohos_cxx}
            AR=${_ohos_ar}
            RANLIB=${_ohos_ranlib}
            STRIP=${_ohos_strip}
            ${OPENSSL_MAKE_PROGRAM} install_sw

        UPDATE_COMMAND ""
    )
elseif (WIN32)
    if (MSVC)
    # Use the binaries from ConanCenter
        ExternalProject_Add(ext_openssl
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://files.kde.org/krita/build/dependencies/openssl-1.1.1k-win64-msvc.tgz
            URL_HASH SHA256=beae4c78e2f2b64cc3621402a34550ef1fffd08cd35b6a6802f22ed0460ff1d4

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying openssl 64-bit dependencies
            CONFIGURE_COMMAND ""
            
            INSTALL_COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/include -DDST=${EXTPREFIX}/include -P ${KRITA_CI_INSTALL_DIRECTORY}
                    COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/lib/libcrypto.lib -DDST=${EXTPREFIX}/lib -P ${KRITA_CI_INSTALL}
                    COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/lib/libssl.lib -DDST=${EXTPREFIX}/lib -P ${KRITA_CI_INSTALL}
                    COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/libcrypto-1_1-x64.dll -DDST=${EXTPREFIX}/bin -P ${KRITA_CI_INSTALL}
                    COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/libssl-1_1-x64.dll -DDST=${EXTPREFIX}/bin -P ${KRITA_CI_INSTALL}

            UPDATE_COMMAND ""
        )
    else(MSVC)
    # Use the binaries from curl for Windows (https://github.com/curl/curl-for-win).
        ExternalProject_Add(ext_openssl
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://files.kde.org/krita/build/dependencies/openssl-1.1.1l_4-win64-mingw.zip
            URL_HASH SHA256=6582432d4f537323bfa032a1d91b4eee12efdf0c72e4fabdacc0ac5dd1685e9a

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying openssl 64-bit dependencies
            CONFIGURE_COMMAND ""
            
            INSTALL_COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/include -DDST=${EXTPREFIX}/include -P ${KRITA_CI_INSTALL_DIRECTORY}
                    COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/lib/libcrypto.dll.a -DDST=${EXTPREFIX}/lib -P ${KRITA_CI_INSTALL}
                    COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/lib/libssl.dll.a -DDST=${EXTPREFIX}/lib -P ${KRITA_CI_INSTALL}
                    COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/libcrypto-1_1-x64.dll -DDST=${EXTPREFIX}/bin -P ${KRITA_CI_INSTALL}
                    COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/libssl-1_1-x64.dll -DDST=${EXTPREFIX}/bin -P ${KRITA_CI_INSTALL}
            
            UPDATE_COMMAND ""
        )
    endif(MSVC)
elseif (ANDROID)
    set(PAGE_ALIGNMENT_ENV_ARG)
    if (PAGE_ALIGN_16K_LINKER_FLAGS)
        message(STATUS "Enable 16K page alignment for OpenSSL")
        set(PAGE_ALIGNMENT_ENV_ARG
            LDFLAGS=${PAGE_ALIGN_16K_LINKER_FLAGS})
    endif()

    ExternalProject_Add( ext_openssl
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.openssl.org/source/openssl-1.1.1m.tar.gz
        URL_HASH SHA256=f89199be8b23ca45fc7cb9f1d8d3ee67312318286ad030f5316aca6462db6c96

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
            ${PAGE_ALIGNMENT_ENV_ARG}
            ANDROID_NDK_HOME=${CMAKE_ANDROID_NDK}
            ANDROID_NDK=${CMAKE_ANDROID_NDK}
            PATH=${ANDROID_TOOLCHAIN_ROOT}/bin/:$ENV{PATH}
            <SOURCE_DIR>/Configure android-${CMAKE_ANDROID_ARCH} -D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
            --prefix=${EXTPREFIX} --openssldir=ssl

        BUILD_COMMAND ${CMAKE_COMMAND} -E env
            PATH=${ANDROID_TOOLCHAIN_ROOT}/bin:$ENV{PATH}
            CXXFLAGS=-D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
            make build_libs SHLIB_VERSION_NUMBER= SHLIB_EXT=_1_1.so
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env
            PATH=${ANDROID_TOOLCHAIN_ROOT}/bin:$ENV{PATH}
            CXXFLAGS=-D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
            # OpenSSL cannot handle DESTDIR as an environment variable. Instead,
            # it expects it to be passed as an explicit make argument
            ${CMAKE_COMMAND} -P ${KRITA_CI_RUN_MAKE_WITH_DESTDIR} -- install_sw SHLIB_VERSION_NUMBER= SHLIB_EXT=_1_1.so

        UPDATE_COMMAND ""
)
else (WIN32)

    kis_ExternalProject_Add_with_separate_builds_apple(
        ext_openssl

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.openssl.org/source/openssl-1.1.1m.tar.gz
        URL_HASH SHA256=f89199be8b23ca45fc7cb9f1d8d3ee67312318286ad030f5316aca6462db6c96

        WORKING_DIRECTORY <SOURCE_DIR>
        CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${EXTPREFIX} --openssldir=ssl
        
        BUILD_COMMAND make clean COMMAND make -j${SUBMAKE_JOBS} depend COMMAND make -j${SUBMAKE_JOBS}

        # OpenSSL cannot handle DESTDIR as an environment variable. Instead,
        # it expects it to be passed as an explicit make argument
        INSTALL_COMMAND ${CMAKE_COMMAND} -P ${KRITA_CI_RUN_MAKE_WITH_DESTDIR} -- install_sw
        
        UPDATE_COMMAND ""
    )

endif (WIN32)

krita_add_to_ci_targets(ext_openssl)
