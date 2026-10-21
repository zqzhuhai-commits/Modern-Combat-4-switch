# Frontend 0.1.1 build report — 2026-10-21

Native AArch64 frontend compiled with devkitA64 GCC 16.1.0, libnx 4.12.0 and switch-tools 1.13.1. NSP packaged with hacBrewPack 3.05 using private local input, never distributed.

Validated:

- ELF64 AArch64, NSO0, NRO0, NRO icon and NACP title, AArch64 NPDM and program ID.
- Installable NSP has three NCA files, including Program, Control and Meta contents. NCA sections were decrypted/extracted with hactool; executable matched build byte-for-byte, all 16 NACP language slots read 孤侠魅影, embedded Control icon matched the new 256x256 JPEG, and NPDM ID matched, and RomFS status text matched.
- Production scanner host tests: missing/invalid/wrong/candidate APK, nested and empty files, symlink loop, scan bounds/depth, report export, write failure. Supplied original game library matched its known SHA256 through the same scanner.
- Four 1280x720 screens rendered by the production C renderer on host and visually inspected. These are interface previews, not Switch screenshots.
- Original PortMaster ZIP examined without distributing game files; data preparation dry run recognizes the library and 3,684 resource files.

Not validated: physical Switch installation, display/controller/touch behavior, sleep/resume, gameplay, runtime JNI initialization, game rendering or audio. Valid package structure does not establish hardware acceptance.

Title ID: `0100F53447550000`. Frontend version: `0.1.1`. NRO and NSP hashes are generated in `frontend/build/manifest.json` after local verification. The foreground UI has no game launch action because the gameplay runtime remains pending.

Artwork: built-in image generation using the user reference. Prompt: faithful square recomposition with adventurer, sunset and temple; remove original UI/copyright/English title; add only 孤侠魅影. Project asset: `frontend/assets/icon-source.png`; encoded 256px RGB baseline JPEG is shared by NRO, NSP and the runtime launcher.
