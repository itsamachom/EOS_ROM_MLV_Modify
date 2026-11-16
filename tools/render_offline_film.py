"""Capture the production seven-film cards with mock camera state."""
from pathlib import Path
import tempfile,subprocess,os,struct
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'previews/film-lab/v3r6-lut8';out.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory() as tmp:
 t=Path(tmp)
 subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-pthread','-fsanitize=address,undefined','-g','-iquote',root/'firmware/platform/600D.102',root/'tests/lutjpeg/offline_ui.c','-o',t/'test'],check=True)
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',PHOTO_TEST_ATLAS=str(root/'firmware/data/photo_thai/PHOTO_TH.BIN'),PHOTO_PROOF_RBF_DIR=str(root/'firmware/data/fonts'),PHOTO_TRIAL_ART=str(root/'firmware/data/lutjpeg/FILM7.BIN'),PHOTO_PROOF_OUT=str(t))
 subprocess.run([t/'test'],env=env,check=True)
 pal={0:(0,0,0),1:(255,255,255),2:(0,0,0),5:(80,220,255),6:(55,170,65),7:(15,85,35),11:(20,75,150),15:(245,205,35),19:(255,160,50)}
 pal.update({i:(round((i-38)*255/41),)*3 for i in range(38,80)})
 for f in t.glob('*.pgm'):
  raw=Image.open(f);assert set(raw.tobytes())<=pal.keys()
  im=Image.frombytes('RGB',(720,480),b''.join(bytes(pal[x]) for x in raw.tobytes()))
  sheet=Image.new('RGB',(720,508),'#111111');sheet.paste(im,(0,28));ImageDraw.Draw(sheet).text((8,7),'HOST RENDER | actual offline converter code | simulated camera state',fill='white');sheet.save(out/(f.stem+'.png'))
