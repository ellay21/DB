# Security Policy

## Threat model

Revenant's threat model is **mutually-trusting processes within a single trust domain** on one machine. Byzantine peers are explicitly out of scope for v1.

Revenant does not prevent a malicious process from corrupting shared memory segments it has mapped. It provides crash tolerance for accidental failures, not adversarial ones. Optional hardening (CRC32C checksums, read-only consumer mappings for the data region) is available; see the blueprint for details.

Do not rely on Revenant to protect against a compromised process that shares a segment.

## Reporting a vulnerability

Please do **not** file a public GitHub issue for security vulnerabilities.

Report vulnerabilities by emailing **security@example.com** (replace with your address). You will receive a response within 72 hours. If the vulnerability is confirmed, we will work with you on a coordinated disclosure timeline.

Please include:
- A description of the vulnerability and its impact
- Steps to reproduce, ideally a minimal reproducer
- The affected version or commit
