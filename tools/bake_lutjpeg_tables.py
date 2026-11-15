"""Reproduce the three GPL builtin-model LUTs using vendored source only."""
from pathlib import Path
import tempfile,subprocess,sys
root=Path(__file__).resolve().parents[1];m=root/'firmware/modules/lutjpeg';u=m/'upstream'
with tempfile.TemporaryDirectory(prefix='lut-bake-') as tmp:
 p=Path(tmp)
 subprocess.run(['gcc','-O2','-DFOS_DEV_TEST','-DCONFIG_600D=1','-I'+str(u),str(root/'tests/lutjpeg/host.c'),str(m/'catalog.c')]+[str(u/n) for n in ['fos_dev.c','fos_jpeg.c','fos_print.c','fos_looks.c']]+['-lm','-o',str(p/'bake')],check=True)
 subprocess.run([str(p/'bake'),'-T',str(p)],check=True)
 for i in [0,2,7]:
  name=f'LOOK{i:02d}.LUT';dst=root/'firmware/data/lutjpeg'/name
  if '--check' in sys.argv:assert dst.read_bytes()==(p/name).read_bytes(),name
  else:dst.write_bytes((p/name).read_bytes())
 print('PASS three model-derived LUTs reproduce exactly from vendored GPL source')
