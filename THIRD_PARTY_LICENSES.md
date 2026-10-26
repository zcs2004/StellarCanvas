# Third-party licenses

This file is an overview, not a replacement for the license notices stored
with individual components. When a source file contains its own header or a
component directory contains a license file, that notice controls.

## Major components

| Component | Role | License information |
| --- | --- | --- |
| Krita | Core painting application | GPL-3.0; see `krita-master/COPYING` and individual file headers |
| Qt | UI and HarmonyOS runtime/packaging | Module-dependent LGPL/GPL or commercial terms; consult the exact Qt source/version used for a build |
| KDE Frameworks | Libraries used by Krita | Component-dependent LGPL/GPL terms; consult each framework's source package |
| OpenSSL | TLS/cryptographic dependency | Apache License 2.0 for current OpenSSL 3.x releases; retain the license from the exact downloaded source |
| Raqm and other vendored code | Text shaping and supporting functionality | See the license files under `krita-master/3rdparty_vendor/` and relevant source directories |

## License files retained in this repository

Important notices include, but are not limited to:

- `LICENSE-BSD-3-CLAUSE.txt` — three-clause BSD notice containing the
  copyrights of Zou Changsheng and The Regents of the University of California
- `krita-master/COPYING`
- `krita-master/COPYING-CMAKE-SCRIPTS`
- `krita-master/3rdparty_vendor/raqm/src/raqm-0.10.1/COPYING`
- `krita-master/krita/data/aboutdata/LICENSE`
- `krita-master/krita/data/profiles/ycbcr-icc-profiles/LICENSE-PROFILES.txt`
- `krita-master/krita/integration/3rdparty/LICENSE`
- license files accompanying test fonts and test images
- `krita-master/plugins/generators/simplexnoise/3rdparty/c-open-simplex/LICENSE`
- `krita-master/plugins/impex/raw/3rdparty/libkdcraw/COPYING`
- `krita-master/plugins/impex/raw/3rdparty/libkdcraw/COPYING.LIB`

## Dependencies downloaded during a build

Downloaded dependency source and build output are intentionally not committed.
Before publishing binaries, record the exact versions, source URLs, hashes,
and licenses of every dependency used in that release, and make corresponding
source available whenever its license requires it.
