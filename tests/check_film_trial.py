"""Check the exact seven-look set, display-table integrity, and Canon-safe UI numbers."""
from pathlib import Path
import json,hashlib,re,struct,zlib,unittest
root=Path(__file__).resolve().parents[1];data=root/'firmware/data/lutjpeg'
class TrialTests(unittest.TestCase):
 def test_seven_names_and_ids(self):
  h=(root/'firmware/modules/lutjpeg/seven-films.h').read_text()
  self.assertIn('films[]={8,9,11,15,19,20,26}',h)
  self.assertEqual(re.findall(r'"([^"]+)"',h.split('film_names[]={')[1].split('};')[0]),['Velvia','Astia','Classic Neg','Bleach Byp.','Gold 200','Ektachrome','Fujicolor200'])
 def test_live_table_bounds_crc_and_distinct(self):
  payloads=[]
  for i in [8,9,11,15,19,20,26]:
   b=(data/f'LV{i:03d}.FLV').read_bytes();magic,index,n,crc=struct.unpack('<4sIII',b[:16])
   self.assertEqual((magic,index,n),(b'FLV6',i,32768*4));self.assertEqual(len(b),16+n);self.assertEqual(zlib.crc32(b[16:]),crc);payloads.append(hashlib.sha256(b[16:]).digest())
  self.assertEqual(len(set(payloads)),7)
 def test_safe_status_and_auxiliary_paths(self):
  module=(root/'firmware/modules/lutjpeg/lutjpeg.c').read_text();progress=(root/'firmware/modules/lutjpeg/film-progress.h').read_text()
  self.assertNotIn('%%',module+progress);self.assertNotIn('.PART',module)
  self.assertIn('film_uint',progress);self.assertIn('Last saved file',progress)
  self.assertNotIn('.name="Film collection"',module)
 def test_art_metadata_and_every_sprite(self):
  b=(data/'FILM7.BIN').read_bytes();self.assertEqual(struct.unpack_from('<4sHHHHI',b),(b'FAR7',7,176,194,80,1))
  self.assertEqual(len(b),684+7*176*194)
  for i in range(7):self.assertEqual(zlib.crc32(b[684+i*176*194:684+(i+1)*176*194]),struct.unpack_from('<I',b,656+i*4)[0])
if __name__=='__main__':unittest.main()
