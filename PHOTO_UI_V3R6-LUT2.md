# Thai-Menu v3r6-lut2 — Direct LUT loading and missing Modules page recovery

แก้กรณี S → LUT JPEG ขึ้นให้เปิดโมดูล แต่หน้า + → Modules ไม่มีรายการให้เข้า ใช้กับCanonEOS600D/firmware1.0.2เท่านั้น รุ่นทดลอง ไม่แก้หน้าเว็บ/รุ่นหลักv3r5

## วิธีอัปเดตและเปิด

1. สำรองชุดเดิม ปิดกล้องรอไฟการ์ดดับ แล้วคัดลอก **autoexec.bin และ ML/ ทั้งชุด** ต้องได้ lutjpeg.mo และ 600D_102.sym รุ่นเดียวกับโปรแกรม ห้ามอัปเดตเฉพาะautoexecหรือเฉพาะโมดูล
2. เปิดกล้อง เข้า **S → LUT JPEG** ตามเดิม
3. หากระบบยังไม่โหลด จะเห็น **LUT JPEG loader / SET Load** ให้กดSETเพื่อโหลดจากเมนูนี้โดยตรง ไม่ต้องเข้าหน้าModulesและไม่ต้องกดยืนยันซ้ำ
4. เมื่อขึ้น **LUT ready** กลับเข้า S → LUT JPEG อีกครั้ง จะเห็น Film LUT / Convert latest RAW / Conversion status / Cancel conversion
5. ตั้งCanonเป็นRAWหรือRAW+JPEG ถ่ายหนึ่งภาพ รอไฟการ์ดดับ เลือกหนึ่งในสามโทนแล้ว Convert latest RAW เช่นเดียวกับlut1 ภาพJPEGอยู่ ML/LUTOUT/P400, E100 หรือTX400

หากปุ่มโหลดรายงานไฟล์ขาด ให้คัดลอกML/จากZIPทั้งชุดใหม่ หากรายงานload/link failed ให้เก็บ `ML/LUTLOAD.LOG` และข้อความconsoleไว้ตรวจต่อ ไม่ต้องลบLOADING.LCKหรือปิดcrash guardเพื่อฝืนโหลดโมดูลทุกตัว

## การแก้โค้ด

- เพิ่มloaderในcoreที่ลงทะเบียนเมนูShootเสมอ PhotoUIเลือกengineที่ลงทะเบียนแล้วก่อน หากยังไม่มีจึงใช้loader ไม่ตกไปข้อความUnavailableทั่วไป
- รอstartup module loaderเสร็จก่อนโหลดเอง และจำกัดเวลารอ ตรวจว่ามี lutjpeg.mo / 600D_102.sym จากนั้นใช้module_loadเพื่อเชื่อมLUTแยกจากชุดautoload
- ตรวจABI LUT2ก่อนเรียกbootstrap เมนูใหม่เริ่มต้นแบบidempotent ใช้instanceที่มีอยู่ได้ ไม่ลงทะเบียนซ้ำ เก็บhandleตลอดการใช้และปิดงานแปลงก่อนshutdown
- บันทึกผลโหลดที่ML/LUTLOAD.LOG แยกไฟล์โมดูลขาด, symbol fileขาด, startupยังไม่เสร็จ, linkfail, โมดูลรุ่นเก่าและinitfail
- แก้branchstartupเมื่อมีLOADING.LCK/crash guard: ยังข้ามการโหลดเช่นเดิม แต่สแกนแบบlist-onlyและปรับรายการเมนูด้วย จึงตรวจรายการได้โดยไม่เปิดการรันโมดูลที่ถูกข้าม ไม่ลบล็อกและไม่เปลี่ยนค่าปิดautoloadของผู้ใช้
- อาการบนการ์ดจริงของผู้ใช้ยังไม่ได้ตรวจlog จึงไม่ได้ยืนยันว่ามีLOADING.LCKจริง เพียงแก้เส้นทางโค้ดที่ทำให้รายการหายและเพิ่มทางเข้าอิสระจากหน้านั้น

## ผลตรวจและขอบเขต

ผ่านASAN/UBSANของตัวloaderและengine/adapter, ข้อผิดพลาดแต่ละชนิด, reuseinstance, ABI/initfail cleanup, shutdown, การเลือกfallback→engineในPhotoUI และเรียกloaderโดยไม่เพิ่มconfirmation ผ่านfixtureที่คอมไพล์branchstartupจริง: crash guardเรียกlist-only ไม่โหลดโค้ดโมดูล ส่วนnormalboot/disableautoload/ignorecrashยังทำตามค่าที่ตั้ง

ผ่านARMbuild, linkedmenubindings, resolveimports, packageversion/CRC/XOR/sourceidentity BSS0x00cfe900ยังต่ำกว่า0x00d00000 สามLUTและอัลกอริทึมภาพไม่เปลี่ยนจากlut1

ยังไม่ได้ทดสอบlut2บน600DจริงหรือQEMU ผลภาพยัง720×480และต้องกดแปลงเอง3โทน Portra400/Ektar100/TriX400 ไม่ได้เปิดคิวอัตโนมัติหรือเพิ่มความละเอียด ดูPHOTO_UI_V3R6-LUT1.mdสำหรับข้อจำกัดและผลทดสอบภาพสังเคราะห์

เครดิตM-FilmGPL-2.0-onlyและสิทธิ์/ข้อค้างdependencyตามTHIRD_PARTY_NOTICES.md ความช่วยเหลือLLMเปิดเผยในAI_ASSISTANCE.md
