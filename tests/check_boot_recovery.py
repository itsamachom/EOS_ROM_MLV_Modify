"""Check recovery entry points in production source, host execution and ARM output."""
from pathlib import Path
import os,struct,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as temp:
 exe=Path(temp)/'adapter'
 subprocess.run(['gcc','-O2','-fsanitize=address,undefined','-I'+str(root/'tests/lutjpeg'),'-I'+str(root/'firmware/modules/lutjpeg/upstream'),str(root/'tests/lutjpeg/recovery_adapter.c'),'-lm','-o',str(exe)],check=True)
 subprocess.run([exe],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
subprocess.run(['python3',str(root/'tests/check_module_recovery.py')],check=True)
b=root/'firmware/platform/600D.102/build'
nm=Path(os.environ.get('ARM_BINPATH','/workspace/ml-env/toolchain/usr/bin'))/'arm-none-eabi-nm'
syms=subprocess.check_output([nm,b/'lut-overlay/image.elf'],text=True)
entry=next(int(line.split()[0],16) for line in syms.splitlines() if line.endswith(' lutjpeg_bootstrap'))
# Actual ARM entry consists only of `mov r0, #1; bx lr`: no initializer call.
assert struct.unpack_from('<II',(b/'lut-overlay/image.bin').read_bytes(),entry)==(0xe3a00001,0xe12fff1e)
core_syms=(b/'magiclantern.sym').read_text()
for name in ['photo_lut_load_task','photo_film_key','photo_film_filter','photo_film_draw','photo_film_register']:
 assert name not in core_syms,name
for name,hook in [('gui-common.c','photo_film_key'),('tweaks.c','photo_film_filter'),('photo-menu-600d.h','photo_film_draw')]:
 assert hook not in (root/'firmware/src'/name).read_text()
print('PASS ARM initializer rejects activation; core contains no film loader task or lut6 key/draw/filter hooks')
