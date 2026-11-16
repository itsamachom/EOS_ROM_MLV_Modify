# Thai-Menu v3r6-lut4 — experimental three-look test

**Public Experimental — รุ่นทดลอง ยังไม่ยืนยันบนกล้องจริง** สำหรับ Canon EOS 600D / Canon firmware 1.0.2 เท่านั้น
ฐาน: v3r6-lut3 (`43f95e41a55e3d416ca74c064be92e465dbbd0f4`)

## สิ่งที่เปลี่ยน

- S → FLIM TONE → เปิดเมนู แสดงเพียง **Portra 400 / Ektar 100 / Tri-X 400** ในแถวเดียว มีตัวนับ 1/3–3/3 ไม่มีแท็บหมวดที่ว่าง
- เก็บคลัง 72 สูตร รูปกลัก รายละเอียด และการคืนค่าเดิมไว้ครบ ไม่ลบสูตรเก่า
- เปิดคลังเดิมกลับได้ด้วยการเปลี่ยนค่าเริ่มต้น `PHOTO_TONE_FULL_LIBRARY` เป็น 1 ใน `firmware/src/photo-tone-600d.h` แล้ว clean build ใหม่ การทดสอบ host รองรับ `PHOTO_TONE_FULL_LIBRARY=1 bash tests/run-jpeg-tone-tests.sh`
- S → FLIM TONE → LUT JPEG มีสามสูตรเดียวกัน ปรับตารางสีใหม่ด้วย colour matrix, tone curve, selective colour และ split colour แบบออฟไลน์ ไม่เพิ่มงานคำนวณ stage เหล่านี้ขณะแปลงในกล้อง
- เพิ่ม `RECIPES.json` ที่มีค่าจริงและเก็บพร้อม manifest บนการ์ด เปลี่ยน recipe identity ของ engine เพื่อปฏิเสธตาราง lut3/ตารางคนละ revision แทนการใช้ตารางเก่าเงียบ ๆ

## สีใหม่ทำงานที่ไหน

**เปิดเมนู** ยังคงใช้สูตร native JPEG เดิม: เปลี่ยน ISO จริง, WB และ Picture Style ตามหน้ารายละเอียด กด SET เปิดรายละเอียด กด SET อีกครั้งจึงใช้ Q คืนค่าที่สำรองใน RAM ไม่ได้ใส่ LUT ใหม่ให้ JPEG ที่ Canon ถ่าย

**LUT JPEG** ใช้ RAW หลังถ่าย สร้าง JPEG ใหม่ 720×480 โดยไม่เปลี่ยน ISO/WB/Picture Style ของกล้องและไม่เขียนทับ RAW ต้นฉบับ ไม่มี Live View LUT หรือคิวแปลงอัตโนมัติ

ตารางเป็น 33×33×33 บนโดเมน Cineon ของ engine เดิม ใช้ M-Film model เป็นฐาน แล้วปรับค่าผลลัพธ์ RGB ของโมเดลก่อนบันทึกตาราง:

1. ถอด sRGB ของผลโมเดล → ผสมด้วยเมทริกซ์ 3×3 ที่ผลรวมแต่ละแถวเป็น 1 → กลับ display RGB นี่คือ linearized model-output ไม่ใช่ sensor-linear RAW
2. ปรับเส้นความสว่างแบบ monotonic โดยบวก delta ร่วมทุกช่อง
3. ปรับ hue เป็นองศา, saturation เป็นตัวคูณ และ brightness เป็น delta ของ **HSV value** ใน 12 กลุ่มสีที่ผสมกันอย่างนุ่มนวล ลดผลใกล้สีเทา ไม่ใช่หน่วย HSL ของ Olympus
4. เติมสีเงา/ไฮไลต์อย่างอ่อนสำหรับสูตรสี ส่วน Tri-X บังคับสามช่องเท่ากัน

Portra: ลดความอิ่มสีเล็กน้อย ยกดำและผ่อนไฮไลต์ ผิวสว่างขึ้นเล็กน้อย; Ektar: เพิ่มการแยกสีและความเข้มฟ้า/เขียว เพิ่ม contrast อย่างพอประมาณ; Tri-X: ปรับ toe/contrast/shoulder โดยคงขาวดำ ไม่มีการอ้างว่าตรงฟิล์มจริงหรือสี Olympus

แนวคิดอ้างอิง [Caduceus E-P7](https://github.com/cakelesscoder/caduceus-ep7/tree/a0c351f5398a4e7b69738103bda3e5c92f81971d) เรื่องแยก matrix/curve/รายสี แต่เขียนสูตรและโค้ดใหม่ ไม่คัดลอกโค้ด Apache-2.0, ค่าสูตร, ตารางจาก firmware หรือแพตช์ Olympus เข้าส่วน GPLv2 นี้ Spatial grain/halation ยังคงเป็นพฤติกรรมของ engine ฐาน ไม่ได้เพิ่มเอฟเฟกต์เหล่านี้ลง LUT

## วิธีทดสอบบนกล้อง

1. สำรอง `autoexec.bin` และ `ML/` ชุดเดิม ปิดกล้องและรอไฟการ์ดดับ
2. ใช้ **UPDATE** กับกล้อง/การ์ดที่มี ML ใช้งานอยู่แล้ว คัดลอก `autoexec.bin` และ `ML/` ทั้งชุดจากแพ็กเดียวกัน ห้ามผสม `LUTJPEG.BIN`, `600D_102.sym` หรือตารางจาก lut3 ไม่มี FIR ในชุด UPDATE และไม่เรียก Firmware Update
3. ตั้ง RAW หรือ RAW+JPEG ถ่ายหนึ่งภาพ รอเขียนการ์ดเสร็จ เข้า S → FLIM TONE → LUT JPEG โหลด engine ตามเดิม แล้วเลือก Film LUT และ Convert latest RAW
4. แปลง RAW ใบเดิมทั้งสามสูตร ผลอยู่ `ML/LUTOUT/P400`, `E100`, `TX400` เก็บไฟล์ RAW/JPEG ต้นฉบับสำหรับเทียบ ตรวจสีผิว ท้องฟ้า ต้นไม้ เงาและไฮไลต์
5. หากโหลด/แปลงล้มเหลว บันทึกข้อความ Conversion status และ `ML/LUTLOAD.LOG` ก่อนคืนชุดสำรอง อย่าลบ crash guard เพื่อบังคับผ่าน

## การตรวจและข้อจำกัด

- Host ASAN/UBSAN: เมนู 3 สูตรและการเปิดคลัง 72 สูตรกลับ, bounds ของภาพไทย/อังกฤษ, SET ยืนยัน, MENU ย้อนกลับ, Q restore และ palette cleanup
- Recipe math: identity/neutral matrix, tone ramp, monochrome, hue wrap/gamut และ manifest; rebake ต้องได้ LUT และ recipe identity เดิม
- Production RAW/LUT/JPEG บน RAW สังเคราะห์: JPEG ทั้งสามต่างกัน, Tri-X เป็นขาวดำ, RAW คงเดิม, cancel/I/O/low memory/corrupt/mismatched table
- ARM build และ bound-image relocation/CRC/core/symbol matching พร้อมตรวจ bindings และโมดูลเกมใน ZIP

ผลภาพ `previews/lutjpeg/*.JPG` มาจาก RAW สังเคราะห์ ไม่ใช่ภาพถ่ายจริง ภาพเมนูอยู่ `previews/lutjpeg/v3r6-lut4-menu/` เป็น production renderer บน host ไม่ใช่ screenshot จากกล้อง

**ยังไม่ทดสอบ lut4 บนกล้องจริงหรือ QEMU** การ execute payload ที่ฐาน RAM จริง เวลาแปลง การใช้ RAM บนกล้อง และความแม่นยำของสียังต้องทดลอง ผล build/host ไม่ยืนยันพฤติกรรมฮาร์ดแวร์ ข้อค้าง corresponding source ของ inherited dietlibc และ pic_view import เดิมยังคงอยู่ตามเอกสารฐาน

ทำซ้ำ: `python3 tools/bake_lutjpeg_tables.py` แล้ว `bash scripts/build-photo-ui.sh` (ตรวจ rebake ก่อน build) ชุด RAW host tests ต้องมี NumPy และ Pillow; ใช้ Python venv ของ cloud ที่เตรียมไว้ แพ็กชุดทดลองด้วย `python3 tools/package_photo_ui_current.py` หลังตรวจและเพิ่มไฟล์ซอร์สใหม่เข้า Git index เพื่อให้ source ZIP ครบ

การพัฒนาครั้งนี้มี Codex/LLM ช่วยสร้างโค้ด สูตร การทดสอบ และเอกสาร ไม่ได้ส่ง upstream และไม่อ้างเป็นงานที่ upstream รับรอง
