"""Bake antialiased cylinder sprites and scoped color ramps for the 600D picker.
M-Film label/ramp cues: GPL-2.0-only; independent offline geometry and packing.
Barlow fonts: OFL. No photographs, RAW or LUT processing.
"""
from pathlib import Path
import json,math,struct,zlib,hashlib
import numpy as np
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1];out=root/'firmware/data/photo_thai'
labels=json.loads((root/'research/film-labels/CATALOG.json').read_text())['labels']
# Same hue families as the pinned source, 20 shades each; native UI entries 0..79 stay intact.
ramps=[(0x080402,0xF4A81C,0xFFF2D2,185),(0x080202,0xDF262D,0xFFD8D4,150),(0x080302,0xEA5D1E,0xFFDEC9,160),(0x060102,0x9C1320,0xF6C4C8,120),(0x020604,0x13894B,0xD0F5DF,140),(0x040208,0x5A2D8A,0xE9DCF7,120),(0x020508,0x1E77C6,0xD5EBFF,150),(0x050403,0x4C3B2E,0xDCCAB6,130)]
rgb=lambda c:np.array([(c>>16)&255,(c>>8)&255,c&255],dtype=np.float32)
def ramp_color(r,lv):
 if r==0:return rgb(0x040303)+(rgb(0xFFFCF6)-rgb(0x040303))*lv/255
 if r==1:return rgb(0x050608)+(rgb(0xF4F7FA)-rgb(0x050608))*lv/255
 lo,mid,hi,at=ramps[r-2]
 return rgb(lo)+(rgb(mid)-rgb(lo))*lv/at if lv<=at else rgb(mid)+(rgb(hi)-rgb(mid))*(lv-at)/(255-at)
colors={38+i:(round(i*255/41),)*3 for i in range(42)}
palette=[]
for r in range(2,10):
 for j in range(20):
  c=tuple(int(round(x)) for x in ramp_color(r,j*255/19));palette.append(c);colors[80+len(palette)-1]=c
# Build a 5-bit RGB lookup once, instead of a nearest-color scan per sprite pixel.
indices=np.array(list(colors),dtype=np.uint8);pal=np.array(list(colors.values()),dtype=np.int32)
lut=np.empty(32768,dtype=np.uint8)
for start in range(0,32768,1024):
 q=np.arange(start,min(start+1024,32768));p=np.stack([((q>>10)&31)*8+4,((q>>5)&31)*8+4,(q&31)*8+4],axis=1)
 d=(p[:,None,:]-pal[None,:,:])**2;dist=d[:,:,0]*2+d[:,:,1]*3+d[:,:,2]
 lut[start:start+len(q)]=indices[np.argmin(dist,axis=1)]
SS=3;W,H=176,194
fontdir=root/'assets/fonts/barlow'
font=lambda size,big=False:ImageFont.truetype(str(fontdir/('BarlowCondensed-ExtraBold.ttf' if big else 'BarlowCondensed-Bold.ttf')),round(size*SS))
def texture(l):
 im=Image.new('RGB',(320*SS,150*SS),tuple(map(int,ramp_color(*l['bg']))));d=ImageDraw.Draw(im)
 def rect(box,pair):d.rectangle(tuple(round(v*SS) for v in box),fill=tuple(map(int,ramp_color(*pair))))
 if l['style']==0:rect((0,34,320,67),l['band']);rect((0,121,320,124),l['accent'])
 elif l['style']==1:rect((0,0,320,24),l['band']);rect((0,24,320,28),l['accent']);rect((0,125,320,132),l['band'])
 else:
  rect((0,5,320,7),l['accent'])
  for j in range(2):d.polygon([(0,(119+6*j)*SS),(320,(112+6*j)*SS),(320,(114+6*j)*SS),(0,(121+6*j)*SS)],fill=tuple(map(int,ramp_color(*l['accent']))))
 def text(x,y,s,size,pair,big=False):d.text((x*SS,y*SS),s,font=font(size,big),fill=tuple(map(int,ramp_color(*pair))),anchor='mm')
 text(160,16,l['top'],8,l['num'])
 text(160,50,l['word'],19,l['text']);text(160,92,l['num_s'],43,l['num'],True)
 text(160,116,l['sub'],10,l['num']);text(160,143,l.get('process',''),9,l['num'])
 # Side names wrap naturally into the cylinder silhouette.
 for u,s in [(92,l['brand']),(228,l['name'])]:
  m=Image.new('RGBA',(150*SS,14*SS));md=ImageDraw.Draw(m);md.text((0,0),s,font=font(10),fill=tuple(map(int,ramp_color(*l['num'])))+(255,))
  m=m.rotate(-90,expand=True);im.paste(m,(u*SS,20*SS),m)
 return np.array(im,dtype=np.float32)
recipes=json.loads((root/'research/jpeg-tones/UPSTREAM_ORIGINAL_CATALOG.json').read_text())['recipes']
Y,X=np.mgrid[:H*SS,:W*SS].astype(np.float32);x=(X+.5)/SS;y=(Y+.5)/SS
cx,top,R,body=73,28,46,149
nx=(x-cx)/R;inside=np.abs(nx)<1;theta=np.arcsin(np.clip(nx,-1,1));z=np.sqrt(np.maximum(0,1-nx*nx));dy=8*z
v=y-top-dy;mask=inside&(v>=0)&(v<body)
key=np.maximum(0,np.cos(theta+.48));light=np.clip(.20+.80*key,.20,1)
spec=.20*np.exp(-((theta+.40)/.22)**2)+.26*np.exp(-((theta+.37)/.055)**2)+.13*np.exp(-((theta-1.25)/.10)**2)
images=[];crcs=[]
for i,(l,r) in enumerate(zip(labels,recipes)):
 l=dict(l,process=r['process']);tex=texture(l)
 tx=np.clip(((theta/(2*math.pi)+.5)*320*SS).astype(int),0,320*SS-1);ty=np.clip((v*SS).astype(int),0,150*SS-1)
 arr=np.zeros((H*SS,W*SS,3),dtype=np.float32);alpha=np.zeros((H*SS,W*SS),dtype=np.uint8)
 src=tex[ty,tx];lit=src*light[:,:,None]+spec[:,:,None]*(255-src)*.8
 arr[mask]=np.clip(lit[mask],0,255);alpha[mask]=255
 # Brown film tongue, tapered and curved, punched holes. Render behind cylinder.
 lx=x-(cx+R-2);bend=lx*lx/290;ly=y-(42+bend);lead=(lx>=0)&(lx<54)&(ly>=0)&(ly<91)&(~inside)
 holes=((lx.astype(int)%12)<5)&(((ly>6)&(ly<13))|((ly>77)&(ly<84)))
 lead&=~holes
 base=ramp_color(0 if r['group']==3 else 9,85)
 ll=.55+.45*np.exp(-((ly-27)/28)**2)
 arr[lead]=np.clip(base*ll[lead,None]+7,0,255);alpha[lead]=255
 im=Image.fromarray(arr.astype(np.uint8),'RGB');ad=Image.fromarray(alpha,'L');d=ImageDraw.Draw(im);da=ImageDraw.Draw(ad)
 def ellipse(box,fill):
  bb=tuple(round(t*SS) for t in box);d.ellipse(bb,fill=fill);da.ellipse(bb,fill=255)
 # Brushed steel elliptical rings, individually shaded columns, not a flat oval.
 for center,ry in [(25,8),(179,6)]:
  for col in range(98*SS):
   xx=24+col/SS;n=(xx-cx)/49
   if abs(n)>=1:continue
   h=ry*math.sqrt(1-n*n);t=math.asin(n)
   m=.17+.26*max(0,math.cos(t+.3))+.46*math.exp(-((t+.4)/.17)**2)+.25*math.exp(-((t-1.3)/.17)**2)
   a,b=round((center-h)*SS),round((center+h)*SS);val=min(255,round(m*255))
   d.line((round(xx*SS),a,round(xx*SS),b),fill=(val,val,val),width=1);da.line((round(xx*SS),a,round(xx*SS),b),fill=255,width=1)
  # A fine rim highlight and dark inner lip.
  bb=((cx-48)*SS,(center-ry)*SS,(cx+48)*SS,(center+ry)*SS)
  d.ellipse(tuple(map(round,bb)),outline=(175,180,183),width=SS)
 ellipse((cx-40,19,cx+40,27),(20,22,24))
 for col in range(27*SS):
  xx=cx-13.5+col/SS;v0=round(18+45*math.exp(-((xx-(cx-5))/4)**2))
  bb=(round(xx*SS),round(8*SS),round(xx*SS),round(24*SS));d.line(bb,fill=(v0,v0,v0));da.line(bb,fill=255)
 ellipse((cx-13.5,5,cx+13.5,12),(60,64,67));ellipse((cx-10,5.5,cx+10,9.5),(8,9,10))
 im=im.resize((W,H),Image.Resampling.LANCZOS);ad=ad.resize((W,H),Image.Resampling.LANCZOS)
 c=np.array(im,dtype=np.uint16);a=np.array(ad)>128
 q=((c[:,:,0]>>3)<<10)|((c[:,:,1]>>3)<<5)|(c[:,:,2]>>3);ix=lut[q];ix[~a]=0
 blob=ix.tobytes();images.append(blob);crcs.append(zlib.crc32(blob))
 print(i+1,l['key']) if i%12==0 else None
meta=struct.pack('<4sHHHHI',b'FAR7',72,W,H,80,1)+b''.join(struct.pack('<I',(c[0]<<16)|(c[1]<<8)|c[2]) for c in palette)+struct.pack('<72I',*crcs)
assert len(meta)==944
blob=meta+b''.join(images);(out/'FILMART.BIN').write_bytes(blob)
(root/'firmware/src/photo-film-art-data.h').write_text(f'/* Generated offline antialiased art metadata. See tools/generate_film_art.py. */\n#define PHOTO_FILM_ART_BYTES {len(blob)}u\n#define PHOTO_FILM_ART_META_CRC 0x{zlib.crc32(meta):08x}u\n#define PHOTO_FILM_ART_META_BYTES 944\n#define PHOTO_FILM_ART_CELL_BYTES {W*H}\n')
(root/'firmware/data/photo_thai/film-art-palette.json').write_text(json.dumps({str(k):v for k,v in colors.items()},indent=2)+'\n')
print('Generated',len(blob),'bytes with 72 CRC-checked sprites and 160 art-only shades')
