# Thai-Menu v3r6-lut3 — FLIM TONE submenu and direct LUT engine

รุ่นทดลอง Canon EOS600D / firmware1.0.2 แก้ข้อความ “Module startup busy; retry” ที่ผู้ใช้พบในlut2 และจัดเมนูใหม่ตามคำขอ ไม่แก้หน้าเว็บ/คู่มือ/รุ่นหลักv3r5

## เมนูใหม่

S → FLIM TONE

1. **เปิดเมนู** — เปิดหน้ากลักฟิล์ม Native JPEG เดิม
2. **LUT JPEG** — โหลดตัวแปลง RAW/LUT/JPEG และเปิดตัวเลือกการแปลง

ไม่มีLUT JPEGเป็นรายการแยกท้ายหมวดSแล้ว หากตัวแปลงยังไม่โหลด รายการที่2จะแสดงSET Load กดSETเพื่อโหลดโดยตรง เมื่อขึ้นLUT readyให้เลือกรายการLUT JPEGอีกครั้ง จะเข้าFilm LUT / Convert latest RAW / Conversion status / Cancel conversion

## อัปเดต

สำรองautoexec.binและML/เดิม ปิดกล้องรอไฟการ์ดดับ คัดลอก **autoexec.bin และ ML/ ทั้งชุด** โดยเฉพาะไฟล์ใหม่ `ML/LUTDATA/LUTJPEG.BIN`, `ML/LUTDATA/DIRECT.ON` และ `ML/modules/600D_102.sym` ต้องตรงกับโปรแกรมชุดเดียวกัน ไม่เรียกFirmwareUpdateซ้ำ

ตั้งCanonเป็นRAWหรือRAW+JPEG ถ่ายหนึ่งภาพ รอไฟการ์ดดับ เข้าS→FLIM TONE→LUT JPEG เลือกFilm LUTหนึ่งในPortra400/Ektar100/TriX400 แล้วConvert latest RAW ผลJPEG720×480อยู่ML/LUTOUT/P400, E100หรือTX400 เปลี่ยนLUTแล้วแปลงRAWใบเดิมซ้ำเพื่อเทียบสีได้ ต้นฉบับไม่ถูกแก้ไข

## แก้ตัวโหลด

lut2ยังรอmodule_startup_complete ซึ่งจากภาพผู้ใช้ไม่เคยพร้อมในเวลารอ รุ่นนี้ **ไม่มีการรอstartup moduleและไม่เรียกTCCในกล้องเพื่อโหลดLUT** ใช้bound imageที่เตรียมไว้สำหรับcore600Dชุดนี้แทน ไม่ใช่การข้ามตัวรอแล้วเรียกlinkerร่วมกับtaskเดิม

ขณะbuild:

- สร้างengineจากsourceเดิมและlinkกับที่อยู่importของcoreตรงชุด
- ตรวจARM ELF relocation: ภายในใช้base relocationเฉพาะABS32 ส่วนbranchสัมพันธ์อยู่ในภาพเดียวกัน ตรวจว่าไม่มีbranchสัมพันธ์ออกไปcoreที่ย้ายฐานไม่ได้
- แพ็กimage, ขนาดRAM/BSS, entrypoints, รายการrelocation และCRCของheader/image/relocation/core/symbol fileเป็นLUTJPEG.BIN

ขณะโหลด:

- ตรวจformat/รุ่น600D/ขนาด/entrypoint/CRCทุกส่วน รวมทั้งautoexec.binและsymbol fileที่อยู่บนการ์ด
- อ่านผ่านDMA buffer8192ไบต์ลงRAMแบบมีขอบเขต ล้างBSS ปรับเฉพาะตำแหน่งภายในที่ตรวจไว้ ไม่ปรับimportaddressของcore
- sync cachesก่อนเรียกABIและbootstrap เก็บengineไว้ในRAMจนปิดกล้อง ใช้instanceเดิมเมื่อเข้าอีกครั้ง
- DIRECT.ONทำให้startup loaderไม่autoload lutjpeg.moแม้มีlutjpeg.enตกค้างจากรุ่นก่อน ไม่เปลี่ยนการเปิด/ปิดโมดูลอื่น และไม่ลบcrash guard
- ปิดงานแปลงก่อนshutdown บันทึกข้อผิดพลาดที่ML/LUTLOAD.LOG ข้อความแจ้งสั้นลงเพื่อไม่ทับหัวเมนู

ยังแนบlutjpeg.moเป็นผลbuild/compatibility artifact แต่เส้นทางLUTของรุ่นนี้ใช้LUTJPEG.BIN และไม่ต้องเข้าหน้าModules

## แก้พาธการ์ด

พบว่าcard_info.drive_letterในAPIจริงเป็นchar* แต่adapterเดิมใช้%c จึงแก้เป็น%sทั้งพาธตารางสีและโฟลเดอร์ผลลัพธ์ ปรับfixtureให้ใช้ชนิดข้อมูลตรงจริง และเปิดตรวจformat mismatchในhost compileเพิ่ม จุดนี้ยังไม่ถึงขั้นทดสอบบนกล้องของผู้ใช้ในlut1/2เพราะติดตัวโหลดก่อน

## การตรวจ

- ARMbuild/BSSยังอยู่ต่ำกว่า0x00d00000 และlinkedmenuมีFLIM TONEสองรายการจริง
- ทดสอบproduction loader ASAN/UBSAN: header/image/relocation CRC, ขอบเขตentrypoint/relocation, duplicate relocation, โปรแกรม/symbol fileไม่ตรง, ไฟล์ขาด, short read, DMA/low memory, ABI/initfail cleanup, reuseและshutdown โดยไม่มีฟังก์ชันstartup/TCCในfixture
- ตรวจbound ELFและจำลองปรับฐานที่heapสามตำแหน่ง: internal pointersย้ายครบ, importคงที่, branchอยู่ในภาพ, BSSเป็นศูนย์ การตรวจนี้ยังไม่ใช่การexecuteARMบน600D
- ทดสอบproduction RAW/LUT/JPEGและadapterกับRAWสังเคราะห์หลังแก้drive-letter path ผ่าน ภาพสามโทนต่างกันตามเดิม
- ทดสอบเมนูOpen menu/LUT JPEG, การเปิดpicker, การเข้าsubmenuengineหลังโหลด และnativeJPEG regression ผ่าน
- packageตรวจCRC/XOR/versionและbound imageตรงกับautoexec/symbol file/source ชุดเดียวกัน

**ยังไม่ทดสอบlut3บน600DจริงหรือQEMU** วิธีโหลดแบบใหม่นี้ต้องทดลองบนกล้องเพื่อยืนยันการexecuteที่ฐานRAMจริง รวมทั้งRAMประมาณ5.5MBที่ใช้ตอนแปลงและเวลาจริง ไม่อ้างว่าทดสอบฮาร์ดแวร์สำเร็จแล้ว ผลลัพธ์ยัง720×480/3โทน/กดแปลงเอง ไม่เพิ่มคิวอัตโนมัติหรือLiveView LUT

[ภาพเมนูไทยจากproductionrendererบนhost](previews/jpeg-tones/film-menu-th.png) ใช้ค่าระบบจำลอง ไม่ใช่ภาพจากกล้องจริง

เครดิต/สิทธิ์ตามTHIRD_PARTY_NOTICES.md, AI_ASSISTANCE.mdและprovenanceของM-FilmGPL-2.0-only ข้อค้างinheriteddietlibc sourceและpic_view importเดิมยังคงอยู่
