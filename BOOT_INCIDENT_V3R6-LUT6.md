# v3r6-lut6 boot incident — 16 November 2026

**Do not use the lut6 camera package. Its recommendation is withdrawn.** A user reports that the camera hangs immediately after switching on, before pressing any buttons; the red LED keeps blinking. Removing the card does not recover the already-hung camera; removing/reinserting the battery with the affected card absent restores normal operation. The published tag and archives remain unchanged for traceability, not as a recommendation to install them.

## กู้กลับก่อน

- ใช้กล้องโดยไม่ใส่การ์ดชุด lut6 ตามวิธีที่ผู้ใช้กู้กลับมาได้แล้ว หรือใช้การ์ดเดิมที่ทราบว่าใช้งานได้ อย่าใส่การ์ด lut6 กลับเพื่อทำให้อาการเกิดซ้ำ
- ใช้เครื่องอ่านการ์ดบน PC สำรอง DCIM และ log ก่อนแก้ไฟล์ ไม่ต้องนำการ์ดกลับเข้ากล้องเพื่อเก็บข้อมูล
- หากจะคืนรุ่นก่อน ให้คืน **autoexec.bin และ ML ทั้งชุดที่เคยบูตผ่าน** ห้ามผสม autoexec กับ overlay/โมดูลคนละรุ่น และไม่เรียก Firmware Update เพื่อแก้เหตุการณ์นี้ อย่าลบเฉพาะ autoexec.bin บนการ์ดที่ตั้ง bootable เพราะการ์ด bootable ที่ไม่มีตัวบูตอาจทำให้ค้างได้
- หาก lut5 เป็นชุดที่เคยบูตได้บนกล้องเครื่องนี้ [UPDATE lut5 เดิม](https://raw.githubusercontent.com/itsamachom/EOS_ROM_MLV_Modify/v3r6-lut5/downloads/v3r6-lut5/camera/600D-102-Thai-Menu-v3r6-lut5-UPDATE.zip) เป็นทางเลือกคืนรุ่น ไม่ใช่บิลด์แก้ lut6 และยังมีข้อผิดพลาดการแปลงภาพตามที่รายงานก่อนหน้านี้

## Evidence and limits

The immediate-on-switch report prioritizes startup investigation. It does not prove which function caused the fault: startup loads the overlay, registers callbacks, launches a monitor, and automatically loads artwork/live-colour resources. No crash dump or loader log from the affected camera has been inspected. No claim is made that an SD-card fault, ROM damage, a particular callback, or the Live View pixel loop caused this incident.

Useful existing card files: `ML/LUTLOAD.LOG`, any `CRASH*.LOG` or `ASSERT*.LOG`, and names/locations of lock files. Read existing files with a card reader; do not boot the failing build to generate a new log. Some failures may leave no crash log.

The prior host tests validated file handling, buffers and simulated callbacks, and the ARM build/relocation checks passed. They did **not** execute the whole firmware on a 600D or prove a successful boot. Those checks were insufficient to establish hardware readiness.

## Source-only defensive change after this report

The old `FILMERR.LCK` only guarded conversion jobs. The direct-loader path had no persistent protection around overlay startup or its later callbacks. The post-release source now creates `FILMBOOT.LCK` before executing validated overlay code, keeps it throughout the overlay lifetime, and removes it only after orderly successful shutdown. A remaining marker suppresses automatic overlay startup on the next boot; explicit manual retry remains possible. If the marker cannot be created, the overlay is not executed. The log now marks entry into overlay startup.

The guard does not cover crashes earlier in the core boot, does not recover an already-hung CPU, and is **not a diagnosis or a confirmed fix for this incident**. The published lut6 binaries do not contain this change. No replacement firmware is being distributed as part of this incident response.

Validation: production loader compiled/executed with ASan/UBSan and a fake ARM entry call; tests cover marker creation before entry, failed creation, failed initialization, simulated power loss without shutdown, next-boot suppression, explicit retry and clean-shutdown removal. The 600D ARM build and bound-overlay CRC/relocation checks also pass for this source-only change; no camera test has been performed.
