"""Build and exercise production LUT core and actual camera adapter with synthetic RAW."""
from pathlib import Path
import subprocess,tempfile,os,shutil,json,hashlib
from PIL import Image
import numpy as np
root=Path(__file__).resolve().parents[1];module=root/'firmware/modules/lutjpeg';up=module/'upstream'
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0') # LSan unavailable under managed ptrace; host tracks all core allocations.
with tempfile.TemporaryDirectory(prefix='lutjpeg-check-',dir='/workspace/scratch') as temp:
 t=Path(temp)
 core=[module/'catalog.c']+[up/n for n in ['fos_dev.c','fos_jpeg.c','fos_print.c','fos_looks.c']]
 flags=['gcc','-O1','-g','-DCONFIG_600D=1','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(up),'-I'+str(root/'tests/lutjpeg')]
 def run(args,ok=True,**kw):
  p=subprocess.run(list(map(str,args)),env=env,capture_output=True,text=True,**kw)
  assert (p.returncode==0)==ok,(args,p.returncode,p.stderr)
  assert 'runtime error:' not in p.stderr and 'ERROR: AddressSanitizer' not in p.stderr,p.stderr
  return p
 run(['gcc','-O2',root/'tests/lutjpeg/mkcr2.c','-lm','-o',t/'mkcr2'])
 for name in ['host','adapter']:run(flags+[root/f'tests/lutjpeg/{name}.c']+core+['-lm','-o',t/name])
 run([t/'mkcr2',t/'chart.CR2','--sensor','5344,3516,142,51','--scene','chart'])
 original=hashlib.sha256((t/'chart.CR2').read_bytes()).hexdigest()
 results=[];arrays=[]
 proofs=root/'previews/lutjpeg';proofs.mkdir(exist_ok=True)
 for i,name in [(0,'portra400'),(2,'ektar100'),(7,'trix400')]:
  p=run([t/'host',i,root/'firmware/data/lutjpeg',t/'chart.CR2',t/(name+'.BMP'),t/(name+'.JPG')])
  with Image.open(t/(name+'.JPG')) as im:
   im.load();assert im.size==(720,480);arrays.append(np.asarray(im.convert('RGB')).astype(int))
  shutil.copy(t/(name+'.JPG'),proofs/(name+'.JPG'))
  results.append({'name':name,'jpeg_sha256':hashlib.sha256((t/(name+'.JPG')).read_bytes()).hexdigest(),'host_log':p.stderr})
 assert len({r['jpeg_sha256'] for r in results})==3
 mae=float(np.mean(np.abs(arrays[0]-arrays[1])));assert mae>2,mae
 assert np.max(np.abs(arrays[2][:,:,0]-arrays[2][:,:,1]))<=2
 assert np.max(np.abs(arrays[2][:,:,1]-arrays[2][:,:,2]))<=2
 # All failures must clean core allocations (host asserts balance returns to zero).
 (t/'missing').mkdir()
 run([t/'host',0,t/'missing',t/'chart.CR2',t/'bad.BMP',t/'bad.JPG'],ok=False)
 assert not (t/'bad.JPG').exists()
 (t/'badtable').mkdir()
 broken=bytearray((root/'firmware/data/lutjpeg/LOOK00.LUT').read_bytes());broken[0]^=1
 (t/'badtable/LOOK00.LUT').write_bytes(broken)
 run([t/'host',0,t/'badtable',t/'chart.CR2',t/'bad.BMP',t/'bad.JPG'],ok=False)
 assert not (t/'bad.JPG').exists()
 (t/'broken.CR2').write_bytes((t/'chart.CR2').read_bytes()[:256])
 run([t/'host',0,root/'firmware/data/lutjpeg',t/'broken.CR2',t/'bad.BMP',t/'bad.JPG'],ok=False)
 p=subprocess.run([str(t/'host'),'0',str(root/'firmware/data/lutjpeg'),str(t/'chart.CR2'),str(t/'bad.BMP'),str(t/'bad.JPG')],env=dict(env,LUT_TEST_MEM='1048576'),capture_output=True,text=True)
 assert p.returncode==2 and 'runtime error:' not in p.stderr,p.stderr
 # Actual adapter, using mocked Canon FIO which verifies DMA buffers and chunk sizes.
 (t/'A:/DCIM/100CANON').mkdir(parents=True);(t/'A:/ML/LUTDATA').mkdir(parents=True)
 shutil.copy(t/'chart.CR2',t/'A:/DCIM/100CANON/IMG_0001.CR2')
 for f in (root/'firmware/data/lutjpeg').glob('*.LUT'):shutil.copy(f,t/'A:/ML/LUTDATA'/f.name)
 p=run([t/'adapter'],cwd=t)
 assert hashlib.sha256((t/'A:/DCIM/100CANON/IMG_0001.CR2').read_bytes()).hexdigest()==original
 (proofs/'TEST_RESULTS.json').write_text(json.dumps({'kind':'Synthetic RAW processed by actual production core; not a camera photograph or camera timing','matrix':'600D','size':[720,480],'color_mae_portra_ektar':mae,'results':results,'adapter_checks':p.stdout.strip(),'checks':['ASAN/UBSAN','three distinct LUT JPEGs','monochrome output','DMA bounce chunk bounds','output collision avoidance','original RAW unchanged','cancel','write failure','missing RAW','missing/corrupt table','rename failure','truncated RAW','low memory']},indent=2)+'\n')
 print('PASS production RAW/LUT/JPEG and camera adapter; Portra/Ektar RGB mean difference:',round(mae,2))
