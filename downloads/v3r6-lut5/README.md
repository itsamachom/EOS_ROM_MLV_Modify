# Thai-Menu v3r6-lut5 — Automatic Film Lab

**ทดลองสำหรับ Canon EOS 600D / Canon firmware 1.0.2 ยังไม่ทดสอบบนกล้องจริงหรือ QEMU**

[ดาวน์โหลด UPDATE สำหรับกล้อง/การ์ดที่มี ML แล้ว](https://raw.githubusercontent.com/itsamachom/EOS_ROM_MLV_Modify/v3r6-lut5/downloads/v3r6-lut5/camera/600D-102-Thai-Menu-v3r6-lut5-UPDATE.zip) — ไม่มี FIR ไม่เรียก Firmware Update

[ซอร์สตรงกับบิลด์](https://raw.githubusercontent.com/itsamachom/EOS_ROM_MLV_Modify/v3r6-lut5/deliveries/photo-ui-v3r6-lut5-full/600D-PHOTO-UI-v3r6-lut5-source.zip) · [SHA256](SHA256SUMS)

สำรองชุดเดิมก่อน แล้วคัดลอก autoexec.bin และ ML/ ทั้งชุดจาก UPDATE เดียวกัน เปิดกล้องแล้วตัวแปลงเริ่มเอง เข้า S → FLIM TONE เลือกหมวด/โทนแล้วถ่าย ระบบขอเปิด RAW+JPEG เมื่อข้อมูลกล้องพร้อม และเข้าคิวแปลง RAW ใหม่ให้เอง ไม่ต้องโหลดโมดูลหรือกดแปลงทุกภาพ

มี 64 สูตรจากแพ็ก Caduceus ปัจจุบัน + 64 รายการ Standard รุ่นเก่า และ 3 สูตร lut4 เดิม ตั้งต้น JPEG ครึ่งขนาด RAW ประมาณ 2592×1728 หาก RAM พอ หน้าสถานะแจ้งขนาดจริงหากต้องลดลง

[ดูหน้าสถานะเคลื่อนไหว](../../previews/film-lab/progress/film-progress-demo.gif) · [รูปตัวอย่างและค่าครบทุกสูตร](../../research/caduceus/GALLERY_TH.md) · [คู่มือและข้อจำกัด](../../PHOTO_UI_V3R6-LUT5.md)

ภาพแปลงใหม่อยู่ ML/LUTOUT/Lxxx ปุ่ม Playback ของ Canon ยังเป็น JPEG เดิม สีพอร์ตเป็นโมเดล Canon ไม่ใช่การยืนยันว่าสีเหมือน Olympus 1:1 รายละเอียดความต่างของ tone/FX/NR และเครดิตครบในคู่มือ
