<div align="center">
  <img
    src="AppScope/resources/base/media/app_icon.png"
    alt="StellarCanvas application icon"
    width="180"
  />
  <h1>StellarCanvas</h1>
  <p><strong>Krita for HarmonyOS tablets and 2-in-1 devices</strong></p>
</div>

---

StellarCanvas is a work-in-progress port and adaptation of Krita for HarmonyOS
tablets and 2-in-1 devices. The repository contains the HarmonyOS application
shell, cross-compilation configuration, dependency-build changes, and the
modified Krita source snapshot used during the port.

> **Project status:** experimental. This is not an official KDE or Krita
> release. The current checkout documents the ongoing port, but does not yet
> provide a one-command reproducible build.

## Current target

| Item | Value |
| --- | --- |
| Operating system | HarmonyOS |
| SDK configuration | 6.1.1, API 24 |
| Target architecture | arm64 / `arm64-v8a` |
| UI/runtime | Qt and Krita native code packaged as a HarmonyOS application |
| Primary development host | Windows |

The exact upstream Krita commit was not preserved in the original source
snapshot. Before making a release, the project should record and pin the
upstream revision so changes can be reviewed and rebased reliably.

## Repository layout

- `AppScope/` — HarmonyOS application metadata and icons.
- `entry/` — ArkTS prototype module and native bridge declarations. This is
  not the generated Qt/Krita application module.
- `harmony/` — small Qt/HarmonyOS smoke tests and package probes.
- `krita-master/` — modified Krita source snapshot used by StellarCanvas.
- `krita-deps-management/` — dependency build definitions, including the
  HarmonyOS OpenSSL changes.
- `scripts/toolchains/` — Meson machine files with public path placeholders.
- `hvigor/` and root `*.json5` files — HarmonyOS/Hvigor project metadata.

## Building

See [docs/BUILDING.md](docs/BUILDING.md) for the known build stages and the
remaining reproducibility limitations.

The public configuration intentionally contains no signing identity. Configure
a local debug or release certificate in DevEco Studio, or provide signing
material through protected CI secrets. Never commit certificate files,
keystores, profiles, or passwords.

## Contributing

Contributions are welcome while the port is being stabilized. Please read
[CONTRIBUTING.md](CONTRIBUTING.md) before opening a change. When reporting an
issue, include the HarmonyOS SDK version, device model, architecture, build
type, and relevant logs with personal information removed.

## Upstream and licensing

StellarCanvas 使用并修改了
[KDE Krita GitHub 仓库](https://github.com/kde/krita)中的源代码。

StellarCanvas uses and modifies source code from the
[KDE Krita GitHub repository](https://github.com/kde/krita). Krita is developed
by the Krita community and the KDE community; its official website is
[krita.org](https://krita.org/), and its primary KDE repository is hosted at
[invent.kde.org/graphics/krita](https://invent.kde.org/graphics/krita).
StellarCanvas is an independent, unofficial project and is not endorsed by or
affiliated with KDE.

The combined program is distributed under the GNU General Public License,
version 3. Individual files and third-party components may use other compatible
licenses. In addition to the GPLv3 text in [LICENSE](LICENSE), the repository
includes a three-clause BSD notice in
[LICENSE-BSD-3-CLAUSE.txt](LICENSE-BSD-3-CLAUSE.txt) containing the copyright
notice for Zou Changsheng and The Regents of the University of California. The
BSD notice applies only to the files it covers and does not replace the GPLv3
terms for the combined Krita-derived program.

See [NOTICE.md](NOTICE.md),
[THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md), and the license notices
retained alongside the relevant source files for further attribution.
