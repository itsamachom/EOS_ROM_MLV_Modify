# เปลี่ยนชื่อ repository เป็น EOS_ROM_MLV_Modifly

ชื่อที่ต้องการ: `itsamachom/EOS_ROM_MLV_Modifly` จากเดิม `itsamachom/eos_rom_mlv_modifly`
เป็นการเปลี่ยนตัวพิมพ์ของชื่อเดิม ไม่ต้องสร้าง repository ใหม่หรือย้ายประวัติ Git

## สถานะ

เตรียมลิงก์ GitHub และ raw.githubusercontent.com ในเอกสาร/หน้าเว็บ/ตัวสร้างเอกสารให้ใช้ชื่อใหม่แล้ว
ทดสอบ raw URL ที่ใช้ตัวพิมพ์ใหม่ได้ HTTP 200 แม้ชื่อแสดงผลของ repository ยังไม่ได้เปลี่ยน
ไฟล์แพ็กเกจที่เผยแพร่แล้วและข้อมูล provenance/checksum คงเป็นหลักฐานของรุ่นเดิม
การเปลี่ยนชื่อ repository ไม่ต้องติดตั้งเฟิร์มแวร์กล้องใหม่

**การแก้ไฟล์ไม่ได้เปลี่ยนชื่อแสดงผลใน GitHub Settings**
การเรียก GitHub API จากสภาพแวดล้อมนี้ถูกบล็อกที่ api.github.com (HTTP CONNECT 403)
จึงต้องให้เจ้าของเปลี่ยนชื่อผ่าน GitHub ก่อน:

1. เปิด https://github.com/itsamachom/EOS_ROM_MLV_Modifly/settings
2. ใน General หา Repository name
3. ใส่ `EOS_ROM_MLV_Modifly` แล้วกด Rename
4. ตรวจชื่อที่แสดงบนหน้า repository ว่าเปลี่ยนแล้ว

## ผลต่อลิงก์

- ลิงก์ repository, branch, tag, Issues และ PR ของ GitHub รองรับชื่อ repository แบบไม่แยกตัวพิมพ์ และ GitHub มีระบบ redirect สำหรับการ rename ทั่วไป
- URL ดาวน์โหลดที่ใช้ repository ตัวพิมพ์ใหม่ถูกตรวจแยกจาก URL เว็บไซต์ GitHub Pages
- ไม่แก้ tag เดิม ไม่เปลี่ยนชื่อ branch และไม่สร้างแพ็กเกจกล้องใหม่เพียงเพื่อเปลี่ยนตัวพิมพ์ของ URL
- ประวัติ commit, source ZIP และ PDF ที่เผยแพร่แล้วอาจยังมีชื่อเดิม ลิงก์ GitHub เดิมยังอาศัยการรองรับชื่อ/redirect ได้
- ถ้าเป็นการเปลี่ยนชื่อเป็นคำอื่นในอนาคต อย่าสร้าง repository ใหม่ที่ชื่อเก่าหากยังต้องพึ่ง redirect เดิม

## GitHub Pages ต้องตรวจแยก

เว็บไซต์ปัจจุบันอ้างอิง https://itsamachom.github.io/eos_rom_mlv_modifly/
ยังคง URL นี้ในลิงก์เว็บไซต์จนกว่า GitHub Settings → Pages จะยืนยัน URL หลัง rename
**อย่าอาศัย redirect ของ repository มาเป็นหลักประกันว่าเว็บไซต์ Pages จะ redirect ด้วย**

หลังเปลี่ยนชื่อ ให้เปิด https://github.com/itsamachom/EOS_ROM_MLV_Modifly/settings/pages
ตรวจ Source เดิม: Deploy from a branch → `site/eos-600d-bilingual` → `/docs`
ดู URL จากปุ่ม Visit site และทดสอบหน้าแรก, download.html, manual-th.html และ en/index.html
หาก URL เปลี่ยนเป็น `/EOS_ROM_MLV_Modifly/` ให้ปรับลิงก์ Pages และ base URL ใน tools/website-content/docs ตาม URL ที่ GitHub แสดง
อย่าเปลี่ยน Source/branch/folder โดยไม่จำเป็น

## อัปเดต remote บนเครื่องที่ clone ไว้

```sh
git remote set-url origin https://github.com/itsamachom/EOS_ROM_MLV_Modifly.git
git remote -v
git fetch origin
```

ชื่อโฟลเดอร์ในเครื่องไม่จำเป็นต้องเปลี่ยนตาม repository
สำหรับงาน Codex ใหม่ เลือก repository ตามชื่อใหม่หลังระบบ GitHub แสดงชื่อเรียบร้อย
