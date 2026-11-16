# Thai-Menu v3r6-lut5 — automatic film lab

**Public Experimental: Canon EOS 600D / Canon firmware 1.0.2 เท่านั้น ยังไม่ยืนยันบนกล้องจริงหรือ QEMU**

## ใช้งานแบบใหม่

1. อัปเดต `autoexec.bin` และ `ML/` **ทั้งชุด** จาก UPDATE ที่ตรงกัน แล้วเปิดกล้อง ตัวแปลงจะเริ่มเอง ไม่ต้องเปิดโมดูล
2. เข้า **S → FLIM TONE** จะเข้าตัวเลือกฟิล์มโดยตรง เลือก **หมวดฟิล์ม / เลือกโทนฟิล์ม**
3. **แปลงหลังถ่าย = Automatic** เป็นค่าเริ่มต้น ระบบขอเปลี่ยน JPEG-only หรือ mRAW/sRAW เป็น **RAW+JPEG ขนาดเต็ม** หนึ่งครั้งเมื่อข้อมูลกล้องพร้อม ไม่เปลี่ยน ISO/WB/Picture Style ระหว่างเลือกสูตร หาก Canon ปฏิเสธ ให้ใช้ P/Tv/Av/M และรายการเปิด RAW พร้อม JPEG
4. ถ่ายแล้วรอการเขียน RAW เสร็จ ภาพเข้าคิวและแปลงเอง หน้าสถานะแสดงขั้นตอน เปอร์เซ็นต์ จำนวนภาพรอ และพื้นหลังแถบฟิล์มยาวเคลื่อนไหว ไม่ต้องหา `Convert latest RAW` อีก
5. ภาพใหม่อยู่ **`ML/LUTOUT/Lxxx/LUT_0001.JPG`** โดย xxx เป็นรหัสสูตรใน `ML/LUTDATA/caduceus/CATALOG.json` ตัวอย่าง Portra 400 จาก Standard 1.2 อยู่ L017 ภาพ RAW และ Canon JPEG เดิมยังอยู่ DCIM ตามเดิม

**ปุ่ม Playback ของ Canon ยังแสดง Canon JPEG เดิม** รุ่นนี้ไม่แทนที่ภาพเดิมและไม่ลงทะเบียน JPEG ใหม่ในฐานข้อมูล Playback ของ Canon เปิดภาพที่แปลงแล้วจากโฟลเดอร์ ML/LUTOUT บนคอมพิวเตอร์ ยังไม่มี Live View LUT

เลือก **แปลงภาพล่าสุดอีกครั้ง** ได้เมื่อต้องการลองสูตรอื่นกับ RAW ล่าสุด ส่วนการถ่ายปกติไม่ต้องกดรายการนี้ ตั้งค่าเลือกสูตร/อัตโนมัติ/ขนาดไว้ใน `ML/SETTINGS/FILMLAB.CFG`

## สูตรทั้งหมด

[แกลเลอรีครบทุกแพ็ก พร้อมรูปและค่าทั้งสูตร](research/caduceus/GALLERY_TH.md)

นำเข้า JSON ต้นฉบับจาก Caduceus E-P7 commit `a0c351f5398a4e7b69738103bda3e5c92f81971d` ทั้ง 9 แพ็กโดยไม่ตัดฟิลด์:

| หมวด | รายการ |
|---|---:|
| Standard 1.2 | 32 |
| Tokyo Days | 5 |
| Tokyo Nights | 6 |
| Summer | 5 |
| Autumn | 5 |
| Winter | 5 |
| Film Noir | 6 |
| Standard 1.1 (รุ่นเก่า) | 32 |
| Standard 1.0 (รุ่นเก่า) | 32 |

รวม **64 รายการในแพ็กปัจจุบัน + 64 รายการรุ่นเก่า** ไม่ใช่ชื่อฟิล์มต่างกัน 128 ชื่อ ยังเก็บ Portra/Ektar/Tri-X แบบ lut4 ไว้ในหมวด Native / lut4 อีก 3 รายการ รวมเลือกได้ 131 รายการ ไม่ลบคลัง Native 72 สูตรเดิมในซอร์ส

ไฟล์ JSON, LICENSE และ NOTICE ของ Caduceus อยู่แยกใน `ML/LUTDATA/caduceus/` พร้อมต้นทางและ SHA256 ข้อมูลสูตรเป็น Apache-2.0 ส่วนตัวประมวลผล GPL คงใบอนุญาตเดิม ไม่ได้นำโค้ด TypeScript/Python หรือ firmware Olympus มาเชื่อมเข้าตัวประมวลผล GPLv2 นี้ ภาพตัวอย่างเป็นภาพ CC0 และคงเครดิตภาพครบ

## ความหมายของการพอร์ตสูตร

**สูตรถูกเก็บครบ แต่ผลยังเป็นโมเดล Canon ไม่ใช่การยืนยันว่าสีตรง E-P7 หรือฟิล์มจริง 1:1** เซนเซอร์ เมทริกซ์ RAW และระบบ JPEG ของสองกล้องต่างกัน ต้องถ่าย ColorChecker/ผิวคน/กลางวัน/กลางคืนเพื่อสอบเทียบจริง

- WB shift, matrix 3×3, Hue/Saturation/Lightness 12 กลุ่ม, tone และ split tone ใช้ค่าของแต่ละสูตร ผ่านตัวแปลความหมายที่เขียนใหม่สำหรับ Canon แล้ว bake เป็นตาราง 33³ บนโดเมน Cineon ของ engine
- Tone ใช้การปรับช่วงความสว่างอย่างนุ่มนวล และ hue ใช้หน่วยของโมเดล Canon ไม่ใช่การเรียก LUT ของ Olympus หาก tone ปัจจุบันเป็นศูนย์ จะนำคำแนะนำ tone ในสูตรรุ่นเก่ามาประกอบด้วย
- Base curve Auto ใช้ shoulder จำลอง ไม่ใช่ Gradation Auto ของ E-P7; Bloom/tinted glow และ Clarity ใช้การนุ่ม/เพิ่ม local contrast ตามแนวนอนของภาพใน engine นี้ ไม่ใช่การเบลอ 2 มิติของ Olympus
- ฟิลด์ `grain` ในแค็ตตาล็อก E-P7 รุ่นนี้หมายถึงคำแนะนำ Noise Reduction ไม่ใช่ความแรงเม็ดฟิล์ม จึงเก็บไว้ครบแต่ไม่แปลงเป็น grain ปลอม ไม่มีพอร์ต NR ของ Olympus
- ทุกสูตรที่นำเข้ามี Art Effect = off และ Vignette = 0 ไม่ได้อ้างว่าพอร์ตเอฟเฟกต์ที่อยู่นอกข้อมูลชุดนี้แล้ว
- ภาพแกลเลอรีเป็นตัวอย่าง desktop บน JPEG CC0 ไม่ใช่หลักฐานสีจาก 600D และไม่รวม spatial FX/NR

## ขนาดภาพและทรัพยากร

ค่าเริ่มต้น **Large: ครึ่งหนึ่งของ RAW แต่ละด้าน** ประมาณ 2592×1728 สำหรับภาพ 3:2 ปกติ ขนาดจริงยึดข้อมูล crop/orientation ของ RAW ไม่ยืดภาพ 3:2 ให้เป็น 16:9

ทดสอบ RAW สังเคราะห์ 600D fixture แล้วได้ **2601×1732** ที่เปิดอ่าน JPEG ได้จริงทั้งสีและขาวดำ ใช้ processing allocation สูงสุดประมาณ **33.34 MB บน host** ไม่ใช่ผลวัด RAM หรือเวลาในกล้องจริง

หาก RAM ไม่พอ engine ลดลงเป็น Medium ประมาณ 1728×1152 แล้วจึง Small 720×480 หน้าสถานะแจ้งขนาดจริงและ Memory: smaller output ไม่อ้างว่าได้ FHD หากลดขนาดแล้ว ตัวเลือก Medium/Small เหมาะกับการทดสอบประสิทธิภาพ

## คิวและการคืนสภาพ

- ไม่แปลงภาพเก่าที่ค้างอยู่ตอนเปิดกล้อง; จับหมายเลขภาพใหม่ รวมกรณีหมายเลขกระโดดใน burst
- เก็บสูตรที่เลือกไว้กับแต่ละงาน เปลี่ยนสูตรระหว่างรอไม่เปลี่ยนสีของภาพที่เข้าคิวไปแล้ว
- รอ Canon หยุดเขียนและขนาด RAW คงที่อย่างน้อย 500 ms; หากไม่มี RAW จะแจ้งหลังเวลารอ ไม่วนแปลงไฟล์ว่าง
- คิวสูงสุด 16 งาน ถ้าเต็มจะแสดงจำนวน overflow ต้นฉบับยังอยู่ แต่รายการล้นต้องลองแปลงใหม่เอง ไม่ใช่ระบบ burst ไม่จำกัด
- Half-shutter/งานถ่ายใหม่พักการแปลงแล้วเข้าคิวใหม่ การยกเลิกจะล้างทั้งงานปัจจุบันและคิว; shutdown รอปิดไฟล์และคืนหน่วยความจำ
- JPEG เขียนลง TMP ก่อน rename และหลบชื่อที่มีอยู่ ไม่เขียนทับ RAW/Canon JPEG
- `ML/LUTDATA/FILMERR.LCK` ป้องกันเริ่มอัตโนมัติซ้ำหลังแปลงค้าง/กล้องดับระหว่างงาน ให้ตรวจข้อความแล้วกด retry ใน FLIM TONE เพื่อกู้คืน ไม่ลบ crash guard ของ Modules

## การทดสอบ

ตรวจ boot initializer จริง, direct loader/CRC/relocation/core-symbol matching, marker ป้องกันเริ่มซ้ำ, API ของเมนู, คิวอัตโนมัติ, RAW stable, สูตรที่ snapshot, overflow, cancel, การบันทึกการตั้งค่าและ I/O fault ผ่าน host tests พร้อม ASAN/UBSAN

ตรวจข้อมูลครบ 9 แพ็ก/128 สูตร, หมวดและดัชนี, output finite/gamut/mono, การทำงานแยกของ WB/matrix/HSL/tone/split และ hash ของ 131 LUT ตรวจ rebake ได้เหมือนเดิมก่อน ARM build

หน้าสถานะใช้ production renderer ทดสอบกรอบ 720×480 ทั้งไทย/อังกฤษและแอนิเมชันแถบฟิล์ม [ดูตัวอย่าง](previews/film-lab/progress/film-progress-demo.gif) เป็นสถานะจำลองบน host ไม่ใช่ screenshot หรือเวลาวัดจากกล้อง

ยังต้องทดสอบจริงบน 600D: การเริ่มอัตโนมัติและ property RAW+JPEG, RAM/เวลาของ Large, การถ่ายต่อระหว่างแปลง, การคืนหน้าจอ Canon และการสอบเทียบสี ไม่มีหลักฐานว่าทดสอบฮาร์ดแวร์ผ่านแล้ว ข้อค้าง inherited dietlibc source และ pic_view เดิมยังคงอยู่ตามเอกสารฐาน

## ทำซ้ำสำหรับพัฒนา

ติดตั้ง `tools/requirements-thai.txt` ใน venv พร้อม GCC/ARM toolchain/docutils ตาม setup เดิม

```sh
python tools/import_caduceus_catalog.py --source /path/to/pinned/caduceus-ep7
python tools/bake_lutjpeg_tables.py
bash scripts/build-photo-ui.sh
python tests/check_caduceus_catalog.py
python tests/check_film_autostart.py
python tests/run-lutjpeg-tests.py
python tests/check_module_recovery.py
python tests/check_lutjpeg_overlay.py
bash tests/run-photo-menu-tests.sh
bash tests/run-jpeg-tone-tests.sh
python tools/render_film_progress.py
```

การพัฒนานี้มี Codex/LLM ช่วยเขียนตัวแปลสูตร ระบบคิว เมนู renderer การทดสอบ และเอกสาร ไม่ได้อ้างว่าเป็นโค้ดที่ Magic Lantern หรือ Caduceus รับรอง
