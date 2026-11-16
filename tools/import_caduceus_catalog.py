"""Import attributed external recipe data and generate neutral model identities. Idempotent; no firmware/code changes."""
from pathlib import Path
import json,shutil,hashlib,subprocess
import argparse
ap=argparse.ArgumentParser();ap.add_argument('--source',type=Path,required=True);args=ap.parse_args()
r=Path(__file__).resolve().parents[1];src=args.source;
assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=src,text=True).strip()=='a0c351f5398a4e7b69738103bda3e5c92f81971d','Review the new catalog revision before importing'
dest=r/'firmware/data/lutjpeg/caduceus';dest.mkdir(exist_ok=True)
files=['standard-1.2.json','tokyo-days-1.0.json','tokyo-nights-1.0.json','summer-1.0.json','autumn-1.0.json','winter-1.0.json','film-noir-1.0.json','standard-1.1.json','standard-1.0.json']
packs=[];looks=[];offsets=[]
for n in files:
 b=(src/'webapp/src/data/packs'/n).read_bytes();(dest/n).write_bytes(b);j=json.loads(b);offsets.append(len(looks));packs.append(j)
 for look in j['looks']:looks.append({'index':8+len(looks),'pack':j['id'],'pack_name':j['name']+' '+j['version'],'recipe':look})
for n in ['LICENSE','NOTICE']:shutil.copyfile(src/n,dest/n)
meta={'repository':'https://github.com/cakelesscoder/caduceus-ep7','commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=src,text=True).strip(),'license':'Apache-2.0','scope':'All nine published webapp pack JSON files, including both legacy Standard revisions; no recipe fields removed. Data assets retained separately from GPL processing code.','files':{n:hashlib.sha256((dest/n).read_bytes()).hexdigest() for n in files},'looks':looks}
(dest/'CATALOG.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n')
m=r/'firmware/modules/lutjpeg'
# Metadata and neutral upstream model only. Actual recipe values remain external data tables.
rows=[];models=[]
for e in looks:
 key='cd%03d'%e['index'];name=e['recipe']['name'];pack=e['pack_name'];mono=int(e['recipe']['kind']=='mono')
 rows.append('    { '+json.dumps(key)+', "Caduceus", '+json.dumps(name)+', "LUT", "Recipe data; see card catalog", 100,100,PS_STANDARD,0,0,0,0,5500,0,0,0,0,0,0,"",FOS_GROUP_COLOUR,0 },')
 models.append('    { '+json.dumps(key)+',5500,1,0,1,{.18f,.18f,.18f},{0,0,0},{0,0,0},{1,1,1},0,{{0}},0,.75f,{1,1,1},0,1,'+str(mono)+',{.299f,.587f,.114f} },')
(m/'caduceus-films.h').write_text('/* Generated metadata, from attributed Caduceus pack names. Numeric recipes are external Apache-2.0 data. */\n'+'\n'.join(rows)+'\n')
(m/'caduceus-models.h').write_text('/* Neutral model identities. Actual colour tables are generated offline. */\n'+'\n'.join(models)+'\n')
labels=[p['name']+' '+p['version'] for p in packs]+['Native / lut4']
allnames=[e['recipe']['name'] for e in looks]+['Portra 400 (lut4)','Ektar 100 (lut4)','Tri-X 400 (lut4)']
ids=[e['index'] for e in looks]+[0,2,7]
notes=[e['recipe']['notes'] for e in looks]+['Original lut4 warm skin look','Original lut4 vivid colour look','Original lut4 monochrome look']
head='/* Generated catalog metadata; see ML/LUTDATA/caduceus/NOTICE and CATALOG.json. */\n'
for name,values,typ in [('film_names',allnames,'const char *const'),('film_notes',notes,'const char *const'),('pack_names',labels,'const char *const'),('pack_offsets',offsets+[len(looks)],'const int'),('pack_counts',[len(p['looks']) for p in packs]+[3],'const int'),('films',ids,'const int')]:
 head+=f'static {typ} {name}[]={{'+','.join(json.dumps(x) for x in values)+'};\n'
(m/'film-library.h').write_text(head)
labels=[p['name']+' '+p['version'] for p in packs]+['Native / lut4']
allnames=[e['recipe']['name'] for e in looks]+['Portra 400 (lut4)','Ektar 100 (lut4)','Tri-X 400 (lut4)']
ids=[e['index'] for e in looks]+[0,2,7]
notes=[e['recipe']['notes'] for e in looks]+['Original lut4 warm skin look','Original lut4 vivid colour look','Original lut4 monochrome look']
head='/* Generated catalog metadata; see ML/LUTDATA/caduceus/NOTICE and CATALOG.json. */\n'
for name,values,typ in [('film_names',allnames,'const char *'),('film_notes',notes,'const char *'),('pack_names',labels,'const char *'),('pack_offsets',offsets+[len(looks)],'const int'),('pack_counts',[len(p['looks']) for p in packs]+[3],'const int'),('films',ids,'const int')]:
 head+=f'static {typ} {name}[]={{'+','.join(json.dumps(x) for x in values)+'};\n'
(m/'film-library.h').write_text(head)
p=m/'catalog.c';s=p.read_text().replace('\n};\n\nconst struct fos_film *fos_films=builtin;','\n#include "caduceus-films.h"\n};\n\nconst struct fos_film *fos_films=builtin;').replace('int fos_nfilms=8;','int fos_nfilms=sizeof(builtin)/sizeof(builtin[0]);');p.write_text(s)
p=m/'upstream/fos_looks.c';s=p.read_text().replace('\n};\nconst int fos_nlooks','\n#include "../caduceus-models.h"\n};\nconst int fos_nlooks');p.write_text(s)
p=r/'tests/lutjpeg/host.c';s=p.read_text().replace('const int films[]={0,2,7};fos_dev_cache(argv[2]);','fos_dev_cache(argv[2]);').replace('for(int i=0;i<3;i++)if(!fos_dev_table(films[i]))return 2;','for(int i=0;i<fos_nfilms;i++)if(!fos_dev_table(i))return 2;');p.write_text(s)
rows=['{0,0,0,{256,256,256}},']*8
for e in meta['looks']:
 fx=e['recipe'].get('effect') or {};kind={'bloom':1,'halation':1,'clarity':2}.get(fx.get('type'),0);strength=int(fx.get('strength',0));tint={'neutral':[256]*3,'red':[256,128,97],'warm':[256,223,184],'amber':[256,192,108],'cool':[200,230,282]}.get(fx.get('tint','neutral'),[256]*3)
 # grain field is NR advice in modern EP7, not a film grain amplitude.
 rows.append('{%d,%d,%d,{%s}},'%(kind,strength,0,','.join(map(str,tint))))
(r/'firmware/modules/lutjpeg/film-effects.h').write_text('/* Numeric FX metadata derived from attributed external recipe assets. */\nstruct film_effect {int kind,strength,grain,tint[3];};\nstatic const struct film_effect film_effects[]={\n'+'\n'.join(rows)+'\n};\n')
