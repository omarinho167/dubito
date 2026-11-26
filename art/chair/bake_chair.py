"""Reproducible texture pass on the validated chair (never rebuilds geometry).
Blender 5.3 --background --factory-startup --python-exit-code 1 --python art/chair/bake_chair.py
Run build_chair.py first only when rebuilding the original shape. No CPU Cycles fallback.
"""
from pathlib import Path
import bpy, sys, os, math, time, json, hashlib
from collections import defaultdict
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'unity/Assets/Dubito/Art/Props/Chair'
REVIEW=Path(os.environ['TEMP'])/'dubito-review/chair'; REVIEW.mkdir(parents=True,exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=str(ROOT/'art/chair/chair.blend'))
sc=bpy.data.scenes['CHAIR']; bpy.context.window.scene=sc
ob=bpy.data.objects['SM_Chair']; me=ob.data
def signature():
    return hashlib.sha256(repr(([tuple(v.co) for v in me.vertices],[tuple(p.vertices) for p in me.polygons])).encode()).hexdigest()
original=signature()
prefs=bpy.context.preferences.addons['cycles'].preferences
for backend in ('OPTIX','CUDA'):
    try:
        prefs.compute_device_type=backend; prefs.get_devices()
        devices=[d for d in prefs.devices if d.type==backend and '3070' in d.name]
        if devices: break
    except Exception: devices=[]
else: raise RuntimeError('RTX 3070 unavailable: CPU fallback forbidden')
for d in prefs.devices: d.use=d in devices
sc.render.engine='CYCLES'; sc.cycles.device='GPU'; sc.cycles.samples=32
print('GPU_DEVICE',backend,[d.name for d in devices],flush=True)
# Connected physical pieces, independent from the original smart UV map.
parent=list(range(len(me.vertices)))
def find(i):
    while parent[i]!=i: parent[i]=parent[parent[i]]; i=parent[i]
    return i
for e in me.edges:
    a,b=map(find,e.vertices); parent[a]=b
pieces=defaultdict(list)
for v in me.vertices: pieces[find(v.index)].append(v.index)
wood=me.attributes.get('WoodCoord') or me.attributes.new('WoodCoord','FLOAT_VECTOR','POINT')
knot=me.attributes.get('KnotCoord') or me.attributes.new('KnotCoord','FLOAT_VECTOR','POINT')
axes={}; piece_info=[]
for idx,ids in enumerate(pieces.values()):
    pts=[me.vertices[i].co for i in ids]
    lo=Vector(tuple(min(p[j] for p in pts) for j in range(3)))
    hi=Vector(tuple(max(p[j] for p in pts) for j in range(3)))
    size=hi-lo; centre=(hi+lo)/2
    # Seat grain runs across the width, as in the supplied front photograph.
    seat=size.x>.4 and size.y>.4
    axis=0 if seat else max(range(3),key=lambda j:size[j])
    long=Vector(tuple(float(j==axis) for j in range(3)))
    if axis==2 and size.y>.06: long=Vector((0,.145,1)).normalized()
    across=Vector((1,0,0)) if axis!=0 else Vector((0,1,0))
    depth=long.cross(across).normalized()
    for i in ids:
        p=me.vertices[i].co-centre
        wc=Vector((p.dot(across),p.dot(depth),p.dot(long)))
        wood.data[i].vector=wc+Vector((.025+(idx%3)*.013,idx*.003,idx*.031))
        # A few small knots; centred on the broad face, not one per strip.
        knot.data[i].vector=(wc-Vector((.004,0,size[axis]*.17))) if idx in (0,1,12,16) else Vector((10,10,10))
        axes[i]=long
    piece_info.append({'piece':idx,'vertices':len(ids),'axis':list(long),'seat':seat})
# Coplanar connected charts: orthographic mapping is an isometry, including bevels.
adj=defaultdict(list)
for p in me.polygons:
    for edge in p.edge_keys: adj[tuple(sorted(edge))].append(p.index)
fp=list(range(len(me.polygons)))
def ff(i):
    while fp[i]!=i: fp[i]=fp[fp[i]]; i=fp[i]
    return i
for fs in adj.values():
    if len(fs)==2:
        a,b=fs
        if me.polygons[a].normal.dot(me.polygons[b].normal)>.999999:
            fp[ff(a)]=ff(b)
charts=defaultdict(list)
for p in me.polygons: charts[ff(p.index)].append(p.index)
rects=[]
for fs in charts.values():
    p=me.polygons[fs[0]]; n=p.normal; v=axes[p.vertices[0]]
    v=v-n*v.dot(n)
    if v.length<.01:
        v=n.cross(Vector((1,0,0)))
        if v.length<.01: v=n.cross(Vector((0,1,0)))
    v.normalize(); u=v.cross(n).normalized()
    coords={li:(me.vertices[me.loops[li].vertex_index].co.dot(u),me.vertices[me.loops[li].vertex_index].co.dot(v)) for f in fs for li in me.polygons[f].loop_indices}
    xmin=min(c[0] for c in coords.values()); ymin=min(c[1] for c in coords.values())
    w=max(c[0] for c in coords.values())-xmin; h=max(c[1] for c in coords.values())-ymin
    rects.append(dict(coords=coords,xmin=xmin,ymin=ymin,w=w,h=h))
rects.sort(key=lambda r:r['h'],reverse=True)
RES=2048; PAD=20
def pack(density,assign=False):
    x=y=PAD; row=0
    for r in rects:
        w=r['w']*density; h=r['h']*density
        if x+w+PAD>RES: x=PAD; y+=row+PAD; row=0
        if y+h+PAD>RES: return False
        if assign: r['px']=x; r['py']=y
        x+=w+PAD; row=max(row,h)
    return True
low,high=1.,2048.
for _ in range(40):
    mid=(low+high)/2
    if pack(mid): low=mid
    else: high=mid
density=low*.999; assert pack(density,True)
uv=me.uv_layers.active or me.uv_layers.new(name='UVMap')
for r in rects:
    for li,(x,y) in r['coords'].items():
        uv.data[li].uv=((r['px']+(x-r['xmin'])*density)/RES,(r['py']+(y-r['ymin'])*density)/RES)
# Parametric light oak, metre-based coordinates continuous over each wooden piece.
mat=me.materials[0]; assert mat.name=='M_Wood'
nt=mat.node_tree; nt.nodes.clear()
def node(t,name):
    n=nt.nodes.new(t); n.label=name; return n
def link(a,out,b,inp): nt.links.new(a.outputs[out],b.inputs[inp])
def vm(a,scale):
    n=node('ShaderNodeVectorMath','Metre grain scale'); n.operation='MULTIPLY'; link(a,'Vector',n,0); n.inputs[1].default_value=scale; return n
attr=node('ShaderNodeAttribute','Per-piece longitudinal coordinates'); attr.attribute_name='WoodCoord'
vec=vm(attr,(330,330,7))
noise=node('ShaderNodeTexNoise','Elongated oak fibres'); noise.inputs['Scale'].default_value=1; noise.inputs['Detail'].default_value=3; noise.inputs['Roughness'].default_value=.65; link(vec,'Vector',noise,'Vector')
growth=vm(attr,(90,90,4))
wave=node('ShaderNodeTexWave','Elongated growth rings'); wave.wave_type='RINGS'; wave.rings_direction='Y'; wave.inputs['Scale'].default_value=1.2; wave.inputs['Distortion'].default_value=3.5; wave.inputs['Detail Scale'].default_value=.8; wave.inputs['Detail Roughness'].default_value=.65; link(growth,'Vector',wave,'Vector')
mix=node('ShaderNodeMixRGB','Subtle fibre blend'); mix.blend_type='MIX'; mix.inputs[0].default_value=.38; link(noise,'Fac',mix,1); link(wave,'Fac',mix,2)
# Sparse elongated knot rings with a soft dark heart, blending into the fibres.
ka=node('ShaderNodeAttribute','Sparse knot centres'); ka.attribute_name='KnotCoord'
kv=vm(ka,(155,0,32)); dist=node('ShaderNodeVectorMath','Knot radius'); dist.operation='LENGTH'; link(kv,'Vector',dist,0)
mask=node('ShaderNodeMapRange','Knot falloff'); link(dist,'Value',mask,'Value'); mask.inputs['From Min'].default_value=.1; mask.inputs['From Max'].default_value=2.4; mask.inputs['To Min'].default_value=.65; mask.inputs['To Max'].default_value=0
ringfreq=node('ShaderNodeMath','Knot growth rings'); ringfreq.operation='MULTIPLY'; ringfreq.inputs[1].default_value=13; link(dist,'Value',ringfreq,0)
ring=node('ShaderNodeMath','Irregular elliptical knot rings'); ring.operation='SINE'; link(ringfreq,0,ring,0)
ringamp=node('ShaderNodeMath','Ring contrast'); ringamp.operation='MULTIPLY_ADD'; ringamp.inputs[1].default_value=.18; ringamp.inputs[2].default_value=.30; link(ring,0,ringamp,0)
km=node('ShaderNodeMixRGB','Knot darkening'); km.blend_type='MULTIPLY'; link(mask,'Result',km,0); link(mix,'Color',km,1); link(ringamp,0,km,2)
ramp=node('ShaderNodeValToRGB','Light oak palette'); link(km,'Color',ramp,'Fac')
def srgb(x): return x/12.92 if x<=.04045 else ((x+.055)/1.055)**2.4
for el,c in zip(ramp.color_ramp.elements,[(.40,.22,.09),(.865,.663,.435)]): el.color=(*map(srgb,c),1)
el=ramp.color_ramp.elements.new(.35); el.color=(*map(srgb,(.72,.49,.28)),1)
el=ramp.color_ramp.elements.new(.65); el.color=(*map(srgb,(.82,.60,.37)),1)
p=node('ShaderNodeBsdfPrincipled','Oak'); p.inputs['Roughness'].default_value=.6; link(ramp,'Color',p,'Base Color')
out=node('ShaderNodeOutputMaterial','Surface'); link(p,'BSDF',out,'Surface')
previous=bpy.data.materials.get('M_Wood_Procedural')
if previous: bpy.data.materials.remove(previous)
procedural=mat.copy(); procedural.name='M_Wood_Procedural'; procedural.use_fake_user=True
# No normal map: sealed fine grain is expressed in colour; avoid exaggerated grooves.
img=bpy.data.images.get('T_Chair_BaseColor')
if img: bpy.data.images.remove(img)
img=bpy.data.images.new('T_Chair_BaseColor',width=RES,height=RES,alpha=False)
img.colorspace_settings.name='sRGB'; img.filepath_raw=str(OUT/'T_Chair_BaseColor.png'); img.file_format='PNG'
target=node('ShaderNodeTexImage','Bake destination'); target.image=img; nt.nodes.active=target
bpy.ops.object.select_all(action='DESELECT'); ob.hide_set(False); ob.select_set(True); bpy.context.view_layer.objects.active=ob
sc.render.bake.use_pass_direct=False; sc.render.bake.use_pass_indirect=False; sc.render.bake.use_pass_color=True; sc.render.bake.margin=8
start=time.perf_counter(); bpy.ops.object.bake(type='DIFFUSE'); bake_seconds=time.perf_counter()-start
img.save()
nt.nodes.clear(); p=node('ShaderNodeBsdfPrincipled','Baked oak'); p.inputs['Roughness'].default_value=.6
target=node('ShaderNodeTexImage','sRGB oak base colour'); target.image=img; link(target,'Color',p,'Base Color'); out=node('ShaderNodeOutputMaterial','Surface'); link(p,'BSDF',out,'Surface')
# Relative texture path survives moving the repository.
img.filepath=bpy.path.relpath(str(OUT/'T_Chair_BaseColor.png'),start=str(ROOT/'art/chair'))
assert signature()==original, 'Geometry was changed'
# Numeric UV audit: disjoint bounding rectangles guarantee inter-chart separation;
# compare singular values of every triangle's metric to catch stretching.
import numpy as np
stretch=[]; densities=[]
for f in me.polygons:
    a,b,c=[me.vertices[i].co for i in f.vertices]
    x=(b-a).normalized(); n=(b-a).cross(c-a).normalized(); y=n.cross(x)
    B=np.array([[(b-a).dot(x),(c-a).dot(x)],[(b-a).dot(y),(c-a).dot(y)]])
    q=[uv.data[i].uv for i in f.loop_indices]
    U=np.array([[q[1].x-q[0].x,q[2].x-q[0].x],[q[1].y-q[0].y,q[2].y-q[0].y]])*RES
    sv=np.linalg.svd(U@np.linalg.inv(B),compute_uv=False)
    stretch.append(float(sv.max()/sv.min())); densities.append(float(np.sqrt(sv.prod())))
for i,r in enumerate(rects):
    for s in rects[i+1:]:
        dx=max(s['px']-(r['px']+r['w']*density),r['px']-(s['px']+s['w']*density))
        dy=max(s['py']-(r['py']+r['h']*density),r['py']-(s['py']+s['h']*density))
        assert max(dx,dy)>=PAD-.001
uv_audit={'max_anisotropy':max(stretch),'density_min':min(densities),'density_max':max(densities),'density_spread':max(densities)/min(densities),'min_chart_gap_px':PAD,'texture_edge_padding_px':PAD}
assert uv_audit['density_spread']<1.01
sys.path.insert(0,str(ROOT/'.agents/skills/blender-asset-qa/scripts')); import qa_check
qa=qa_check.run(objects=[ob.name],profile='game'); (REVIEW/'textured_qa.json').write_text(json.dumps(qa,indent=2))
assert qa['pass'],qa['errors']
# Identical FBX settings to build_chair.py; textures remain external.
bpy.ops.export_scene.fbx(filepath=str(OUT/'SM_Chair.fbx'),use_selection=True,object_types={'MESH'},global_scale=1,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',axis_forward='-Z',axis_up='Y',bake_space_transform=True,use_mesh_modifiers=True,mesh_smooth_type='FACE',use_triangles=True,add_leaf_bones=False,bake_anim=False,embed_textures=False,path_mode='RELATIVE')
# Studio presentation, GPU Cycles only, no additions to the exported mesh.
for o in list(sc.objects):
    if o.name.startswith('TEXTURED_'): bpy.data.objects.remove(o,do_unlink=True)
sc.world.use_nodes=True; bg=next(n for n in sc.world.node_tree.nodes if n.type=='BACKGROUND'); bg.inputs[0].default_value=(.65,.65,.65,1); bg.inputs[1].default_value=.65
def light(name,loc,power,size):
    d=bpy.data.lights.new(name,'AREA'); d.energy=power; d.shape='DISK'; d.size=size
    o=bpy.data.objects.new(name,d); sc.collection.objects.link(o); o.location=loc; o.rotation_euler=(Vector((0,0,.45))-o.location).to_track_quat('-Z','Y').to_euler()
light('TEXTURED_Key',(-2,-3,3),160,3); light('TEXTURED_Fill',(2,0,2),90,2)
sc.render.resolution_x=900; sc.render.resolution_y=1200; sc.render.resolution_percentage=100
sc.render.film_transparent=False; sc.view_settings.view_transform='AgX'; sc.cycles.use_denoising=True; sc.cycles.denoiser='OPTIX'
for name in ('Front','ThreeQuarter'):
    sc.camera=bpy.data.objects['CAM_'+name]; sc.render.filepath=str(REVIEW/('textured_'+name+'.png')); bpy.ops.render.render(write_still=True)
sc.camera=bpy.data.objects['CAM_ThreeQuarter']; bpy.context.preferences.filepaths.save_version=0
bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'art/chair/chair.blend'))
report={'device':[d.name for d in devices],'backend':backend,'bake_seconds':bake_seconds,'resolution':[RES,RES],'triangles':len(me.polygons),'geometry_unchanged':signature()==original,'geometry_sha256':original,'pieces':len(pieces),'uv_charts':len(rects),'density_px_m':density,'uv_audit':uv_audit,'chart_gap_px':PAD,'bake_margin_px':8,'uv_method':'Coplanar isometric charts, longitudinal direction V; 20px nonoverlapping rectangle gaps','normal':'Omitted: fine sealed wood, colour grain only','piece_axes':piece_info,'qa':qa}
(REVIEW/'textured_report.json').write_text(json.dumps(report,indent=2)); print('TEXTURED_REPORT',json.dumps(report),flush=True)
# Round-trip verification in an isolated scene, never saved over the chair scene.
rt=bpy.data.scenes.new('Textured_Roundtrip'); bpy.context.window.scene=rt
bpy.ops.import_scene.fbx(filepath=str(OUT/'SM_Chair.fbx'))
rtob=next(o for o in rt.objects if o.type=='MESH')
bpy.context.view_layer.update()
assert len(rtob.data.polygons)==len(me.polygons)
expected=(ob.dimensions.x,ob.dimensions.z,ob.dimensions.y)
assert all(abs(a-b)<1e-5 for a,b in zip(rtob.dimensions,expected)),(list(rtob.dimensions),list(expected))
(REVIEW/'textured_roundtrip.json').write_text(json.dumps({'triangles':len(rtob.data.polygons),'dimensions':list(rtob.dimensions),'materials':[m.name for m in rtob.data.materials]},indent=2))
