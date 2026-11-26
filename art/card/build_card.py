"""Poker card, Blender Z up; image top -Y. Run with Blender --background --factory-startup --python this.py.
UVs use planar bounds, deliberately clipping the image corners; back is reversed in U.
No image textures are saved or exported. Unity: face +Y, image top +Z.
"""
WIDTH=.063
HEIGHT=.088
THICKNESS=.0008
CORNER_RADIUS=.003
CORNER_SEGMENTS=6

import bpy, bmesh, math, sys, json, os, tempfile
from pathlib import Path
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
REVIEW = Path(tempfile.gettempdir()) / 'dubito-review' / 'models'
REVIEW.mkdir(parents=True, exist_ok=True)
assert bpy.app.background, 'Headless only'
for obj in list(bpy.data.objects): bpy.data.objects.remove(obj, do_unlink=True)
scene=bpy.context.scene
scene.unit_settings.system='METRIC'
scene.unit_settings.scale_length=1.0
scene.unit_settings.length_unit='METERS'
def material(name, hexcolor, metallic=0):
    m=bpy.data.materials.new(name)
    srgb=[int(hexcolor[i:i+2],16)/255 for i in (0,2,4)]
    rgb=[c/12.92 if c<=.04045 else ((c+.055)/1.055)**2.4 for c in srgb]
    m.diffuse_color=(*rgb,1)
    p=next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    p.inputs['Base Color'].default_value=(*rgb,1)
    p.inputs['Roughness'].default_value=.65
    p.inputs['Metallic'].default_value=metallic
    return m
def mesh_object(name, verts, faces, materials, indices):
    me=bpy.data.meshes.new(name); me.from_pydata(verts,[],faces); me.update()
    ob=bpy.data.objects.new(name,me); scene.collection.objects.link(ob)
    for m in materials: me.materials.append(m)
    for p,i in zip(me.polygons,indices): p.material_index=i
    bm=bmesh.new(); bm.from_mesh(me)
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
    bmesh.ops.triangulate(bm,faces=list(bm.faces))
    bm.to_mesh(me); bm.free(); me.update()
    bpy.context.view_layer.objects.active=ob; ob.select_set(True)
    return ob
def lathe(name, profile, segments, materials, felt_top=False):
    verts=[]; rings=[]; faces=[]; mids=[]
    for r,z in profile:
        ring=[]
        for j in range(segments if r else 1):
            a=2*math.pi*j/segments; ring.append(len(verts)); verts.append((r*math.cos(a),r*math.sin(a),z))
        rings.append(ring)
    for k,(a,b) in enumerate(zip(rings,rings[1:])):
        for j in range(segments):
            t=(j+1)%segments
            f=(a[0],b[t],b[j]) if len(a)==1 else ((a[j],a[t],b[0]) if len(b)==1 else (a[j],a[t],b[t],b[j]))
            faces.append(f); mids.append(0 if felt_top and k==len(rings)-2 else (1 if felt_top else 0))
    ob=mesh_object(name,verts,faces,materials,mids)
    # Unique per-triangle UV tiles: uniform-color props need no texture seams.
    uv=ob.data.uv_layers.new(name='UVMap')
    n=math.ceil(math.sqrt(len(ob.data.polygons)))
    for p in ob.data.polygons:
        x=p.index%n; y=p.index//n
        for li,co in zip(p.loop_indices,((.1,.1),(.9,.1),(.1,.9))): uv.data[li].uv=((x+co[0])/n,(y+co[1])/n)
    return ob
def finish(ob, asset, destination, budget):
    bpy.context.view_layer.update(); ob.data.calc_loop_triangles()
    assert len(ob.data.loop_triangles)<=budget
    sys.path.insert(0,str(ROOT/'.agents/skills/blender-asset-qa/scripts'))
    import qa_check
    qa=qa_check.run(objects=[ob.name], profile='game', eps=.00005)
    (REVIEW/(asset+'_qa.json')).write_text(json.dumps(qa,indent=2),encoding='utf8')
    assert qa['pass'], qa['errors']
    blend=ROOT/'art'/asset/(asset+'.blend')
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    out=ROOT/destination; out.parent.mkdir(parents=True,exist_ok=True)
    # Unity static preset. Blender +Z -> FBX +Y; Blender -Y -> FBX +Z.
    # Card image top is -Y in Blender; front U follows -X, back U follows +X.
    bpy.ops.export_scene.fbx(filepath=str(out), use_selection=True, object_types={'MESH'},
        global_scale=1, apply_unit_scale=True, apply_scale_options='FBX_SCALE_ALL',
        axis_forward='-Z',axis_up='Y',bake_space_transform=True,
        use_mesh_modifiers=True,mesh_smooth_type='FACE',use_triangles=True,
        add_leaf_bones=False,bake_anim=False,path_mode='AUTO')
    print('BUILD',json.dumps({'asset':asset,'dimensions_blender':list(ob.dimensions),'tris':len(ob.data.loop_triangles),'qa':qa['pass'],'warnings':qa['warnings'],'fbx':str(out)}))

outline=[]
for cx,cy,start in [(WIDTH/2-CORNER_RADIUS,HEIGHT/2-CORNER_RADIUS,0),(-WIDTH/2+CORNER_RADIUS,HEIGHT/2-CORNER_RADIUS,90),(-WIDTH/2+CORNER_RADIUS,-HEIGHT/2+CORNER_RADIUS,180),(WIDTH/2-CORNER_RADIUS,-HEIGHT/2+CORNER_RADIUS,270)]:
    for j in range(CORNER_SEGMENTS+1):
        a=math.radians(start+j*90/CORNER_SEGMENTS)
        outline.append((cx+CORNER_RADIUS*math.cos(a),cy+CORNER_RADIUS*math.sin(a)))
n=len(outline)
verts=[(x,y,z) for z in (-THICKNESS/2,THICKNESS/2) for x,y in outline]
faces=[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]
indices=[1,0]
for j in range(n): faces.append((j,(j+1)%n,(j+1)%n+n,j+n)); indices.append(1)
ob=mesh_object('SM_Card',verts,faces,[material('M_CardFace','F4F0E5'),material('M_CardBack','344C6A')],indices)
uv=ob.data.uv_layers.new(name='UVMap')
for p in ob.data.polygons:
    surface=abs(p.normal.z)>.9
    for j,li in enumerate(p.loop_indices):
        co=ob.data.vertices[ob.data.loops[li].vertex_index].co
        if surface:
            uv.data[li].uv=(.5+(-co.x if p.normal.z>0 else co.x)/WIDTH,.5-co.y/HEIGHT)
        else:
            uv.data[li].uv=((.01,.01),(.03,.01),(.01,.03))[j]
finish(ob,'card','unity/Assets/Dubito/Art/Cards/Model/SM_Card.fbx',200)
