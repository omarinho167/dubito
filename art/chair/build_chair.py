"""Dubito chair, Blender 5.3. Run with --background --factory-startup --python.
Only the generated CHAIR scene is replaced; unrelated scenes are preserved.
-- --iteration 1 produces the first comparison; default 2 is the delivery.
Iteration 1: dossier trop court, lattes trop fines, traverse basse trop haute.
Iteration 2: height .940, slats .040, lean 8.3 deg, lower rail .535.
After rebuilding, run bake_chair.py headless to restore the baked oak textures,
then run review_textured.py with system Python/Pillow for reference sheets.
Remaining: inferred back, four side bars visible in profile.
Perspective measurements (front 1361x1824): top y249, seat y1034, floor y1675;
seat x345..1016; posts x424..938. Side top x1028 y234, knee x947 y1035.
These are perspective constraints, not orthographic dimensions. Hidden back inferred.
"""
from pathlib import Path
import bpy, bmesh, math, sys, json, os, argparse
from mathutils import Vector
# Named dimensions in metres; seat top is the imposed scale anchor.
SEAT_TOP = .450
SEAT_WIDTH = .420
SEAT_DEPTH = .420
SEAT_THICKNESS = .020
SEAT_REAR_NOTCH_Y = .151
SEAT_REAR_NOTCH_X = .164
TOTAL_HEIGHT = .940
LEG_SECTION = .038
LEG_X = .181
FRONT_Y = -.180
REAR_FOOT_Y = .180
REAR_KNEE_Y = .172
REAR_TOP_Y = .246
KNEE_HEIGHT = .430
APRON_HEIGHT = .055
APRON_THICKNESS = .022
BACK_TOP_RAIL_HEIGHT = .055
BACK_LOW_RAIL_Z = .535
BACK_LOW_RAIL_HEIGHT = .035
SLAT_WIDTH = .040
SLAT_THICKNESS = .018
SLAT_CENTRES = (-.066, 0, .066)
STRETCHER_HEIGHTS = (.220, .285)
STRETCHER_SECTION = (.022, .028)
BEVEL_WIDTH = .0015
SEAT_BEVEL = .004
BEVEL_SEGMENTS = 2
WOOD_SRGB = (192/255,138/255,85/255)
ROUGHNESS = .6
ROOT = Path(__file__).resolve().parents[2]
REVIEW = Path(os.environ['TEMP'])/'dubito-review'/'chair'
SKILLS = ROOT/'.agents'/'skills'
args = argparse.ArgumentParser(); args.add_argument('--iteration',type=int,default=2)
a = args.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
if a.iteration == 1:
    TOTAL_HEIGHT=.900; REAR_TOP_Y=.220; SLAT_WIDTH=.028; BACK_LOW_RAIL_Z=.590
REVIEW.mkdir(parents=True,exist_ok=True)
# New isolated scene. Never mutate objects belonging to another scene.
old=bpy.data.scenes.get('CHAIR')
if old:
    for ob in list(old.objects):
        if ob.get('dubito_chair_generated'): bpy.data.objects.remove(ob,do_unlink=True)
    bpy.data.scenes.remove(old)
sc=bpy.data.scenes.new('CHAIR'); bpy.context.window.scene=sc
sc.unit_settings.system='METRIC'; sc.unit_settings.scale_length=1
parts_col=bpy.data.collections.new('Chair_Source'); sc.collection.children.link(parts_col)
ref_col=bpy.data.collections.new('REF'); sc.collection.children.link(ref_col)
export_col=bpy.data.collections.new('EXPORT'); sc.collection.children.link(export_col)
def srgb(c): return c/12.92 if c<=.04045 else ((c+.055)/1.055)**2.4
mat=bpy.data.materials.new('M_Wood'); mat.use_nodes=True
p=next(n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
p.inputs['Base Color'].default_value=(*map(srgb,WOOD_SRGB),1)
p.inputs['Roughness'].default_value=ROUGHNESS
mat.diffuse_color=(*map(srgb,WOOD_SRGB),1)
parts=[]
def meshpart(name,verts,faces,bevel=BEVEL_WIDTH):
    me=bpy.data.meshes.new(name); me.from_pydata(verts,[],faces); me.update()
    bm=bmesh.new(); bm.from_mesh(me); bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces)); bm.to_mesh(me); bm.free()
    ob=bpy.data.objects.new(name,me); parts_col.objects.link(ob); ob['dubito_chair_generated']=True
    me.materials.append(mat)
    for f in me.polygons: f.use_smooth=True
    md=ob.modifiers.new('Soft_wood_edges','BEVEL'); md.width=bevel; md.segments=BEVEL_SEGMENTS
    md=ob.modifiers.new('Planar_normals','WEIGHTED_NORMAL'); md.keep_sharp=True; md.weight=50
    parts.append(ob); return ob
faces=[(0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]
def box(name,centre,size,bevel=BEVEL_WIDTH):
    v=[(centre[0]+x*size[0]/2,centre[1]+y*size[1]/2,centre[2]+z*size[2]/2) for z in (-1,1) for x,y in [(-1,-1),(1,-1),(1,1),(-1,1)]]
    return meshpart(name,v,faces,bevel)
def ry(z):
    return REAR_KNEE_Y+(REAR_TOP_Y-REAR_KNEE_Y)*(z-KNEE_HEIGHT)/(TOTAL_HEIGHT-KNEE_HEIGHT)
for x in (-LEG_X,LEG_X):
    box('FrontLeg_L' if x<0 else 'FrontLeg_R',(x,FRONT_Y,(SEAT_TOP-SEAT_THICKNESS)/2),(LEG_SECTION,LEG_SECTION,SEAT_TOP-SEAT_THICKNESS+.004))
    # One continuous bent rear post; ring at the seat, no seam in the knee.
    verts=[]
    for z,y in [(0,REAR_FOOT_Y),(KNEE_HEIGHT,REAR_KNEE_Y),(TOTAL_HEIGHT,REAR_TOP_Y)]:
        verts.extend([(x+dx*LEG_SECTION/2,y+dy*LEG_SECTION/2,z) for dx,dy in [(-1,-1),(1,-1),(1,1),(-1,1)]])
    fs=[(0,3,2,1),(8,9,10,11)]+[(k+i,k+(i+1)%4,k+(i+1)%4+4,k+i+4) for k in (0,4) for i in range(4)]
    meshpart('RearPost_L' if x<0 else 'RearPost_R',verts,fs)
    box('ApronSide',(x,(FRONT_Y+REAR_KNEE_Y)/2,SEAT_TOP-SEAT_THICKNESS-APRON_HEIGHT/2+.006),(APRON_THICKNESS,REAR_KNEE_Y-FRONT_Y,APRON_HEIGHT))
    for z in STRETCHER_HEIGHTS:
        box('SideStretcher',(x,(FRONT_Y+REAR_FOOT_Y)/2,z),(STRETCHER_SECTION[0],REAR_FOOT_Y-FRONT_Y,STRETCHER_SECTION[1]))
for y in (FRONT_Y,REAR_KNEE_Y):
    box('ApronFront' if y<0 else 'ApronBack',(0,y,SEAT_TOP-SEAT_THICKNESS-APRON_HEIGHT/2+.006),(2*LEG_X,APRON_THICKNESS,APRON_HEIGHT))
# Seat rear edge stops before the posts: custom outline avoids visible intersections.
sx,sy=SEAT_WIDTH/2,SEAT_DEPTH/2
nx,ny=SEAT_REAR_NOTCH_X,SEAT_REAR_NOTCH_Y
outline=[(-sx,-sy),(sx,-sy),(sx,ny),(nx,ny),(nx,sy),(-nx,sy),(-nx,ny),(-sx,ny)]
v=[(x,y,z) for z in (SEAT_TOP-SEAT_THICKNESS,SEAT_TOP) for x,y in outline]
n=len(outline)
f=[tuple(reversed(range(n))),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
meshpart('Seat',v,f,SEAT_BEVEL)
# Rails/slats follow the same lean as the rear posts, via shear in Y.
def backbox(name,x,z,w,h,t):
    verts=[(x+dx*w/2,ry(z+dz*h/2)+dy*t/2,z+dz*h/2) for dz in (-1,1) for dx,dy in [(-1,-1),(1,-1),(1,1),(-1,1)]]
    return meshpart(name,verts,faces)
ztop=TOTAL_HEIGHT-.010-BACK_TOP_RAIL_HEIGHT/2
backbox('BackTopRail',0,ztop,2*LEG_X,BACK_TOP_RAIL_HEIGHT,.029)
backbox('BackLowerRail',0,BACK_LOW_RAIL_Z,2*LEG_X,BACK_LOW_RAIL_HEIGHT,.027)
zlo=BACK_LOW_RAIL_Z+BACK_LOW_RAIL_HEIGHT/2-.004
zhi=ztop-BACK_TOP_RAIL_HEIGHT/2+.004
for i,x in enumerate(SLAT_CENTRES): backbox('BackSlat_'+str(i+1),x,(zlo+zhi)/2,SLAT_WIDTH,zhi-zlo,SLAT_THICKNESS)
# Feet footprint centred on origin. Front leg base starts exactly on ground.
for ob in parts:
    for v in ob.data.vertices:
        if v.co.z<0: v.co.z=0
# Evaluated joined export copy; source bevel stacks stay editable.
bpy.context.view_layer.update(); deps=bpy.context.evaluated_depsgraph_get()
vs=[]; fs=[]
for ob in parts:
    ev=ob.evaluated_get(deps); me=ev.to_mesh(); off=len(vs)
    vs.extend([tuple(ob.matrix_world@v.co) for v in me.vertices]); fs.extend([tuple(off+i for i in f.vertices) for f in me.polygons]); ev.to_mesh_clear()
me=bpy.data.meshes.new('SM_Chair'); me.from_pydata(vs,[],fs); me.update(); me.materials.append(mat)
chair=bpy.data.objects.new('SM_Chair',me); export_col.objects.link(chair); chair['dubito_chair_generated']=True
for ob in parts: ob.hide_render=True; ob.hide_set(True)
bpy.ops.object.select_all(action='DESELECT'); chair.select_set(True); bpy.context.view_layer.objects.active=chair
# Unique UVs for possible future texture/lightmap; no texture needed for greybox.
bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='SELECT'); bpy.ops.uv.smart_project(angle_limit=math.radians(66),island_margin=.015); bpy.ops.object.mode_set(mode='OBJECT')
tri=chair.modifiers.new('Delivery_triangles','TRIANGULATE'); bpy.ops.object.modifier_apply(modifier=tri.name)
# Preserve crisp planar faces and small curved bevels with weighted normals.
for f in me.polygons: f.use_smooth=True
wn=chair.modifiers.new('Delivery_normals','WEIGHTED_NORMAL'); wn.keep_sharp=True
bpy.ops.object.modifier_apply(modifier=wn.name)
refs={'Front':'Wooden_dining_chair_product_photo_35b59d42_2K.jpg','Side':'Side_profile_of_chair_49429f02_2K.jpg'}
for view,file in refs.items():
    ob=bpy.data.objects.new('REF_'+view,None); ref_col.objects.link(ob); ob.empty_display_type='IMAGE'; ob.data=bpy.data.images.load(str(ROOT/'art/chair/ref'/file)); ob.data.pack(); ob.empty_display_size=1.2; ob.hide_render=True; ob.hide_set(True); ob['dubito_chair_generated']=True
# Perspective cameras: front reveals seat top; profile has front pointing screen-left.
def camera(name,loc,target,lens=55):
    d=bpy.data.cameras.new(name); d.lens=lens; d.sensor_fit='HORIZONTAL'; d.sensor_width=36
    ob=bpy.data.objects.new(name,d); sc.collection.objects.link(ob); ob.location=loc; ob.rotation_euler=(Vector(target)-ob.location).to_track_quat('-Z','Y').to_euler(); ob['dubito_chair_generated']=True; return ob
cams=[camera('CAM_Front',(0,-2.40,.90),(0,0,.485),92),camera('CAM_Side',(2.4,-.06,.90),(0,.025,.485),92),camera('CAM_ThreeQuarter',(1.45,-1.9,1.15),(0,.025,.47),58)]
sc.camera=cams[0]; sc.render.engine='BLENDER_WORKBENCH'; sc.render.resolution_x=680; sc.render.resolution_y=912; sc.render.resolution_percentage=100
sc.render.image_settings.file_format='PNG'; sc.render.image_settings.color_mode='RGBA'; sc.render.film_transparent=False
sh=sc.display.shading; sh.light='STUDIO'; sh.color_type='SINGLE'; sh.single_color=(.55,.55,.55); sh.show_shadows=True; sh.show_cavity=True; sh.cavity_type='BOTH'; sh.background_type='WORLD'; sc.world=bpy.data.worlds.new('Chair_World'); sc.world.color=(.65,.65,.65)
sc.view_settings.view_transform='Standard'
for cam in cams[:2]:
    sc.camera=cam
    for mode in ('clay','silhouette'):
        sh.light='FLAT' if mode=='silhouette' else 'STUDIO'; sh.single_color=(0,0,0) if mode=='silhouette' else (.55,.55,.55); sh.show_shadows=mode!='silhouette'; sh.show_cavity=mode!='silhouette'
        sc.render.filepath=str(REVIEW/f'iteration{a.iteration}_{cam.name[4:]}_{mode}.png'); bpy.ops.render.render(write_still=True)
sh.light='STUDIO'; sh.color_type='MATERIAL'; sh.show_cavity=True; sh.show_shadows=True; sc.camera=cams[2]
sc.render.filepath=str(REVIEW/f'iteration{a.iteration}_threequarter.png'); bpy.ops.render.render(write_still=True)
# Strict QA on exactly the delivered mesh. No source/camera/reference gets exported.
sys.path.insert(0,str(SKILLS/'blender-asset-qa/scripts')); import qa_check, review_shots
qa=qa_check.run(objects=[chair.name],profile='game')
(REVIEW/f'iteration{a.iteration}_qa.json').write_text(json.dumps(qa,indent=2),encoding='utf8')
print('CHAIR_QA',json.dumps(qa))
assert qa['pass'], qa['errors']
assert len(chair.data.polygons)<=3000
for mode in ('random',):
    review_shots.run(objects=[chair.name],out=str(REVIEW/f'iteration{a.iteration}_{mode}.png'),views=['front','right','back','three_quarter'],mode=mode,res=480)
sc.camera=cams[2]; sc.render.engine='BLENDER_WORKBENCH'; sh.color_type='MATERIAL'
if a.iteration==2:
    dest=ROOT/'unity/Assets/Dubito/Art/Props/Chair/SM_Chair.fbx'; dest.parent.mkdir(parents=True,exist_ok=True)
    bpy.ops.object.select_all(action='DESELECT'); chair.select_set(True); bpy.context.view_layer.objects.active=chair
    bpy.ops.export_scene.fbx(filepath=str(dest),use_selection=True,object_types={'MESH'},global_scale=1,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',axis_forward='-Z',axis_up='Y',bake_space_transform=True,use_mesh_modifiers=True,mesh_smooth_type='FACE',use_triangles=True,add_leaf_bones=False,bake_anim=False)
    sc.camera=cams[2]; bpy.context.preferences.filepaths.save_version=0
    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'art/chair/chair.blend'))
    stats={'dimensions_xyz_m':list(chair.dimensions),'triangles':len(chair.data.polygons),'materials':len(chair.material_slots),'seat_top_m':SEAT_TOP,'seat_m':[SEAT_WIDTH,SEAT_DEPTH,SEAT_THICKNESS],'leg_section_m':LEG_SECTION,'front_blender':[0,-1,0],'fbx_forward':'-Z','fbx_up':'Y','unity_import_verified':False,'back_inferred':True}
    (REVIEW/'measurements.json').write_text(json.dumps(stats,indent=2)); print('CHAIR_STATS',json.dumps(stats))
    # FBX roundtrip in a separate scene; do not save this over the source.
    rt=bpy.data.scenes.new('Roundtrip'); bpy.context.window.scene=rt
    bpy.ops.import_scene.fbx(filepath=str(dest))
    result=[{'name':o.name,'type':o.type,'dimensions':list(o.dimensions),'scale':list(o.scale),'rotation':list(o.rotation_euler)} for o in rt.objects]
    (REVIEW/'roundtrip.json').write_text(json.dumps(result,indent=2)); print('ROUNDTRIP',json.dumps(result))



