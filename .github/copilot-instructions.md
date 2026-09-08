# GitHub Copilot Instructions

## General Guidelines
- Assume **C++20** by default.
- Prefer the C++ standard library (`std`) over Boost when equivalent functionality exists.
- Never use hardcoded paths unless the user explicitly confirms it.
- Never run commands that traverse/search from the root filesystem (e.g., `find /`, or any command starting its search/scan from "/") without first asking for explicit permission. Such scans can take hours on large hard drives. More generally, never execute any command that operates from the root folder without explicit user permission first.
