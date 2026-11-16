"""Validate complete attributed catalogs and independent Canon colour math."""
from pathlib import Path
import json,hashlib,sys,unittest,copy
import numpy as np
root=Path(__file__).resolve().parents[1];sys.path.insert(0,str(root/'tools'))
from caduceus_bake import render
folder=root/'firmware/data/lutjpeg/caduceus';catalog=json.loads((folder/'CATALOG.json').read_text())
class CatalogTests(unittest.TestCase):
 def test_complete_original_assets(self):
  self.assertEqual(len(catalog['files']),9);self.assertEqual(len(catalog['looks']),128)
  expected=[]
  for name,digest in catalog['files'].items():
   data=(folder/name).read_bytes();self.assertEqual(hashlib.sha256(data).hexdigest(),digest)
   pack=json.loads(data)
   expected.extend((pack['id'],r) for r in pack['looks'])
  self.assertEqual([(r['pack'],r['recipe']) for r in catalog['looks']],expected)
  self.assertEqual([r['index'] for r in catalog['looks']],list(range(8,136)))
  self.assertIn('Apache License',(folder/'LICENSE').read_text()[:200])
 def test_all_controls_known_and_finite(self):
  pixels=np.array([[0,0,0],[1,1,1],[.18,.18,.18],[1,.05,.01],[.01,.9,.4],[.1,.2,1],[16,.01,.1]])
  before=pixels.copy()
  mono=0
  for e in catalog['looks']:
   r=e['recipe'];self.assertTrue(set(r)<= {'name','notes','kind','hue','light','sat','grain','wb','effect','vignette','art_effect','tone','advice','split','matrix','curve'})
   self.assertEqual(r['art_effect'],'off');self.assertEqual(r['vignette'],0)
   self.assertIn((r.get('effect') or {}).get('type'),[None,'bloom','halation','clarity'])
   for k in ('hue','light','sat'):self.assertEqual(len(r[k]),12)
   out=render(pixels,r);self.assertTrue(np.isfinite(out).all());self.assertTrue(((out>=0)&(out<=1)).all())
   if r['kind']=='mono':
    mono+=1;self.assertTrue(np.allclose(out[:,0],out[:,1]));self.assertTrue(np.allclose(out[:,1],out[:,2]))
  self.assertEqual(mono,17);self.assertTrue(np.array_equal(pixels,before))
 def test_distinct_colour_controls(self):
  base={'kind':'colour','hue':[0]*12,'sat':[0]*12,'light':[0]*12,'tone':{},'advice':{}}
  pixels=np.array([[.6,.1,.02],[.03,.2,.7],[.5,.5,.5]])
  identity=render(pixels,base)
  for key,value in [('matrix',[10,-5,0,0,-5,10]),('hue',[3]*12),('sat',[3]*12),('light',[3]*12),('wb',{'a_b':3,'g_m':2}),('tone',{'shadows':3,'highlights':-2}),('split',{'sh_color':8,'sh':2,'hl_color':1,'hl':2})]:
   r=copy.deepcopy(base);r[key]=value;self.assertGreater(np.max(np.abs(render(pixels,r)-identity)),.001,key)
 def test_table_headers_payload_manifest(self):
  manifest=json.loads((folder.parent/'TABLES.json').read_text());self.assertEqual(len(manifest['indices']),131)
  for name,digest in manifest['sha256'].items():
   b=(folder.parent/name).read_bytes();self.assertEqual(len(b),8+33**3*3);self.assertEqual(b[:4],b'FOL1');self.assertEqual(hashlib.sha256(b).hexdigest(),digest)
if __name__=='__main__':unittest.main()
