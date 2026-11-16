"""Verify offline policy against actual loader and source integration."""
from pathlib import Path
import subprocess,sys
root=Path(__file__).resolve().parents[1]
subprocess.run([sys.executable,root/'tests/check_module_recovery.py'],check=True)
m=root/'firmware/modules/lutjpeg'
s='\n'.join((m/n).read_text() for n in ['lutjpeg.c','offline-converter.h','offline-screen.h'])
for forbidden in ['prop_request_change','PROP_PIC_QUALITY','observe_shot','monitor_task','display_filter','photo_film_register']:
 assert forbidden not in s,forbidden
assert 'if(!photo_offline_allowed())return 1;' in s
for file,hook in [('gui-common.c','photo_offline'),('tweaks.c','photo_offline')]:
 assert hook not in (root/'firmware/src'/file).read_text()
assert 'BATCH_LIMIT 4' in s and 'RAW_LIMIT 128' in s
print('PASS manual gate; no capture-property changes, shot monitor, global key hook or display filter; bounded four-file batch')
