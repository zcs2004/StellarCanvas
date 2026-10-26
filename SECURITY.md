# Security policy

## Reporting a vulnerability

Please do not disclose a suspected vulnerability in a public issue. Use
GitHub's private vulnerability reporting feature for this repository when it
is available. Include affected revisions, impact, reproduction information,
and any proposed mitigation, but do not include unrelated credentials or
personal data.

## Secrets and signing material

Certificates, keystores, provisioning profiles, signing passwords, API keys,
and access tokens must never be committed. Keep them in local DevEco Studio
configuration or protected CI secret storage. If a secret is committed, revoke
or rotate it immediately; deleting it in a later commit is not sufficient.
