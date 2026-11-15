"""Capture production JPEG tone UI with real font masks and simulated properties."""
from pathlib import Path
import os,subprocess,tempfile,hashlib,json
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
out=root/'previews/jpeg-tones';out.mkdir(parents=True,exist_ok=True)
with tempfile.TemporaryDirectory(prefix='jpeg-tone-proof-') as tmp:
 env=dict(os.environ,PHOTO_PROOF_RBF_DIR=str(root/'firmware/data/fonts'),PHOTO_PROOF_OUT=tmp)
 subprocess.run(['bash','tests/run-jpeg-tone-tests.sh'],cwd=root,env=env,check=True)
 palette={0:(0,0,0),1:(255,255,255),2:(0,0,0),5:(80,220,255),6:(55,170,65),7:(15,85,35),8:(220,40,40),11:(20,75,150),12:(130,20,40),14:(150,60,170),15:(245,205,35),19:(255,160,50)}
 import struct
 for i in range(38,80):palette[i]=(round((i-38)*255/41),)*3
 meta=(root/'firmware/data/photo_thai/FILMART.BIN').read_bytes()[:944]
 for i in range(160):
  rgb=struct.unpack_from('<I',meta,16+i*4)[0];palette[80+i]=((rgb>>16)&255,(rgb>>8)&255,rgb&255)
 frames=[];zooms=[];records=[]
 for p in sorted(Path(tmp).glob('*.pgm')):
  if not p.name.startswith(('tones-','tone-setting-','tone-active-','tone-browsing-','tone-changed-','tone-detail-','tone-zoom-','film-menu-')):continue
  raw=Image.open(p);assert raw.size==(720,480);assert set(raw.tobytes())<=palette.keys()
  image=Image.frombytes('RGB',raw.size,b''.join(bytes(palette[v]) for v in raw.tobytes()))
  proof=Image.new('RGB',(720,510),'#111111');proof.paste(image,(0,30))
  ImageDraw.Draw(proof).text((10,8),'HOST UI RENDER | simulated camera settings | no photo-color preview',fill='white')
  dest=out/(p.stem+'.png');proof.save(dest)
  records.append({'file':dest.name,'sha256':hashlib.sha256(dest.read_bytes()).hexdigest()})
  if p.name.startswith('tone-setting-'):frames.append(proof)
  if p.name.startswith('tone-zoom-'):zooms.append(proof)
 assert len(frames)==9
 assert len(zooms)==3
 zooms[0].save(out/'selection-zoom.gif',save_all=True,append_images=zooms[1:],duration=40,loop=0)
 frames[0].save(out/'setting-indicator.gif',save_all=True,append_images=frames[1:],duration=160,loop=0)
 (out/'manifest.json').write_text(json.dumps({'release':(root/'scripts/photo-ui-version.txt').read_text().strip(),'kind':'production UI host capture, simulated properties','color_effect_preview':False,'selection_animation':'Three cached-art selection frames; GIF40ms illustrates the transition, not measured camera timing','animation':'Illustration of acknowledged settings stages, not measured camera latency or post-shot processing','files':records},indent=2)+'\n')
 print(out)
