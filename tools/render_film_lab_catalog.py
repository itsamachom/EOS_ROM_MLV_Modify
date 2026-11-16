"""Illustrate every imported recipe on attributed CC0 photos; desktop, not camera evidence."""
from pathlib import Path
import json,sys,html
import numpy as np
from PIL import Image,ImageDraw
from caduceus_bake import render
root=Path(__file__).resolve().parents[1];out=root/'previews/film-lab';out.mkdir(exist_ok=True)
catalog=json.loads((root/'firmware/data/lutjpeg/caduceus/CATALOG.json').read_text())
samples=[]
for name in ['portrait','forest','neon','harbour']:
 im=Image.open(root/f'assets/film-lab-samples/{name}.jpg').convert('RGB');im.thumbnail((240,160));bg=Image.new('RGB',(240,160));bg.paste(im,((240-im.width)//2,(160-im.height)//2));x=np.asarray(bg)/255
 samples.append(np.where(x<=.04045,x/12.92,((x+.055)/1.055)**2.4))
desc={'Velvia':'สีสด ฟ้าและเขียวเข้ม เงาลึก','Astia':'นุ่มนวล สีผิวสะอาด ฟ้าชัดขึ้นเล็กน้อย','Cl. Chrome':'สีหม่น คอนทราสต์แบบภาพสารคดี','Classic Neg':'สีแยกชัด เงาอมเย็นและอุ่นในส่วนสว่าง','Provia 400':'สีสมดุลแบบสไลด์ คอนทราสต์ปานกลาง','Reala Ace':'สีเป็นธรรมชาติและแยกเฉดชัด','Eterna':'โทนภาพยนตร์นุ่ม สีไม่จัด ไฮไลต์อ่อน','Bleach Byp.':'ลดสีและเพิ่มคอนทราสต์','Kodachrome64':'สีอบอุ่น แดงเด่น เงาลึก','Portra 400':'ผิวนุ่ม สีอุ่น ไฮไลต์นุ่ม','Ektar 100':'สีสด รายละเอียดฟ้าและเขียวเด่น','Gold 200':'เหลืองอุ่นแบบฟิล์มถ่ายทั่วไป','Ektachrome':'ลักษณะสไลด์ โทนเย็นและสีสะอาด','CineStill800':'กลางคืน แสงไฟอุ่นและเงาเย็น','Pro 400H':'สีพาสเทล ผิวสว่างและเขียวอ่อน','Faux IR':'สีผิดธรรมชาติแบบอินฟราเรดจำลอง','Kodachrome25':'สีสไลด์อุ่นและเข้ม','Portra 160':'สีผิวอ่อน คอนทราสต์เบา','Fujicolor200':'โทนฟิล์มสีประจำวัน เขียวและฟ้าสมดุล','Cuban Neg':'สีอบอุ่น มีบุคลิกย้อนยุค','Cibachrome':'สีเข้มและอิ่มแบบภาพอัดจากสไลด์','UltraMax 400':'โทนอุ่น สีชัดสำหรับถ่ายทั่วไป','Agfa Vista':'สีสดแบบฟิล์มเนกาทีฟ','CineStill50D':'โทนภาพยนตร์กลางวัน','Vision3 250D':'โทนภาพยนตร์แสงกลางวัน','Vision3 500T':'โทนภาพยนตร์แสงทังสเตน','Polaroid':'สีจางและดำยกแบบภาพทันใจ','Cross Proc.':'สีเพี้ยนและคอนทราสต์เฉพาะตัว','Atrophy':'สีเก่าและซีดจาง','Acros':'ขาวดำไล่น้ำหนักนุ่ม','Tri-X 400':'ขาวดำคอนทราสต์ชัด','HP5 Plus':'ขาวดำใช้งานทั่วไป มีน้ำหนักกลาง'}
lines=['# รายการสูตรฟิล์ม v3r6-lut5','', 'มี 64 รายการในแพ็กปัจจุบัน + 64 รายการจาก Standard 1.0/1.1 ที่เก็บแยกไว้ ไม่ใช่ 128 ชื่อฟิล์มที่ต่างกันทั้งหมด ยังมี 3 สูตร lut4 ในกล้องด้วย','', 'ภาพต่อไปนี้เป็นตัวอย่างบนคอมพิวเตอร์จากภาพ CC0 ไม่ใช่ภาพถ่ายจาก Canon 600D และไม่ใช่การยืนยันว่าสีตรง Olympus หรือฟิล์มจริง ภาพแสดงการทำสี ไม่รวม FX เชิงพื้นที่และการลดนอยส์ของกล้อง ดู [เครดิตภาพ](../../assets/film-lab-samples/CREDITS.md) และ [สูตรต้นฉบับทุกค่า](../../firmware/data/lutjpeg/caduceus/CATALOG.json)','']
last=None
for e in catalog['looks']:
 if e['pack']!=last:
  lines+=['## '+e['pack_name'],''];last=e['pack'];print(last,flush=True)
 r=e['recipe'];sheet=Image.new('RGB',(480,356),'#111111');d=ImageDraw.Draw(sheet);d.text((8,7),e['pack_name']+' / '+r['name'],fill='white')
 for i,x in enumerate(samples):
  im=Image.fromarray((render(x,r)*255+.5).astype('uint8'));sheet.paste(im,((i%2)*240,36+(i//2)*160))
 name=f"look-{e['index']:03d}.jpg";sheet.save(out/name,quality=86)
 lines+=['### '+r['name'],'',desc.get(r['name'],'โทนเฉพาะหมวด '+e['pack_name'])+' — '+r['notes'],'',f'![{r["name"]}](../../previews/film-lab/{name})','', '<details><summary>ค่าทั้งสูตรจากต้นฉบับ</summary>','', '```json',json.dumps(r,ensure_ascii=False,indent=2),'```','', '</details>','']
report=root/'research/caduceus';report.mkdir(exist_ok=True);(report/'GALLERY_TH.md').write_text('\n'.join(lines).rstrip()+'\n')
