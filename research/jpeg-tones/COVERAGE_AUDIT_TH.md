# ตรวจความครอบคลุมโทน JPEG ทั้ง72รายการ

อ้างอิง M-Film commit `8118e9c7c072371bd53eda8920f44910e8a35baf` ซึ่งเป็นต้นทางของชุดปัจจุบัน ไม่ใช่การวัดสี JPEG จากกล้องจริง

ตรวจค่าตัวเลขจาก source ต้นทางใหม่ครบ72รายการ: ไม่พบค่าตั้งกล้องตกหล่น/ต่างจากแคตตาล็อก จำนวนชุดค่าที่ต่างกันเมื่อไม่รวมISO = **13** และเมื่อรวมISO = **25**

## ส่วนที่มีแล้ว

Picture Style, Contrast, Sharpness, Saturation, Color Tone, Kelvin, WB shift GM/BA และ ISO มีครบตาม apply_film ต้นทาง พร้อมอ่านค่ากลับหลังตั้งค่า ทั้งต้นทางและforkเก็บค่าFilter/ToningเดิมในMonochromeไว้ ไม่ใช่สูตรFilterแยกรายฟิล์ม

## เหตุที่หลายชื่อได้ภาพเหมือนกัน

ต้นทางระบุชัดว่าค่าPicture Styleใช้เป็นJPEG preview ส่วนlookจริงเกิดตอนdevelop RAW ตารางgenfilms.pyใช้สูตรเริ่มต้นร่วมกันตามประเภทฟิล์ม หลายชื่อจึงใช้Canon controlsเหมือนกันทั้งหมด การนำชื่อและค่าเหล่านี้มาครบไม่ได้หมายความว่าได้นำกระบวนการสร้างสีทั้งหมดมาด้วย

| ส่วนประมวลผลต้นทางที่ยังไม่มี | ผลต่อภาพ |
|---|---|
| LUT 3มิติและการแปลงสีรายฟิล์ม | แยกบุคลิกสีของชื่อที่ใช้ค่าCanonร่วมกัน |
| เส้นโค้งR/G/B, pivot, black/white | เปลี่ยนสีเงาและไฮไลต์อย่างอิสระ |
| Selective hue/chroma, saturationในlinear light | ปรับเฉพาะย่านสี ต่างจากColor ToneรวมของCanon |
| Film balance / scanner exposure / mono channel mix | สมดุลสี การเปิดเงา และการผสมช่องขาวดำ |
| Halation, grain/grain size | แสงฟุ้งรอบไฮไลต์และลักษณะเกรน |
| Softness | พารามิเตอร์ฝั่งdesktop lab; ไม่อยู่ในstruct fos_lookที่กล้องรับ จึงไม่ควรอ้างว่าต้นทางกล้องมีครบ |

grain/punch/colour/warmthในข้อมูลชั้นวางเป็นคะแนนแสดงบุคลิก ไม่ใช่Canon propertyที่ตกหล่น apply_filmไม่ได้เขียนค่าเหล่านี้

## ตัวอย่างที่ตรงกับอาการทดลอง

Portra 400NC / 400VC / 400UC ใช้Standard, Contrast0, Sharpness2, Saturation0, Color Tone0, 5200K, GM0, BA0 และISO400เหมือนกันทั้งหมด จึงไม่มีขั้นตอนในชุดปัจจุบันที่จะทำให้สีสามชื่อนี้ต่างกัน

Portra 400 (ตัวหลัก), Ektar 100 และGold 200มีค่าต่างกันจริง ตรวจreadbackแล้วในhost fixture แต่ยังไม่ยืนยันผลบนฮาร์ดแวร์ของผู้ใช้

## แนวทางสำหรับJPEGทันที

1. หากต้องการใช้สูตรต้นทางเหมือนเดิมทุกค่า ต้องยอมรับว่าหลายชื่อได้ภาพเหมือนกัน
2. หากต้องการให้หลายโทนต่างกันโดยยังถ่ายJPEGทันที ให้พัฒนาสูตรNative JPEGของเราเองหรือประเมินCanon custom Picture Styleพร้อมทดสอบบน600D ทั้งสองทางเป็นการประมาณ ไม่ใช่การย้ายLUTต้นทางโดยตรง
3. หากต้องการlookตามกระบวนการต้นทาง ต้องเพิ่มการประมวลผลภาพแยก ซึ่งต้องประเมินคุณภาพ เวลาและหน่วยความจำ ไม่ควรเปิดใช้เงียบๆในอัปเดตแก้UIนี้

รุ่นtone8แก้ระยะข้อความเท่านั้น ไม่เดาสูตรใหม่ ไม่เปลี่ยนISO/WB/สูตร72รายการ และไม่เพิ่มRAW/LUT engine

## ตารางทั้ง72รายการ

กลุ่มสีเดียวกันหมายถึงค่าทั้ง8ช่องด้านล่างเหมือนกัน ไม่รวมISO ชื่อสีเป็นชื่อpresetต้นทาง ไม่รับรองผลสีเท่าฟิล์มจริง

| # | ชื่อโทน | กลุ่มสี | ISOจริง | Style | C | Sharp | Sat | Tone | K | GM | BA |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | KODAK Portra 400 | 1 | 400 | Standard | -1 | 2 | -1 | 1 | 5500 | 0 | 1 |
| 2 | CINESTILL 800T | 2 | 800 | Standard | 1 | 2 | 1 | -1 | 3200 | 0 | 0 |
| 3 | KODAK Ektar 100 | 3 | 100 | Landscape | 1 | 3 | 2 | 0 | 5200 | 0 | 0 |
| 4 | KODAK Gold 200 | 4 | 200 | Standard | 1 | 2 | 1 | 2 | 5600 | 0 | 2 |
| 5 | FUJIFILM Superia 400 | 5 | 400 | Standard | 1 | 2 | 1 | 0 | 5200 | 1 | 0 |
| 6 | FUJIFILM Velvia 50 | 6 | 100 | Landscape | 3 | 3 | 4 | 0 | 5200 | 0 | 0 |
| 7 | ILFORD HP5 Plus | 7 | 400 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 8 | KODAK Tri-X 400 | 8 | 400 | Mono | 3 | 2 | kept | kept | 5200 | 0 | 0 |
| 9 | CINESTILL 400T | 9 | 400 | Standard | 1 | 2 | 1 | -1 | 5200 | 0 | 0 |
| 10 | KODAK Portra 160 | 10 | 200 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 11 | KODAK Portra 800 | 10 | 800 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 12 | KODAK Portra 160NC | 10 | 200 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 13 | KODAK Portra 160VC | 10 | 200 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 14 | KODAK Portra 400NC | 10 | 400 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 15 | KODAK Portra 400VC | 10 | 400 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 16 | KODAK Portra 400UC | 10 | 400 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 17 | KODAK Elite Color 200 | 10 | 200 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 18 | KODAK Elite Color 400 | 10 | 400 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 19 | FUJIFILM Pro 160C | 10 | 200 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 20 | FUJIFILM Pro 400H | 10 | 400 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 21 | FUJIFILM Pro 800Z | 10 | 800 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 22 | FUJIFILM Reala 100 | 10 | 100 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 23 | FUJIFILM Superia 100 | 10 | 100 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 24 | FUJIFILM Superia 200 | 10 | 200 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 25 | FUJIFILM Superia 800 | 10 | 800 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 26 | FUJIFILM Superia 1600 | 10 | 1600 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 27 | FUJIFILM X-TRA 800 | 10 | 800 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 28 | AGFA Vista 200 | 10 | 200 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 29 | AGFA Ultra 100 | 10 | 100 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 30 | LOMOGRAPHY Redscale 100 | 10 | 100 | Standard | 0 | 2 | 0 | 0 | 5200 | 0 | 0 |
| 31 | FUJIFILM Velvia 100 | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 32 | FUJIFILM Provia 100F | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 33 | FUJIFILM Provia 400F | 11 | 400 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 34 | FUJIFILM Provia 400X | 11 | 400 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 35 | FUJIFILM Astia 100F | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 36 | FUJIFILM Sensia 100 | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 37 | KODAK Kodachrome 25 | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 38 | KODAK Kodachrome 64 | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 39 | KODAK Kodachrome 200 | 11 | 200 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 40 | KODAK Ektachrome E100GX | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 41 | KODAK Ektachrome E100VS | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 42 | KODAK Elite Chrome 200 | 11 | 200 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 43 | KODAK Elite Chrome 400 | 11 | 400 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 44 | KODAK ExtraColor 100 | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 45 | AGFA Precisa 100 | 11 | 100 | Landscape | 2 | 2 | 2 | 0 | 5200 | 0 | 0 |
| 46 | KODAK Elite 100 X-Pro | 12 | 100 | Landscape | 3 | 2 | 3 | 0 | 5200 | 0 | 0 |
| 47 | FUJIFILM Superia 200 X-Pro | 12 | 200 | Landscape | 3 | 2 | 3 | 0 | 5200 | 0 | 0 |
| 48 | LOMOGRAPHY X-Pro Slide 200 | 12 | 200 | Landscape | 3 | 2 | 3 | 0 | 5200 | 0 | 0 |
| 49 | KODAK T-MAX 100 | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 50 | KODAK T-MAX 400 | 7 | 400 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 51 | KODAK T-MAX P3200 | 7 | 3200 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 52 | KODAK BW400CN | 7 | 400 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 53 | KODAK HIE Infrared | 7 | 400 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 54 | ILFORD Delta 100 | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 55 | ILFORD Delta 400 | 7 | 400 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 56 | ILFORD Delta 3200 | 7 | 3200 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 57 | ILFORD FP4 Plus | 7 | 200 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 58 | ILFORD Pan F Plus | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 59 | ILFORD HPS 800 | 7 | 800 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 60 | ILFORD XP2 Super | 7 | 400 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 61 | FUJIFILM Acros 100 | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 62 | FUJIFILM Neopan 1600 | 7 | 1600 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 63 | AGFA APX 25 | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 64 | AGFA APX 100 | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 65 | ROLLEI Retro 80S | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 66 | ROLLEI Retro 100 Tonal | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 67 | ROLLEI Ortho 25 | 7 | 100 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 68 | ROLLEI Infrared 400 | 7 | 400 | Mono | 1 | 2 | kept | kept | 5200 | 0 | 0 |
| 69 | KODAK Vision 2383 | 13 | 200 | Standard | 1 | 2 | -1 | 0 | 5200 | 0 | 0 |
| 70 | KODAK Vision 2393 | 13 | 200 | Standard | 1 | 2 | -1 | 0 | 5200 | 0 | 0 |
| 71 | FUJIFILM Eterna 3510 | 13 | 200 | Standard | 1 | 2 | -1 | 0 | 5200 | 0 | 0 |
| 72 | FUJIFILM Eterna 3513 | 13 | 200 | Standard | 1 | 2 | -1 | 0 | 5200 | 0 | 0 |
