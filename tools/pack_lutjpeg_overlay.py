"""Bind the 600D LUT engine to its matching core, then pack audited base relocations.
No camera-side TCC or startup module state is needed to read this image.
"""
from pathlib import Path
import argparse,subprocess,struct,zlib,tempfile,json
root=Path(__file__).resolve().parents[1]
ap=argparse.ArgumentParser();ap.add_argument('--core-build',type=Path,required=True);ap.add_argument('--arm-bin',type=Path,required=True);args=ap.parse_args()
b=args.core_build.resolve();m=root/'firmware/modules/lutjpeg/build';out=b/'lut-overlay';out.mkdir(exist_ok=True)
syms={l.split()[1]:int(l.split()[0],16) for l in (b/'magiclantern.sym').read_text().splitlines() if len(l.split())==2}
deps=(m/'lutjpeg.dep').read_text().splitlines();assert set(deps)<=syms.keys()
(out/'core.lds').write_text('\n'.join(f'{n} = 0x{syms[n]:x};' for n in deps))
(out/'image.lds').write_text('''SECTIONS {
 . = 0;
 .text : { *(.text*) }
 .rodata : ALIGN(16) { *(.rodata*) }
 .data : ALIGN(16) { *(.data*) *(.module_strings*) }
 _lut_file_end = .;
 .bss (NOLOAD) : ALIGN(16) { *(.bss*) *(COMMON) }
 _lut_mem_end = .;
 /DISCARD/ : { *(.comment) *(.ARM.attributes) *(.ARM.exidx*) *(.ARM.extab*) }
}''')
subprocess.run([str(args.arm_bin/'arm-none-eabi-ld'),'-T',str(out/'image.lds'),'-T',str(out/'core.lds'),'--emit-relocs','-o',str(out/'image.elf'),str(m/'lutjpeg.mo')],check=True)
subprocess.run([str(args.arm_bin/'arm-none-eabi-objcopy'),'-O','binary',str(out/'image.elf'),str(out/'image.bin')],check=True)
e=(out/'image.elf').read_bytes();eh=struct.unpack_from('<16sHHIIIIIHHHHHH',e)
assert eh[0][:6]==b'\x7fELF\x01\x01' and eh[2]==40
sections=[struct.unpack_from('<10I',e,eh[6]+i*eh[11]) for i in range(eh[12])]
st=next(s for s in sections if s[1]==2);strings=sections[st[6]];strings=e[strings[4]:strings[4]+strings[5]]
symbols=[];names={}
for p in range(st[4],st[4]+st[5],st[9]):
 n,v,size,info,other,sec=struct.unpack_from('<IIIBBH',e,p);name=strings[n:].split(b'\0',1)[0].decode();symbols.append((name,v,sec));names[name]=v
image=(out/'image.bin').read_bytes();memory=(names['_lut_mem_end']+15)&~15
assert len(image)==names['_lut_file_end'] and len(image)<=memory<=512*1024
relocs=[];external_pc=[]
for s in sections:
 if s[1]!=9:continue
 for p in range(s[4],s[4]+s[5],8):
  pos,info=struct.unpack_from('<II',e,p);kind=info&255;name,value,sec=symbols[info>>8]
  assert kind in (2,28,29,40),(kind,name)
  if kind in (28,29) and sec==0xfff1:external_pc.append((pos,name))
  if kind==2 and 0<sec<0xff00:
   assert pos%4==0 and pos+4<=len(image)
   word=struct.unpack_from('<I',image,pos)[0];assert word<=memory,(name,pos,word,memory)
   relocs.append(pos)
assert not external_pc,('Unrebasable external PC-relative call',external_pc)
assert len(relocs)==len(set(relocs)) and len(relocs)<=4096
rel=struct.pack('<%dI'%len(relocs),*sorted(relocs));core=(b/'autoexec.bin').read_bytes();sym=(b/'magiclantern.sym').read_bytes()
entries=[names[n] for n in ['lutjpeg_bootstrap','lutjpeg_shutdown','lutjpeg_abi']]
assert all(x%4==0 and x+4<=len(image) for x in entries)
header=struct.pack('<16I',0x3354554c,1,0x600d0102,len(image),memory,len(relocs),*entries,zlib.crc32(image),zlib.crc32(rel),zlib.crc32(sym),0x4c554933,zlib.crc32(core),len(core),0)
header=header[:60]+struct.pack('<I',zlib.crc32(header[:60]))
result=header+image+rel;dest=b/'zip/ML/LUTDATA/LUTJPEG.BIN';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(result)
(out/'MANIFEST.json').write_text(json.dumps({'layout':1,'target':'600D.102','image_bytes':len(image),'memory_bytes':memory,'relocations':len(relocs),'external_imports':deps,'entry_offsets':entries,'core_crc32':f'{zlib.crc32(core):08x}','symbol_crc32':f'{zlib.crc32(sym):08x}','camera_tcc_required':False},indent=2)+'\n')
print(f'PASS bound LUT overlay: {len(image)} bytes, {memory} memory, {len(relocs)} checked base relocations; no external PC-relative calls')
