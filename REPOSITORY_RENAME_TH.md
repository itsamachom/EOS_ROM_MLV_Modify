# ชื่อ repository ปัจจุบัน: EOS_ROM_MLV_Modify

เจ้าของเปลี่ยนชื่อเรียบร้อยแล้วเป็น **itsamachom/EOS_ROM_MLV_Modify**
ตรวจชื่อจากหน้า GitHub โดยตรงเมื่อ 15 พฤศจิกายน 2026
ชื่อเดิมคือ `eos_rom_mlv_modifly`; ชื่อ `EOS_ROM_MLV_Modifly` เป็นชื่อที่เคยเสนอชั่วคราว ไม่ใช่ชื่อสุดท้าย

## ลิงก์หลัก

- Repository: https://github.com/itsamachom/EOS_ROM_MLV_Modify
- เว็บไซต์: https://itsamachom.github.io/EOS_ROM_MLV_Modify/
- ดาวน์โหลด: https://itsamachom.github.io/EOS_ROM_MLV_Modify/download.html
- คู่มือ: https://itsamachom.github.io/EOS_ROM_MLV_Modify/manual-th.html
- English: https://itsamachom.github.io/EOS_ROM_MLV_Modify/en/

ปรับลิงก์ในเอกสาร เว็บ ตัวสร้างเอกสาร และโครงการ Android ให้ใช้ชื่อสุดท้ายแล้ว
GitHub Pages ยังใช้ branch `site/eos-600d-bilingual` และโฟลเดอร์ `/docs` ตามเดิม
ปรับ URL ในคู่มือ PDF ของเว็บไซต์ด้วย แพ็กเกจเฟิร์มแวร์และ checksum ของรุ่นที่เผยแพร่แล้วไม่เปลี่ยน

## ผลต่อลิงก์เก่า

- GitHub มี redirect สำหรับชื่อ repository เก่า รวมถึงการเข้าถึง Git ส่วนลิงก์ดาวน์โหลด raw ต้องตรวจแยก
- ไม่สร้าง repository ใหม่ที่ชื่อเดิม หากยังต้องอาศัย redirect ของชื่อเก่า
- **GitHub Pages ไม่รับประกัน redirect ตาม repository** ควรแก้บุ๊กมาร์ก โพสต์ และลิงก์ที่แชร์ไว้ให้เป็น `/EOS_ROM_MLV_Modify/`
- ข้อความใน commit เก่า, source ZIP, PDF ในแพ็กเกจเก่า และ provenance อาจยังมีชื่อเก่า ไม่เขียนประวัติหรือแพ็กเกจทับเพียงเพื่อเปลี่ยน URL
- ไม่เปลี่ยนชื่อ branch/tag และไม่ต้องติดตั้งเฟิร์มแวร์กล้องใหม่เพื่อเปลี่ยนชื่อ repository

## ตรวจ Pages หลังเผยแพร่

เปิด https://github.com/itsamachom/EOS_ROM_MLV_Modify/settings/pages และใช้ URL ที่ GitHub แสดงจาก Visit site
ตรวจหน้าแรก, download.html, manual-th.html, คู่มือ PDF และ en/index.html
หากหน้าเว็บยังเป็นข้อมูลเก่า ให้รอ workflow `pages build and deployment` ของ commit ล่าสุดเสร็จ
เครื่องมือรอบนี้ถูกนโยบายเครือข่ายบล็อกการเปิด itsamachom.github.io จึงทดสอบเว็บและดาวน์โหลดบนเครื่อง พร้อมตรวจ deployment ผ่าน GitHub แยกจากการยืนยัน HTTP ของเว็บไซต์จริง

## อัปเดต remote บนเครื่องที่ clone ไว้

```sh
git remote set-url origin https://github.com/itsamachom/EOS_ROM_MLV_Modify.git
git remote -v
git fetch origin
```

ชื่อโฟลเดอร์ในเครื่องไม่จำเป็นต้องเปลี่ยน สำหรับงาน Codex ใหม่ให้เลือก repository ชื่อ EOS_ROM_MLV_Modify
