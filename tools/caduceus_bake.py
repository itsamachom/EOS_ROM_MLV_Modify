"""Independent Canon renderer for external Apache-2.0 recipe DATA, not Olympus firmware.

The same controls are retained; Canon RAW colour, tone-band response and spatial
FX are approximations. No E-P7 code or camera firmware tables are linked.
"""
import json
from pathlib import Path
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
DATA=ROOT/'firmware/data/lutjpeg/caduceus'
CENTRES=np.array([60,30,15,0,330,270,240,210,180,150,120,90],float)

def render(lin,r):
    rgb=np.maximum(np.asarray(lin,float),0).copy()
    wb=r.get('wb') or {};ab=wb.get('a_b',0)*.015;gm=wb.get('g_m',0)*.015
    rgb*=np.array([1+ab,1+gm,1-ab])
    m=np.eye(3)
    for (a,b),v in zip([(0,1),(0,2),(1,0),(1,2),(2,0),(2,1)],r.get('matrix',[0]*6)):
        m[a,b]=v/128;m[a,a]-=v/128
    rgb=np.maximum(rgb@m.T,0)
    # A modest shoulder before sRGB: Canon adaptation, not the Olympus ISP LUT.
    if r.get('curve')=='auto':rgb=rgb/(1+.10*rgb)
    rgb=np.clip(rgb,0,1)
    rgb=np.where(rgb<=.0031308,12.92*rgb,1.055*rgb**(1/2.4)-.055)
    y=rgb@np.array([.299,.587,.114]);tone=r.get('tone') or {}
    if not any(tone.values()):tone=(r.get('advice') or {}).get('tone') or tone
    # Smooth luminance bands, with endpoint controls; explicitly calibrated in
    # this Canon model rather than interpreting Olympus slider units as EV.
    sh=np.maximum(0,1-y/.5)**2;hi=np.maximum(0,(y-.5)/.5)**2;mid=np.exp(-((y-.5)/.23)**2)
    delta=.018*(tone.get('shadows',0)*sh+tone.get('highlights',0)*hi+tone.get('midtones',0)*mid)
    delta+=.012*tone.get('blacks',0)*(1-y)**3+.012*tone.get('whites',0)*y**3
    delta+=.035*(r.get('advice') or {}).get('contrast',0)*(y-.5)*np.sin(np.pi*y)
    rgb=np.clip(rgb+delta[...,None],0,1)
    # RGB opponent hue for smooth overlap; neutral gate prevents grey tinting.
    y=rgb@np.array([.299,.587,.114]);u=rgb[...,2]-y;v=rgb[...,0]-y
    h=np.arctan2(v,u);ch=np.hypot(u,v)
    refs=np.array([[1,1,0],[1,.5,0],[1,.25,0],[1,0,0],[1,0,.5],[.5,0,1],[0,0,1],[0,.5,1],[0,1,1],[0,1,.5],[0,1,0],[.5,1,0]])
    ry=refs@np.array([.299,.587,.114]);angles=np.arctan2(refs[:,0]-ry,refs[:,2]-ry)
    dist=(h[...,None]-angles+np.pi)%(2*np.pi)-np.pi
    w=np.exp(-.5*(dist/(np.pi/6))**2);w/=w.sum(axis=-1,keepdims=True)
    gate=np.minimum(1,ch/.08)
    shift=(w@np.array(r.get('hue',[0]*12)))*5*np.pi/180*gate
    sat=np.maximum(0,1+.10*(w@np.array(r.get('sat',[0]*12))))
    light=.012*(w@np.array(r.get('light',[0]*12)))*gate
    u=ch*sat*np.cos(h+shift);v=ch*sat*np.sin(h+shift);y=y+light
    red=y+v;blue=y+u;green=(y-.299*red-.114*blue)/.587
    rgb=np.clip(np.stack([red,green,blue],axis=-1),0,1)
    if r['kind']=='mono':
        filt=(r.get('advice') or {}).get('mono_filter') or {}
        filters={'none':[.299,.587,.114],'yellow':[.42,.53,.05],'orange':[.52,.44,.04],'red':[.65,.32,.03],'green':[.22,.70,.08],'blue':[.18,.47,.35]}
        neutral=np.array(filters['none']);target=np.array(filters.get(filt.get('colour'),filters['none']))
        weights=neutral+(target-neutral)*min(3,filt.get('strength',1))/3
        y=rgb@(weights/weights.sum());rgb=np.repeat(y[...,None],3,axis=-1)
    else:
        s=r.get('split') or {};y=rgb@np.array([.299,.587,.114])
        for part,weight in [('sh',y*(1-y)**2),('hl',y*y*(1-y))]:
            angle=CENTRES[int(s.get(part+'_color',0))]*np.pi/180
            color=np.cos(angle-np.array([0,2*np.pi/3,4*np.pi/3]));color-=color.mean()
            rgb+=weight[...,None]*float(s.get(part,0))*.035*color
    return np.clip(rgb,0,1)


def bake_into(tmp,check=False):
    catalog=json.loads((DATA/'CATALOG.json').read_text())
    coords=np.indices((33,33,33)).reshape(3,-1).T[:,::-1]/32
    black=.0108
    lin=np.maximum(0,(10**((coords*1023-685)/300)-black)/(1-black))
    results={}
    for entry in catalog['looks']:
        filename=f"LOOK{entry['index']:02d}.LUT"
        source=(tmp/filename).read_bytes()
        pixels=render(lin,entry['recipe'])
        assert pixels.shape==(33**3,3) and np.isfinite(pixels).all()
        data=source[:8]+(pixels*255+.5).astype('uint8').tobytes()
        dest=ROOT/'firmware/data/lutjpeg'/filename
        if check:assert dest.read_bytes()==data,filename
        else:dest.write_bytes(data)
        results[filename]=entry['pack']+'/'+entry['recipe']['name']
    return results
