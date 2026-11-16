# v3r6-lut7 Recovery — Canon EOS 600D / firmware 1.0.2

**UPDATE สำหรับกู้กลับจาก lut6 ที่มีรายงานค้างตั้งแต่เปิดสวิตช์ ไม่ใช่รุ่นเปิดฟีเจอร์ฟิล์มใหม่** ยังไม่มีผลทดสอบการบูตบนตัวกล้องจริง จึงไม่รับรองว่าแก้ต้นเหตุได้แน่นอน

## เปลี่ยนอะไร

- ย้อนการเชื่อมปุ่ม เมนู และ display filter ใน core กลับไปฐาน lut5 ที่ผู้ใช้เคยเปิดได้ ไม่เรียก bridge/Live View/result viewer ที่เพิ่มใน lut6
- ปิด film engine ทั้งการเริ่มตอนบูตและการกดเมนูใน recovery core ไม่มีการอ่าน LUT/ภาพกระป๋อง/ภาพล่าสุด หรือเปลี่ยนคุณภาพภาพผ่านเส้นทางนี้
- ตัว module เองปฏิเสธ initialization ก่อนเพิ่มเมนู สร้าง monitor อ่านตั้งค่า หรือเริ่มแปลง เพื่อไม่ให้ไฟล์ .en เก่าหรือการเรียกโดยตรงข้ามข้อจำกัดของ core ได้
- เก็บ persistent startup/session guard ที่เพิ่มหลังรายงานเหตุไว้ในโค้ด loader ชั้นล่าง แต่ public entry ของรุ่น Recovery ไม่เรียก loader นั้น
- ภาพ/RAW เดิมบนการ์ดไม่ถูกลบหรือแปลง ไม่เปิด Live View LUT, JPEG อัตโนมัติ หรือ ML film viewer ในรุ่นนี้ คลังสูตรและโค้ดฟิล์มยังเก็บเพื่อพัฒนาภายหลัง

ไม่ใช่การแก้ can't write the print หรือรับรอง Canon Playback/การส่งภาพ และยังไม่มี log จากกล้องที่ค้างเพื่อยืนยันต้นเหตุ ดู BOOT_INCIDENT_V3R6-LUT6.md

## ติดตั้ง UPDATE ผ่านเครื่องอ่านการ์ด

1. ปิดกล้องและไม่ใส่การ์ด lut6 ระหว่างเตรียมชุดใหม่ ถ้าระบบค้าง ให้ใช้ขั้นตอนถอดแบตพร้อมเอาการ์ดออกที่กู้กล้องกลับมาได้แล้วก่อนหน้า
2. บน PC สำรอง DCIM, ชุดโปรแกรมเดิม และ ML/LUTLOAD.LOG/CRASH*.LOG/ASSERT*.LOG ที่มีอยู่ก่อนคัดลอก ไม่ต้องเปิดกล้องซ้ำเพื่อสร้าง log
3. แตก UPDATE ZIP แล้วคัดลอก **autoexec.bin และ ML/ ทั้งชุด** ไปยังรากการ์ด ให้ทับไฟล์เดิม อย่าคัดลอกเฉพาะ autoexec.bin และอย่าลบ autoexec.bin บนการ์ด bootable โดยไม่มีตัวใหม่แทน
4. ไม่ต้องเรียก Firmware Update และ UPDATE นี้ไม่มี ML-SETUP.FIR
5. ชื่อเวอร์ชันต้องเป็น PHOTO-UI-v3r6-lut7.600D.102 รายการ Film lab แจ้ง Recovery update: film engine disabled เป็นพฤติกรรมที่ตั้งใจไว้ ใช้ถ่ายภาพด้วยระบบ Canon ตามปกติ

หากยังค้าง ให้หยุดทดลองการ์ดใบนั้นและใช้กล้องโดยไม่มีการ์ดชุดนี้ตามวิธีเดิม ไม่ต้องพยายามเปิดซ้ำหลายครั้ง

## ตรวจแล้ว

- ARM build core, module และ bound overlay ผ่าน ตรวจ CRC/relocation/core identity
- รัน production entry บน host พร้อม ASan/UBSan: ทั้ง auto/manual entry ไม่เรียก engine แม้มี/ไม่มี lock เก่า; module entry ปฏิเสธโดยไม่สร้างเมนู งานเบื้องหลัง อ่าน/เขียนไฟล์ หรือเปลี่ยนคุณภาพภาพ
- ตรวจคำสั่ง ARM ของ lutjpeg_bootstrap จริงเป็น mov r0,#1; bx lr และไม่มี film loader task หรือ hook ปุ่ม/วาด/ฟิลเตอร์ lut6 ใน symbol table ของ core
- ชุดทดสอบเมนูไทย/อังกฤษ, native JPEG controls, binding และ packaged games ผ่าน
- ตรวจ ZIP CRC, SHA256, source archive ตรงไฟล์จริง และไม่มี FIR ใน UPDATE

คำสั่ง: bash scripts/build-photo-ui.sh; python tests/check_boot_recovery.py; python tests/check_lutjpeg_overlay.py; bash tests/run-photo-menu-tests.sh; bash tests/run-jpeg-tone-tests.sh; python tests/check_photo_menu_bindings.py; python tests/check_packaged_games.py

Host tests ของฟีเจอร์ lut6 ที่ถูกปิดเก็บไว้เป็นประวัติ ไม่ได้อ้างว่าผ่านสำหรับรุ่นนี้ ไม่มีผล QEMU หรือ hardware boot test ของ lut7
