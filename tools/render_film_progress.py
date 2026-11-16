"""Capture the real film progress renderer with simulated state, not a camera."""
from pathlib import Path
import os,subprocess,tempfile
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'previews/film-lab/progress';out.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory() as temp:
 t=Path(temp)
 subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror','-pthread','-fsanitize=address,undefined','-g',root/'tests/lutjpeg/progress.c','-o',t/'test'],check=True)
 env=dict(os.environ,PHOTO_TEST_ATLAS=str(root/'firmware/data/photo_thai/PHOTO_TH.BIN'),PHOTO_PROOF_RBF_DIR=str(root/'firmware/data/fonts'),PHOTO_PROOF_OUT=str(t),ASAN_OPTIONS='detect_leaks=0')
 subprocess.run([t/'test'],check=True,env=env)
 palette={0:(0,0,0),1:(255,255,255),2:(0,0,0),5:(80,220,255),15:(245,205,35)}
 palette.update({i:(round((i-38)*255/41),)*3 for i in range(38,80)})
 frames=[]
 for f in sorted(t.glob('*.pgm')):
  raw=Image.open(f);assert set(raw.tobytes())<=palette.keys()
  im=Image.frombytes('RGB',(720,480),b''.join(bytes(palette[v]) for v in raw.tobytes()))
  sheet=Image.new('RGB',(720,508),'#111111');sheet.paste(im,(0,28));ImageDraw.Draw(sheet).text((8,7),'HOST RENDER | simulated status | not a camera capture',fill='white');sheet.save(out/(f.stem+'.png'))
  if '-th-' in f.name:frames.append(sheet)
 frames[0].save(out/'film-progress-demo.gif',save_all=True,append_images=frames[1:],duration=500,loop=0)
