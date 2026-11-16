"""Package the version selected by scripts/photo-ui-version.txt, preserving older deliveries.

Build first with scripts/build-photo-ui.sh and run the documented host checks.
Source ZIP contains exact working files, rather than claiming HEAD alone built them.
"""
import functools
import hashlib
import json
import operator
from pathlib import Path
import re
import struct
import subprocess
import zipfile
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--private-test', action='store_true', help='Label local artifacts as unpublished private test builds.')
args = parser.parse_args()

root = Path(__file__).resolve().parents[1]
release = (root / 'scripts/photo-ui-version.txt').read_text().strip()
assert re.fullmatch(r'v[0-9][A-Za-z0-9._-]*', release)
version = f'PHOTO-UI-{release}.600D.102'
notes = root/f'PHOTO_UI_{release.upper()}.md'
assert notes.is_file(), 'Add release notes for this version before packaging.'
out = root / f'deliveries/photo-ui-{release}-full'
out.mkdir(parents=True, exist_ok=True)
source = out / f'600D-PHOTO-UI-{release}-source.zip'
full = out / f'600D-102-PHOTO-UI-{release}-FULL-EXPERIMENTAL.zip'
assert not any(p.exists() for p in [source, full, out/'BUILD_INFO.json']), 'Delivery exists; bump scripts/photo-ui-version.txt first.'
subprocess.run(['git', 'ls-files', '--error-unmatch', 'scripts/photo-ui-version.txt'], cwd=root, check=True, stdout=subprocess.DEVNULL)
base = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
sha256 = lambda data: hashlib.sha256(data).hexdigest()
with zipfile.ZipFile(root/'firmware/platform/600D.102/build/magiclantern.zip') as z:
    assert z.testzip() is None
    payload = {name: z.read(name) for name in z.namelist() if not name.endswith('/')}
binary = payload['autoexec.bin']
assert version.encode() in binary, 'Build version differs from version file; rebuild first.'
assert len(binary) % 4 == 0
assert functools.reduce(operator.xor, struct.unpack('<%dI' % (len(binary)//4), binary), 0) == 0
assert payload['ML/data/PHOTO_TH.BIN'] == (root/'firmware/data/photo_thai/PHOTO_TH.BIN').read_bytes()
if b'ML/data/FILMART.BIN' in binary:
    assert payload['ML/data/FILMART.BIN']==(root/'firmware/data/photo_thai/FILMART.BIN').read_bytes()
    assert payload['ML/data/FILMART_OFL.txt']==(root/'assets/fonts/barlow/OFL.txt').read_bytes()
if b'ML/data/TONEINFO.BIN' in binary:
    assert payload['ML/data/TONEINFO.BIN']==(root/'firmware/data/photo_thai/TONEINFO.BIN').read_bytes()
    assert payload['ML/data/TONEINFO_OFL.txt']==(root/'assets/fonts/noto-sans-thai/OFL.txt').read_bytes()
assert sha256(payload['ML-SETUP.FIR']) == '7e33d8a2a71ad92c412d291fb0ecb6de0b9404c212571d51fd82f839688070e3'
assert {Path(n).stem for n in payload if n.endswith('.mo')} == {'adv_int','autoexpo','deflick','dual_iso','ettr','file_man','img_name','pic_view','arkanoid','lua','lutjpeg'}
for name in ['LOOK00.LUT','LOOK02.LUT','LOOK07.LUT']:
    assert payload['ML/LUTDATA/'+name]==(root/'firmware/data/lutjpeg'/name).read_bytes()
assert payload['ML/LUTDATA/RECIPES.json'] == (root/'firmware/data/lutjpeg/RECIPES.json').read_bytes()
if release == 'v3r6-lut6':
    for name in ['FILM7.BIN'] + [f'LV{i:03d}.FLV' for i in [8,9,11,15,19,20,26]]:
        assert payload['ML/LUTDATA/'+name] == (root/'firmware/data/lutjpeg'/name).read_bytes()
    assert len([n for n in payload if n.endswith('.FLV')]) == 7
table_info = json.loads(payload['ML/LUTDATA/TABLES.json'])
catalog_bytes=payload['ML/LUTDATA/caduceus/CATALOG.json']
assert sha256(catalog_bytes)==table_info['catalog_sha256']
for name,digest in json.loads(catalog_bytes)['files'].items():
    assert sha256(payload['ML/LUTDATA/caduceus/'+name])==digest
assert payload['ML/LUTDATA/caduceus/LICENSE'] and payload['ML/LUTDATA/caduceus/NOTICE']
assert table_info['recipe_sha256'] == sha256(payload['ML/LUTDATA/RECIPES.json'])
for name, digest in table_info['sha256'].items():
    assert sha256(payload['ML/LUTDATA/'+name]) == digest
assert b'ML/LUTDATA/LUTJPEG.BIN' in binary
assert b'Film lab' in binary
assert b'FILMERR.LCK' in binary
assert 'ML/LUTDATA/FILMERR.LCK' not in payload
assert 'ML/LUTDATA/DIRECT.ON' in payload
assert 'ML/SETTINGS/lutjpeg.en' not in payload
import zlib
hdr=struct.unpack('<16I',payload['ML/LUTDATA/LUTJPEG.BIN'][:64])
assert hdr[13]==zlib.crc32(binary) and hdr[14]==len(binary)
assert hdr[11]==zlib.crc32(payload['ML/modules/600D_102.sym'])
assert hdr[15]==zlib.crc32(payload['ML/LUTDATA/LUTJPEG.BIN'][:60])
assert b'ML/LUTLOAD.LOG' in binary
assert b'Module startup busy; retry' not in binary
assert b'lutjpeg_bootstrap' in payload['ML/modules/lutjpeg.mo']
assert b'lutjpeg_abi' in payload['ML/modules/lutjpeg.mo']
assert payload['ML/LUTDATA/COPYING']==(root/'firmware/modules/lutjpeg/upstream/COPYING').read_bytes()
# Only indexed project sources: do not sweep local files or build outputs into the archive.
indexed = subprocess.check_output(['git','ls-files','-z'], cwd=root, text=True).split('\0')
files = sorted(n for n in indexed if n and n != 'DOWNLOAD.md' and (n.split('/')[0] in {'firmware','scripts','tests','tools','assets','translations','previews','release','research'} or '/' not in n or n=='baseline-600d-102/bootstrap-toolchain.sh'))
prefix = f'600D-photo-ui-{release}-source/'
with zipfile.ZipFile(source, 'w', zipfile.ZIP_DEFLATED) as z:
    for name in files:
        z.write(root/name, prefix+name)
with zipfile.ZipFile(source) as z:
    assert z.testzip() is None
    for name in files:
        assert z.read(prefix+name) == (root/name).read_bytes()
info = {
    'project':'FIRMWARE ADD-ON Thai-Menu',
    'project_website':'https://itsamachom.github.io/EOS_ROM_MLV_Modify/',
    'distribution_status':('Private test; unpublished; hardware unvalidated' if args.private_test else 'Public Experimental; not Stable or source-audit complete'),
    'version': version, 'target':'600D.102', 'base_commit':base,
    'source_state':'Exact indexed build/source files and notes; generated DOWNLOAD.md is omitted to avoid checksum self-reference. Base commit plus working changes, not base alone.',
    'source_zip_sha256':sha256(source.read_bytes()), 'autoexec_sha256':sha256(binary),
    'camera_tested':False, 'qemu_tested':False, 'installer_modified':False,
    'packager_checks':['build-version identity','ZIP CRC','autoexec XOR','Thai atlas identity','module inventory','unchanged installer','exact working-source bytes'],
    'dependency_source_status':'Inherited dietlibc corresponding source remains unverified; see included release/v3r2/PROVENANCE.md and READINESS.json.',
    'ai_assistance':'Codex/LLM-assisted fork; see AI_ASSISTANCE.md and release notes.',
}
with zipfile.ZipFile(full, 'w', zipfile.ZIP_DEFLATED) as z:
    for name, data in payload.items():
        z.writestr(name, data)
    z.write(source, 'SOURCE/'+source.name)
    z.writestr('SOURCE/changes-from-base.patch', subprocess.check_output(['git','diff','--binary','HEAD','--','firmware','scripts','tests','tools','assets','translations','*.md'], cwd=root))
    z.writestr('BUILD_INFO.json', json.dumps(info,indent=2)+'\n')
    for path,name in [('INSTALL_TH.md','INSTALL_TH.md'),('INSTALL_EN.md','INSTALL_EN.md'),('firmware/doc/INSTALL.txt','INSTALL_UPSTREAM.txt'),('GAMES_TH.md','GAMES_TH.md'),('firmware/COPYING','COPYING'),('AI_ASSISTANCE.md','AI_ASSISTANCE.md'),('THIRD_PARTY_NOTICES.md','THIRD_PARTY_NOTICES.md')]:
        z.write(root/path, name)
    z.write(notes, 'UPDATE_TH.md')
    for name in files:
        if name.startswith('release/v3r2/'):
            z.write(root/name,name)
    z.writestr('PAYLOAD_SHA256SUMS',''.join(f'{sha256(data)}  {name}\n' for name,data in sorted(payload.items())))
with zipfile.ZipFile(full) as z:
    assert z.testzip() is None
    for name,data in payload.items():
        assert z.read(name) == data
    assert sha256(z.read('SOURCE/'+source.name)) == info['source_zip_sha256']
info['full_zip_sha256'] = sha256(full.read_bytes())
artifacts = [full, source]
if args.private_test:
    update = out / f'600D-102-Thai-Menu-{release}-TEST-UPDATE.zip'
    update_payload = {n: data for n, data in payload.items() if n == 'autoexec.bin' or n.startswith('ML/')}
    with zipfile.ZipFile(update, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, data in update_payload.items():
            z.writestr(name, data)
        z.write(notes, 'UPDATE_TH.md')
        z.write(root/'firmware/COPYING', 'COPYING')
        z.write(root/'THIRD_PARTY_NOTICES.md', 'THIRD_PARTY_NOTICES.md')
        z.writestr('BUILD_INFO.json', json.dumps(info, indent=2)+'\n')
        z.writestr('SOURCE.txt', f'Private unpublished test. Matching source: {source.name}\nSHA256: {info["source_zip_sha256"]}\n')
        z.writestr('PAYLOAD_SHA256SUMS', ''.join(f'{sha256(data)}  {name}\n' for name, data in sorted(update_payload.items())))
    with zipfile.ZipFile(update) as z:
        assert z.testzip() is None and not any(n.upper().endswith('.FIR') for n in z.namelist())
        for name, data in update_payload.items():
            assert z.read(name) == data
    info['test_update_zip_sha256'] = sha256(update.read_bytes())
    artifacts.append(update)
(out/'BUILD_INFO.json').write_text(json.dumps(info,indent=2)+'\n')
(out/'SHA256SUMS').write_text(''.join(f'{sha256(p.read_bytes())}  {p.name}\n' for p in artifacts+[out/'BUILD_INFO.json']))
print('PASS package version, source identity, module inventory, CRC and XOR')
print(full)
print(info['full_zip_sha256'])
