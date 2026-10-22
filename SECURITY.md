# Security Policy

## Supported versions

Only the latest `master` / tagged release is supported with security fixes.

| Version | Supported |
| ------- | --------- |
| latest release (`v1.x`) | ✅ |
| older tags | ❌ |

## Reporting a vulnerability

Do **not** open a public issue for security reports.

Email the maintainer privately with:

- Affected version / commit
- Steps to reproduce (input sequence, OS, SDL3 version)
- Impact assessment if known

You will receive an acknowledgment and a fix timeline. Once fixed,
a release note will credit the reporter (unless anonymity is requested).

## Scope

FastCalc is a local desktop app with no network, no IPC, and no elevated
privileges. Realistic concerns are: crashes / hangs on crafted input
(paste, key sequences), excessive memory use, and unsafe font handling.
Clipboard content is treated as untrusted plain-text and strictly validated.
