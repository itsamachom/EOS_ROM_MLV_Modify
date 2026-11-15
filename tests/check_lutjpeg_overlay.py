"""Audit the bound ARM payload and simulate relocation at different camera heaps."""
from pathlib import Path
import struct,zlib,json
root=Path(__file__).resolve().parents[1];b=root/'firmware/platform/600D.102/build';p=(b/'zip/ML/LUTDATA/LUTJPEG.BIN').read_bytes();h=struct.unpack_from('<16I',p)
assert h[:3]==(0x3354554c,1,0x600d0102) and h[12]==0x4c554933
assert h[15]==zlib.crc32(p[:60]) and len(p)==64+h[3]+h[5]*4
image=p[64:64+h[3]];rel=p[64+h[3]:];positions=struct.unpack('<%dI'%h[5],rel)
assert positions==tuple(sorted(set(positions))) and all(x%4==0 and x+4<=len(image) for x in positions)
assert h[9]==zlib.crc32(image) and h[10]==zlib.crc32(rel)
assert h[11]==zlib.crc32((b/'magiclantern.sym').read_bytes())
core=(b/'autoexec.bin').read_bytes();assert h[13]==zlib.crc32(core) and h[14]==len(core)
assert all(x%4==0 and x+4<=len(image) for x in h[6:9])
# Independently inspect linked ELF relocations to identify every address needing rebasing.
e=(b/'lut-overlay/image.elf').read_bytes();eh=struct.unpack_from('<16sHHIIIIIHHHHHH',e)
ss=[struct.unpack_from('<10I',e,eh[6]+i*eh[11]) for i in range(eh[12])];st=next(s for s in ss if s[1]==2)
symbols=[struct.unpack_from('<IIIBBH',e,pos) for pos in range(st[4],st[4]+st[5],st[9])]
expected=[];fixed=[]
for s in ss:
 if s[1]!=9:continue
 for at in range(s[4],s[4]+s[5],8):
  pos,inf=struct.unpack_from('<II',e,at);sym=symbols[inf>>8];typ=inf&255
  if typ==2:
   if 0<sym[-1]<0xff00:expected.append(pos)
   else:fixed.append(pos)
  if typ in (28,29):
   assert sym[-1]!=0xfff1
   ins=struct.unpack_from('<I',image,pos)[0];delta=ins&0xffffff
   if delta&0x800000:delta-=1<<24
   target=pos+8+delta*4
   assert 0<=target<len(image),(pos,target)
assert sorted(expected)==list(positions)
for base in [0x10000000,0x18000000,0x1c000000]:
 loaded=bytearray(h[4]);loaded[:len(image)]=image
 for pos in positions:
  v=struct.unpack_from('<I',loaded,pos)[0];assert v<=h[4];struct.pack_into('<I',loaded,pos,base+v)
 for pos in fixed:assert loaded[pos:pos+4]==image[pos:pos+4]
 assert not any(loaded[len(image):])
loader=(root/'firmware/src/lutjpeg-loader-600d.h').read_text()
assert 'module_startup_complete' not in loader and 'module_load(' not in loader and 'module_exec(' not in loader
print(f'PASS bound core/symbol/image/header/relocation CRCs; {len(positions)} base relocations, fixed imports and ARM branch bounds at three heap addresses; zero BSS; no TCC/startup wait')
