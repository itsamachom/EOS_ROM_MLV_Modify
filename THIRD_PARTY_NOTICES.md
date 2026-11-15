# Third-party notices and source scope

This file is an attribution index, not a replacement for individual licenses or
an assertion that a legal/source audit is complete. Preserve all original file
headers, notices, license texts and attribution when redistributing.

| Component | Attribution / terms | Source or notice in this repository |
| --- | --- | --- |
| Magic Lantern core and modules | Magic Lantern contributors; GPL, with version/upgrade terms in individual files | firmware/COPYING, firmware/README.md, firmware/doc/CREDITS.txt, source headers |
| TinyCC / runtime module linker | Fabrice Bellard and contributors; original LGPL notices retained | firmware/tcc, firmware/tcc/COPYING, firmware/modules/tcc |
| Lua | Copyright (C) 1994–2015 Lua.org, PUC-Rio; MIT notice | firmware/modules/lua/lua/lua.h, release/v3r2/licenses/LUA-MIT.txt |
| UMM allocator | Copyright (c) 2015 Ralph Hempel; MIT | firmware/modules/lua/umm_malloc/LICENSE |
| Noto Sans Thai and derived Thai phrase masks | 2022 The Noto Project Authors; SIL OFL 1.1 | assets/fonts/noto-sans-thai/OFL.txt, SOURCE.md, release/v3r2/licenses/NOTO-SANS-THAI-OFL-1.1.txt |
| Arkanoid | Original module credits include pravdomil.cz; original GPL notices retained | firmware/modules/arkanoid/README.rst and arkanoid.c |
| Pong / Sokoban scripts | Original source comments and attribution retained byte-for-byte | firmware/scripts/pong.lua, firmware/scripts/sokoban.lua |
| Inherited dietlibc 0.33 archive | Exact upstream binary inheritance verified; GPL notice recovered from PR #321; corresponding archive source remains unverified | firmware/src/libs/dietlib-0.33-none-eabi-Os/dietlibc.a; see release/v3r2/PROVENANCE.md |
| Inherited Newlib 1.20.0 archives | Newlib 1.20.0 source URL/build notes and COPYING.NEWLIB recovered from PR #321; original per-file terms still apply | firmware/src/libs/README.txt, arm-none-eabi-O3 archives and release/v3r2/licenses/NEWLIB-NOTICES.txt |
| GCC runtime | Toolchain-provided libgcc linked into the payload; keep applicable GCC runtime licensing/exception information | Toolchain documentation and release source audit; not a claim that the compiler itself is part of this fork |
| Original ML-SETUP.FIR | Unchanged prebuilt installer distributed by the identified upstream; not a Canon firmware updater | firmware/platform/600D.102/ML-SETUP.FIR; shared installer sources in firmware/installer and firmware/src |

## Identity and provenance

The user identified reticulatedpines/magiclantern_simplified as the source.
Comparison with upstream commit 142472e95054d73640f50789ac0fd66325499d82 found
all **2,257 imported file blobs identical** to the initial imported firmware tree
in commit 359f828. This verifies an exact content match. It does not prove the
original upload's Git history or complete source of prebuilt dependencies.

This fork does not claim authorship of upstream code or fonts, and does not
claim approval, support or endorsement by Magic Lantern, its developers, Canon,
Google, the Noto Project or the third-party authors.

No actual Canon ROM dump was found in the named ROM test fixtures: the two
small files under firmware/tools/ghidra/detect_rom_test_files are textual test
fixtures (63 and 58 bytes). Do not replace these with real camera dumps or
publish installer-generated ML/LOGS/ROM*.BIN files.

See AI_ASSISTANCE.md for fork modifications and LLM disclosure. Before marking a
binary release ready, resolve the source/notices questions in
release/v3r2/READINESS.json rather than treating inherited binaries as proof of
complete corresponding source.

## Follow-up license evidence

Upstream PR #321, inspected at d727e1e148e48a2a63c1cf1742e71a361ca61361,
adds GPL/Newlib license texts. Verbatim copies and hashes are preserved in
release/v3r2/licenses and UPSTREAM_PR_EVIDENCE.json. The proposed README supplies
the Newlib 1.20.0 source URL, but explicitly says dietlibc source/build notes are
not in the repository. This does not resolve the dietlibc corresponding-source
gate. PR #290 is a Lua header compatibility fix; no build fixes were applied to
the camera payload in this documentation update.

## Native JPEG tone recipes (v3r6-tone1 experiment)

The 13 distinct native Picture Style/WB recipes derive from the preview settings in HNGSeries/M-Film 0.2.1 (`8118e9c7c072371bd53eda8920f44910e8a35baf`), `ml-dev/filmos/fos_film.c` and `ml-dev/filmos/tools/genfilms.py`. Credit HNGSeries/M-Film contributors; the derived recipe table retains GPL-2.0-only terms. See `research/jpeg-tones/UPSTREAM_NATIVE_RECIPES.json` and the verbatim GPL text beside it. Recipe names/grouping, transaction adapter and canister menu were implemented for this fork. No Film-Luts tables, M-Film RAW decoder, Darkrooms desktop code or third-party sample photos are included in this feature. These native JPEG approximations do not claim the full developed film looks.

### tone2 original catalog

v3r6-tone2 retains all 72 original preset names, native JPEG preview numbers and ISO conversion from `ml-dev/fork/src/filmroll.c` at the same upstream commit, rather than tone1's 13 deduplicated names. The auditable catalog/source hashes are `research/jpeg-tones/UPSTREAM_ORIGINAL_CATALOG.json`. Preserve the same GPL-2.0-only recipe terms and M-Film contributor attribution. Film names identify source presets and do not imply endorsement or exact photographic reproduction. The independent canister artwork contains no copied upstream textures/logos.

### Stock-specific canister labels (tone4)

The compact label table additionally derives from M-Film's `ml-dev/filmos/tools/catalog.py` and the eight built-in label cues in `ml-dev/filmos/fos_film.c`, pinned to the same 8118e9c commit. Credit HNGSeries/M-Film contributors; preserve GPL-2.0-only terms and the verbatim GPL text in research/jpeg-tones. Source hashes, original ramp/level data and scope are in research/film-labels/CATALOG.json. The independent renderer and compact letterforms are fork changes. No manufacturer photograph, third-party logo artwork, upstream raster label texture or RAW/LUT code is bundled by this change. Brand words identify presets and do not imply endorsement. Actual packaging varies; cross-process/cinema-print illustrations do not claim a retail cartridge existed. External retail-photo verification remains incomplete.

### tone5 cylinder technique review

M-Film's `fos_film.c` at the same pinned 8118e9c commit was reviewed for cylinder-angle texture projection, diffuse/specular/material profiles and palette dithering. Credit M-Film contributors for this approach. The fork implements an independent small integer column-profile adaptation and uses the already shipped RBF fonts, without importing the upstream texture/font engine or custom palette. See research/film-labels/CYLINDER_RENDERING.md and tools/generate_film_cylinder.py. Existing GPL label-data terms and notices remain.

### tone6 reference descriptions and Thai masks

Reference-character text derives from M-Film's DESC catalog and eight built-in descriptions at the same pinned commit; preserve GPL-2.0-only attribution to M-Film contributors. Short Thai translations/adaptations are recorded with the original English in research/film-labels/DETAILS_TH.json. TONEINFO.BIN contains bounded, CRC-checked offline masks shaped from Noto Sans Thai plus English reference strings. Preserve the Noto font OFL notice; TONEINFO_OFL.txt is included in the runtime data directory and the font source/license remains in assets/fonts/noto-sans-thai. No source photograph or RAW/LUT implementation is introduced.

### tone7 baked cartridge artwork, cache and palette review

M-Film's fos_shelf.c / fos_ui.c / fos_gfx.c and fork/src/filmroll.c at pinned commit 8118e9c7c072371bd53eda8920f44910e8a35baf were reviewed for cached prepainted cartridges, elapsed-frame timing, multi-level palette ramps and two-bank palette save/restore. Credit M-Film contributors; inherited label data remains GPL-2.0-only. The independent offline renderer in tools/generate_film_art.py uses the original stock label metadata, cylindrical projection, supersampling and material shading to build FILMART.BIN. Its runtime caches four checked sprites and scopes custom palette entries to 80–239; it replaces tone5/6's runtime label rendering. No upstream raster texture, sample photograph, RAW engine or LUT is imported.

Barlow Condensed Bold and ExtraBold font files are copied verbatim from the pinned M-Film tree and retain the SIL Open Font License 1.1, copyright and source notes in assets/fonts/barlow. FILMART_OFL.txt accompanies runtime artwork. Noto/TONEINFO notices continue to apply independently. Film and brand words identify source presets; there is no claim of endorsement or exact retail packaging reproduction.

### v3r6-lut1: actual M-Film RAW/LUT/JPEG processing prototype

Unlike the earlier native JPEG picker, this experiment imports M-Film's open camera development engine: fos_dev.c/.h, fos_jpeg.c/.h, fos_print.c, fos_looks.c, fos.h and fos_gfx.h verbatim from commit 3a5b0d8506d6279b1b223f78612dfa6db47405e8. Copyright M-Film contributors; GPL-2.0-only. Source hashes and the verbatim GPL license are in firmware/modules/lutjpeg/upstream. catalog.c extracts the eight builtin identity records, integer square root and fixed display colors from the same fos_film.c/fos_gfx.c; the UI exposes only Portra400/Ektar100/TriX400. The table indices match that builtin order.

Three LOOKnn.LUT files are generated from the credited builtin GPL model by fos_dev_table, reproducible with tools/bake_lutjpeg_tables.py. No external Film-Luts .cube files, upstream sample photographs/RAWs, closed-source Darkrooms code or four removed cinema print LUTs are bundled. These are artistic model looks, not measured physical-film reproductions. Native JPEG preset data used by the earlier picker remains separate.

The 600D module adapter, menu, file handling and tests are fork modifications with Codex assistance. tests/lutjpeg/mkcr2.c derives from upstream sim/tools/mkcr2.c, modified to use the 600D matrix/white level for synthetic tests; outputs are generated test charts, not real camera results. The prototype module statically links the same existing Newlib libm and GCC support archive as the matching core (local __errno renamed to lut_errno); preserve existing dependency notices and inherited corresponding-source limitations. The previous ten distributed modules are preserved from tone8.
