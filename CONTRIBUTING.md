# Contributing

Thank you for helping with StellarCanvas. The project is an experimental Krita
port, so small, reviewable changes with clear platform rationale are preferred.

## Before opening a change

1. Keep upstream copyright and license headers intact.
2. Separate general Krita fixes from HarmonyOS-specific adaptations where
   practical; broadly useful fixes may belong upstream first.
3. Do not commit build output, downloaded SDKs, HAP files, device dumps,
   credentials, certificates, signing profiles, or machine-specific paths.
4. Describe the SDK, host toolchain, device, architecture, and build mode used
   for testing.
5. Include reproduction steps for bug fixes and tests where feasible.

## Commit and review guidance

- Explain why a platform-specific conditional is required.
- Avoid unrelated formatting changes in imported upstream files.
- Preserve an auditable relationship to the upstream Krita source.
- Update `docs/BUILDING.md` when changing the build or packaging workflow.
- Confirm that logs and screenshots contain no personal or device-identifying
  information before attaching them.
