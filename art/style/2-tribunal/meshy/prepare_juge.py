"""Rebuild the Unity judge from the unchanged Meshy source; run with Blender 5.3 -b."""
import bpy, bmesh, json, sys, math
from pathlib import Path
from mathutils import Vector, Matrix

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
OUT = ROOT/'unity/Assets/Dubito/Art/Characters/Juge'
REVIEW = ROOT/'art/style/2-tribunal/review'
OUT.mkdir(parents=True, exist_ok=True)
REVIEW.mkdir(parents=True, exist_ok=True)
sys.path.insert(0, str(ROOT/'.agents/skills/blender-asset-qa/scripts'))
import qa_check

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=str(HERE/'juge-rig.fbx'))
rig = next(o for o in bpy.context.scene.objects if o.type == 'ARMATURE')
mesh = next(o for o in bpy.context.scene.objects if o.type == 'MESH')
rig.animation_data_clear()
for a in list(bpy.data.actions): bpy.data.actions.remove(a)
bpy.context.view_layer.update()
before = [mesh.matrix_world @ v.co for v in mesh.data.vertices]
rig.name = 'RIG_Juge'
mesh.name = 'CHR_Juge'
mesh.data.name = 'CHR_Juge'
bpy.ops.object.select_all(action='DESELECT')
for o in (rig, mesh): o.select_set(True)
bpy.context.view_layer.objects.active = rig
bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
bpy.context.view_layer.update()
drift = max((p - mesh.matrix_world @ v.co).length for p,v in zip(before,mesh.data.vertices))
assert drift < 1e-5, drift
image = next(i for i in bpy.data.images if i.packed_file)
image.name = 'T_Juge_BaseColor'
image.filepath_raw = str(OUT/'T_Juge_BaseColor.png')
image.file_format = 'PNG'
image.save()
mat = mesh.data.materials[0]
mat.name = 'M_Juge'
bsdf = next(n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
bsdf.inputs['Roughness'].default_value = 0.72
bsdf.inputs['Metallic'].default_value = 0

def weights():
    active = [[g.weight for g in v.groups if g.weight > 1e-6] for v in mesh.data.vertices]
    return {'unweighted':sum(not g for g in active), 'max_influences':max(map(len,active)),
            'max_normalization_error':max(abs(sum(g)-1) for g in active)}
report = {'transform_max_drift_m':drift, 'weights_before':weights()}
# Preserve Meshy's authored corner normals while welding geometric UV seams.
# FBX needs those smooth normals explicitly; rebuilding a bmesh alone loses them.
uv=mesh.data.uv_layers.active.data
def corner_key(vertex, texcoord):
    return tuple(round(float(x),6) for x in (*vertex,*texcoord))
normals={corner_key(mesh.data.vertices[l.vertex_index].co,uv[l.index].uv):tuple(mesh.data.corner_normals[l.index].vector)
         for l in mesh.data.loops}
report['source_has_custom_normals']=mesh.data.has_custom_normals
# Meshy splits UV seams into coincident vertices. Weld only exact-position seams;
# loop UVs remain separate, but the skin now shares a continuous joint surface.
bm=bmesh.new();bm.from_mesh(mesh.data)
old_count=len(bm.verts)
bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=1e-6)
# Two spectacle surfaces meet at the same position but are separate shells.
# Keep that contact split rather than creating a three-face edge when welding.
contacts=[e for e in bm.edges if len(e.link_faces)>2]
bmesh.ops.split_edges(bm,edges=contacts)
report['split_nonmanifold_contacts']=len(contacts)
bm.to_mesh(mesh.data);bm.free();mesh.data.update()
uv=mesh.data.uv_layers.active.data
restored=[normals[corner_key(mesh.data.vertices[l.vertex_index].co,uv[l.index].uv)] for l in mesh.data.loops]
for p in mesh.data.polygons: p.use_smooth=True
mesh.data.normals_split_custom_set(restored)
report['welded_exact_seam_vertices']=old_count-len(mesh.data.vertices)
if report['weights_before']['max_influences'] > 4:
    bpy.context.view_layer.objects.active = mesh
    bpy.ops.object.vertex_group_limit_total(limit=4)
if report['weights_before']['max_normalization_error'] > 1e-5 or report['weights_before']['max_influences'] > 4:
    bpy.context.view_layer.objects.active = mesh
    bpy.ops.object.vertex_group_normalize_all(lock_active=False)
report['weights_after'] = weights()
assert report['weights_after']['unweighted'] == 0
mesh.data.calc_loop_triangles()
report['geometry'] = {'vertices':len(mesh.data.vertices), 'faces':len(mesh.data.polygons),
                      'triangles':len(mesh.data.loop_triangles), 'dimensions_m':list(mesh.dimensions),
                      'bones':len(rig.data.bones), 'material':mat.name, 'texture':list(image.size)}
report['qa_tolerance_note']='10 micrometres: the default 1 mm flags nearby spectacle rims and wig folds as coplanar overlaps; these are distinct detail surfaces.'
report['qa_rest'] = qa_check.run(objects=[mesh.name],profile='game',eps=0.00001)

# Export the bind pose only. Existing Mixamo Humanoid clips are retargeted in Unity.
bpy.ops.object.select_all(action='DESELECT')
for o in (rig,mesh): o.select_set(True)
bpy.context.view_layer.objects.active = rig
bpy.ops.wm.save_as_mainfile(filepath=str(HERE/'juge-unity.blend'))
bpy.ops.export_scene.fbx(filepath=str(OUT/'CHR_Juge.fbx'),use_selection=True,object_types={'MESH','ARMATURE'},
    global_scale=1,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',axis_forward='-Z',axis_up='Y',
    bake_space_transform=False,use_mesh_modifiers=True,mesh_smooth_type='FACE',use_triangles=True,
    add_leaf_bones=False,use_armature_deform_only=True,bake_anim=False,path_mode='STRIP')

# Diagnostic seated stress pose (not an exported animation): transform in armature/world axes.
def rotate(name,axis,degrees):
    p=rig.pose.bones[name]
    r=Matrix.Rotation(math.radians(degrees),4,axis)
    p.matrix = Matrix.Translation(p.head) @ r @ Matrix.Translation(-p.head) @ p.matrix
    bpy.context.view_layer.update()
for side in ('Left','Right'):
    rotate(side+'UpLeg','X',-85)
    rotate(side+'Leg','X',95)
    rotate(side+'Arm','Y',75 if side=='Left' else -75)
    rotate(side+'ForeArm','X',-95)
report['qa_stress'] = qa_check.run(objects=[mesh.name],profile='game',eps=0.00001)
deps = bpy.context.evaluated_depsgraph_get()
ev=mesh.evaluated_get(deps); deformed=ev.to_mesh()
ratios=[]
for e in mesh.data.edges:
    a,b=e.vertices
    rest=(mesh.data.vertices[a].co-mesh.data.vertices[b].co).length
    if rest>1e-5: ratios.append(((deformed.vertices[a].co-deformed.vertices[b].co).length/rest,a,b))
ev.to_mesh_clear()
report['stress_edge_ratios']={'max':max(ratios), 'over_3x':sum(x[0]>3 for x in ratios)}

# EEVEE textured and wire multi-angle review; no CPU Cycles path.
sc=bpy.context.scene
sc.render.engine='BLENDER_EEVEE'
sc.render.resolution_x=700; sc.render.resolution_y=850; sc.render.resolution_percentage=100
sc.render.image_settings.file_format='PNG'
sc.world=bpy.data.worlds.new('QA_World'); sc.world.color=(0.18,0.18,0.18)
camdata=bpy.data.cameras.new('QA_Camera'); cam=bpy.data.objects.new('QA_Camera',camdata)
sc.collection.objects.link(cam); sc.camera=cam; camdata.type='ORTHO'; camdata.ortho_scale=2.15
for name,loc,power,size in [('Key',(-2,-3,4),450,3),('Fill',(3,-2,2),300,3),('Rim',(0,3,3),450,2)]:
    d=bpy.data.lights.new(name,'AREA');d.energy=power;d.shape='DISK';d.size=size
    o=bpy.data.objects.new(name,d);sc.collection.objects.link(o);o.location=loc
    o.rotation_euler=(Vector((0,0,1))-o.location).to_track_quat('-Z','Y').to_euler()
def render(name,loc,target,wire=False):
    cam.location=loc;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler()
    sc.render.filepath=str(REVIEW/(name+'.png'))
    mod=None
    if wire:
        wiremat=bpy.data.materials.new('QA_Wire');wiremat.diffuse_color=(0.015,0.02,0.025,1)
        mesh.data.materials.append(wiremat)
        mod=mesh.modifiers.new('QA_Wire','WIREFRAME');mod.thickness=0.0005;mod.use_replace=False;mod.material_offset=1
    bpy.ops.render.render(write_still=True)
    if mod: mesh.modifiers.remove(mod); mesh.data.materials.pop(index=1)
render('blender-seated-front',(0,-4,1.3),(0,-0.1,0.95))
render('blender-seated-side',(4,0,1.3),(0,-0.1,0.95))
render('blender-seated-wire',(3,-4,2),(0,-0.1,0.95),True)
for p in rig.pose.bones: p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update()
render('blender-rest-front',(0,-4,1),(0,0,0.9))
render('blender-rest-back',(0,4,1),(0,0,0.9))

# Round-trip the deliverable independently and compare evaluated world bounds.
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=str(OUT/'CHR_Juge.fbx'))
bpy.context.view_layer.update()
report['roundtrip']=[{'name':o.name,'type':o.type,'scale':list(o.scale),'dimensions_m':list(o.dimensions)} for o in bpy.context.scene.objects]
(REVIEW/'blender-qa.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print('QA_SUMMARY '+json.dumps(report))
