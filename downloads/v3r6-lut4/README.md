# Thai-Menu v3r6-lut4 — Public Experimental

Canon EOS 600D / Canon firmware 1.0.2 เท่านั้น ยังไม่ยืนยันบนกล้องจริงหรือ QEMU

- [UPDATE สำหรับกล้อง/การ์ดที่มี Magic Lantern อยู่แล้ว](https://raw.githubusercontent.com/itsamachom/EOS_ROM_MLV_Modify/v3r6-lut4/downloads/v3r6-lut4/camera/600D-102-Thai-Menu-v3r6-lut4-UPDATE.zip) — ไม่มี FIR ไม่เรียก Firmware Update
- [FIRST-INSTALL](https://raw.githubusercontent.com/itsamachom/EOS_ROM_MLV_Modify/v3r6-lut4/downloads/v3r6-lut4/first_install/600D-102-Thai-Menu-v3r6-lut4-FIRST-INSTALL.zip) — อ่าน INSTALL_TH.md ก่อนติดตั้งครั้งแรก
- [ซอร์สตรงกับบิลด์](https://raw.githubusercontent.com/itsamachom/EOS_ROM_MLV_Modify/v3r6-lut4/deliveries/photo-ui-v3r6-lut4-full/600D-PHOTO-UI-v3r6-lut4-source.zip)
- [รายละเอียดและวิธีทดสอบ](../../PHOTO_UI_V3R6-LUT4.md) · [SHA256](SHA256SUMS)

เมนูฟิล์มเหลือ Portra 400, Ektar 100, Tri-X 400 และเก็บคลัง 72 สูตรไว้เปิดกลับในอนาคต สี LUT ใหม่ใช้แปลง RAW หลังถ่ายเป็น JPEG 720×480 ไม่ใช่ Live View LUT และไม่เปลี่ยน JPEG ที่ Canon ถ่ายโดยตรง

สำรองชุดเดิมก่อนอัปเดต คัดลอก autoexec.bin และ ML/ ทั้งชุดจากแพ็กเดียวกัน ไม่ผสมไฟล์ lut3 กับ lut4 ไม่เขียนทับ RAW ต้นฉบับ ผล host/ARM build ไม่ยืนยันพฤติกรรมฮาร์ดแวร์ ดูข้อจำกัดและเครดิตในเอกสารที่แนบ
