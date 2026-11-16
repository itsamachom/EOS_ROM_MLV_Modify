"""Bake 600D signed-UYVY colour previews for the seven trial recipes.
Scene RGB approximated from Canon's rendered Live View, not raw sensor data.
"""
from pathlib import Path
import json,struct,zlib,sys,hashlib
import numpy as np
from caduceus_bake import render
root=Path(__file__).resolve().parents[1];data=root/'firmware/data/lutjpeg';catalog=json.loads((data/'caduceus/CATALOG.json').read_text())
ids=[8,9,11,15,19,20,26]
names=['Velvia','Astia','Classic Neg','Bleach Byp.','Gold 200','Ektachrome','Fujicolor200']
records={x['index']:x['recipe'] for x in catalog['looks']}
# Index: 5-bit luma, 5-bit unsigned storage of signed U, 5-bit V.
v,u,y=np.indices((32,32,32));yy=y.ravel()*8+4;uu=u.ravel()*8+4;vv=v.ravel()*8+4
uu=np.where(uu>=128,uu-256,uu);vv=np.where(vv>=128,vv-256,vv)
r=np.clip(yy+1437*vv/1024,0,255);g=np.clip(yy-352*uu/1024-731*vv/1024,0,255);b=np.clip(yy+1812*uu/1024,0,255)
rgb=np.stack([r,g,b],axis=-1)/255;lin=np.where(rgb<=.04045,rgb/12.92,((rgb+.055)/1.055)**2.4)
hashes={}
for index in ids:
 out=render(lin,records[index])*255
 Y=np.clip(np.round(out@np.array([.299,.587,.114])),0,255).astype('uint32')
 U=np.clip(np.round((out[:,2]-Y)*.564),-128,127).astype('int32')&255
 V=np.clip(np.round((out[:,0]-Y)*.713),-128,127).astype('int32')&255
 payload=(U.astype('uint32')|(Y<<8)|(V.astype('uint32')<<16)|(Y<<24)).astype('<u4').tobytes()
 blob=struct.pack('<4sIII',b'FLV6',index,len(payload),zlib.crc32(payload))+payload
 path=data/f'LV{index:03d}.FLV'
 if '--check' in sys.argv:assert path.read_bytes()==blob,path
 else:path.write_bytes(blob)
 hashes[path.name]=hashlib.sha256(blob).hexdigest()
header='/* Trial set: one visible choice per complete recipe. */\nstatic const int films[]={'+','.join(map(str,ids))+'};\n'
header+='static const char *film_names[]={'+','.join(json.dumps(x) for x in names)+'};\n'
header+='static const char *film_notes[]={'+','.join(json.dumps(records[i]['notes']) for i in ids)+'};\n'
p=root/'firmware/modules/lutjpeg/seven-films.h'
if '--check' in sys.argv:assert p.read_text()==header
else:p.write_text(header)
print('PASS seven Live View tables and one-level film catalog reproduce exactly')
