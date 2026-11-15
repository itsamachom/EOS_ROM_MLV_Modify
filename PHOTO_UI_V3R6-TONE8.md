# Thai-Menu v3r6-tone8 — Complete active-tone heading and recipe coverage audit

รุ่นทดลองสำหรับCanon EOS600D/Canonfirmware1.0.2 แจกดาวน์โหลดเท่านั้น รุ่นหลัก/เว็บ/คู่มือยังเป็นv3r5

- ย้ายหัวข้อ “โทนที่ใช้อยู่” จากy364เป็นy350 หน้ากากไทยสูง32พิกเซลจบที่y381 ก่อนป้ายเหลืองเริ่มy390 จึงเห็นสระล่างครบ
- ตรวจสูตรต้นทางใหม่ครบ72รายการด้วยparserของโครงการโดยไม่รันโค้ดPythonต้นทาง เทียบC tableและadapter Contrast/Sharpness/Saturation/Color Tone/Kelvin/GM/BA/ISOครบตามapply_film
- พบ13ชุดค่าภาพที่ต่างกันเมื่อไม่รวมISO หรือ25ชุดเมื่อรวมISO หลายชื่อจึงให้ภาพเหมือนกันตามค่าpreviewต้นทาง การแสดงชื่อครบ72ชื่อไม่เท่ากับการมี72lookที่ต่างกัน
- ต้นทางมีขั้นตอนdevelopRAWแยก: LUT/เส้นโค้งRGB/สีเฉพาะช่วง/film balance/mono mix/halation/grain ซึ่งไม่มีในNativeJPEG pickerของเรา ดู [รายงานไทยทั้ง72รายการ](research/jpeg-tones/COVERAGE_AUDIT_TH.md) และCOVERAGE_AUDIT.json

ตัวอย่างPortra400NC/400VC/400UCมีค่ากล้องตรงกันทั้งหมด ส่วนPortra400หลัก/Ektar100/Gold200ต่างกันจริง การจะให้ชื่อที่ใช้ค่าเหมือนกันมีสีต่างกันต้องพัฒนาสูตรหรือระบบประมวลผลเพิ่ม ไม่ใช่เติมพารามิเตอร์Canonที่ตกหล่น รอบนี้ไม่เดาสูตรใหม่และไม่เปลี่ยนตัวเลขเดิม

ผ่านhostASAN/UBSANและตัวทดสอบนำหน้ากากไทยเต็มมาวาดซ้ำบนภาพหลังวาดป้าย แล้วตรวจพิกเซลทุกจุดว่าไม่ถูกป้ายบัง ผ่านเทียบสูตร72รายการ/CanonadapterและARMbuild/ขอบเขตBSS รวมถึงแพ็กเกจตรวจเลขรุ่น/CRC/XOR/ไฟล์และsourceตรงกัน ยังไม่ทดสอบtone8บนกล้องจริงหรือQEMU

อัปเดตautoexec.binและML/ทั้งชุด ใช้ไฟล์FILMART.BINและTONEINFO.BINเดิมจากtone7 สำรองชุดเดิมก่อน ไม่เรียกFirmwareUpdateซ้ำเพื่ออัปเดต ชุดทดลองนี้ไม่มีการเปลี่ยนหน้าเว็บหรือPublicReleaseใหม่

เครดิตM-Filmcontributorsสำหรับข้อมูลต้นทางและโค้ดอ้างอิงGPL-2.0-only;ฟอนต์Barlow/NotoคงOFLตามชุดเดิม ข้อค้างinheriteddietlibc sourceและpic_viewimportยังอยู่ตามTHIRD_PARTY_NOTICES.md
