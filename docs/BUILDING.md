# Building StellarCanvas

## Status

The repository contains the source changes from the current HarmonyOS port,
but its original build was assembled in a local development workspace. Local
SDK installations, downloaded dependencies, caches, generated modules, and
compiled binaries are deliberately not committed.

As a result, this document records the known build stages without claiming
that a clean checkout is already reproducible with one command. Contributions
that automate and verify these stages are especially useful.

## Prerequisites

- DevEco Studio with a HarmonyOS/OpenHarmony native SDK compatible with API 24.
- The SDK's Clang/LLVM toolchain and CMake toolchain file.
- CMake, Ninja, Meson, Perl, Python, and a Make-compatible program.
- Enough disk space to build Qt, KDE dependencies, and Krita for arm64.
- A host Qt/tool set needed by the cross-build.

The checked-in project declares HarmonyOS SDK `6.1.1(24)` and targets
`arm64-v8a`. Other versions have not been verified in this snapshot.

## Local path configuration

The files below contain neutral placeholders rather than a contributor's local
paths:

- `scripts/toolchains/ohos-arm64.meson`
- `scripts/toolchains/windows-host-llvm-mingw.meson`

Create local copies if possible, replace `C:/path/to/DevEcoStudio` and
`C:/path/to/StellarCanvas`, and keep machine-specific variants uncommitted.

## Known build stages

1. Build the required host tools, including `pkgconf` and `gperf`.
2. Cross-build the required third-party libraries for arm64 HarmonyOS.
3. Apply/use the dependency definitions in `krita-deps-management/`. The
   OpenSSL definition contains a dedicated OHOS branch.
4. Configure and build the modified source in `krita-master/` against the
   cross-built Qt and dependency prefix.
5. Run Qt's HarmonyOS deployment step. `harmonydeployqt` generates a HarmonyOS
   entry module under:

   ```text
   build/krita-ohos-arm64/krita/harmonyos-build/entry
   ```

6. Open the root project in DevEco Studio or invoke Hvigor after the generated
   module exists.
7. Configure signing locally and build the desired debug or release HAP.

The root `build-profile.json5` intentionally points at the generated module.
The checked-in `entry/` directory is an ArkTS prototype and does not load the
Qt/Krita runtime.

## Outputs and signing

Do not commit build directories, native binaries, HAP files, signing profiles,
certificates, keystores, or passwords. Publish release packages through the
GitHub Releases page, with checksums and a link to the exact corresponding
source revision.

## Reproducibility work still needed

- Record the exact upstream Krita commit used for this snapshot.
- Pin every dependency version and checksum in one machine-readable manifest.
- Add scripts for the host-tool, dependency, Qt, Krita, and packaging stages.
- Add a clean CI build that does not depend on developer-machine paths.
- Document a tested DevEco Studio version and supported device matrix.
