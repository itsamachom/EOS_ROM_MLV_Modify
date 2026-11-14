"""Audit the baked cartridge container consumed by the native picker."""
from pathlib import Path
import re,struct,zlib
root=Path(__file__).resolve().parents[1]
b=(root/'firmware/data/photo_thai/FILMART.BIN').read_bytes()
h=(root/'firmware/src/photo-film-art-data.h').read_text()
def constant(name):return int(re.search(r'#define '+name+r'\s+(0x[0-9a-fA-F]+|\d+)',h)[1],0)
assert struct.unpack_from('<4sHHHHI',b)==(b'FAR7',72,176,194,80,1)
meta=constant('PHOTO_FILM_ART_META_BYTES');cell=constant('PHOTO_FILM_ART_CELL_BYTES')
assert meta==944 and cell==176*194 and len(b)==meta+72*cell==constant('PHOTO_FILM_ART_BYTES')
assert zlib.crc32(b[:meta])==constant('PHOTO_FILM_ART_META_CRC')
assert all(c<=0xffffff for c in struct.unpack_from('<160I',b,16))
unique=set()
for i in range(72):
 sprite=b[meta+i*cell:meta+(i+1)*cell]
 assert zlib.crc32(sprite)==struct.unpack_from('<I',b,656+i*4)[0]
 assert all(v==0 or 38<=v<240 for v in set(sprite)),i
 assert 0 in sprite and len(set(sprite))>40,i
 unique.add(sprite)
assert len(unique)==72
print('PASS 72 unique baked sprites, metadata/cell CRCs, transparency, palette bounds and shading levels')
