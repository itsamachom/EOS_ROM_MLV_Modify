"""Production offline file converter against synthetic 600D RAW; no camera claims."""
from pathlib import Path
import subprocess,tempfile,shutil,os
from PIL import Image
root=Path(__file__).resolve().parents[1];m=root/'firmware/modules/lutjpeg'
with tempfile.TemporaryDirectory(dir='/workspace/scratch') as tmp:
 t=Path(tmp)
 subprocess.run(['gcc','-O2',root/'tests/lutjpeg/mkcr2.c','-lm','-o',t/'mkcr2'],check=True)
 subprocess.run([t/'mkcr2',t/'chart.CR2','--sensor','5344,3516,142,51','--scene','chart'],check=True,capture_output=True)
 for d in ['A:/DCIM/100CANON','A:/ML/LUTDATA','ML/LUTDATA']:(t/d).mkdir(parents=True)
 for i in range(1,6):shutil.copy(t/'chart.CR2',t/f'A:/DCIM/100CANON/IMG_{i:04}.CR2')
 (t/'A:/DCIM/100CANON/IMG_0001.JPG').write_bytes(b'original camera jpeg placeholder; must never be changed')
 shutil.copy(root/'firmware/data/lutjpeg/FILM7.BIN',t/'ML/LUTDATA/FILM7.BIN')
 for f in (root/'firmware/data/lutjpeg').glob('*.LUT'):shutil.copy(f,t/'A:/ML/LUTDATA'/f.name)
 core=[m/'catalog.c']+[m/'upstream'/n for n in ['fos_dev.c','fos_jpeg.c','fos_print.c','fos_looks.c']]
 subprocess.run(['gcc','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(root/'tests/lutjpeg'),'-I'+str(m/'upstream'),root/'tests/lutjpeg/offline.c',*core,'-lm','-o',t/'test'],check=True)
 p=subprocess.run([t/'test'],cwd=t,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'),text=True,capture_output=True)
 assert p.returncode==0,(p.returncode,p.stdout,p.stderr[-3000:]);assert 'runtime error:' not in p.stderr,p.stderr
 print(p.stdout)
 files=list((t/'A:/ML/FILMOUT').glob('*.JPG'));assert len(files)==9
 proof=root/'previews/film-lab/v3r6-lut8';proof.mkdir(exist_ok=True)
 import hashlib,json
 results=[]
 for f in sorted(files):
  with Image.open(f) as im:
   im.load();assert im.width>=1920;size=im.size
   exif=im.getexif().get_ifd(0x8769);assert exif[0xA001]==1 and (exif[0xA002],exif[0xA003])==size and exif[0x9000]==b"0230"
  results.append({'name':f.name,'size':size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()})
  if not f.stem.endswith('_001'):shutil.copy(f,proof/f.name)
 assert len({r['sha256'] for r in results[:5]})>=4
 (proof/'TEST_RESULTS.json').write_text(json.dumps({'kind':'synthetic RAW; host production adapter; not camera or timing validation','checks':p.stdout,'files':results},indent=2)+'\n')
 print('PASS JPEGs decode at large size; named outputs and unchanged input checked')
