"""Headless supplementary wire, piece and UV checker review; never saves the blend."""
import bpy, sys, os, json
import numpy as np
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=Path(os.environ['TEMP'])/'dubito-review/chair'
bpy.ops.wm.open_mainfile(filepath=str(ROOT/'art/chair/chair.blend'))
sc=bpy.data.scenes['CHAIR']; bpy.context.window.scene=sc
ob=bpy.data.objects['SM_Chair']; me=ob.data
image=next(n.image for n in me.materials[0].node_tree.nodes if n.type=='TEX_IMAGE')
assert os.path.exists(bpy.path.abspath(image.filepath))
pixels=np.empty(2048*2048*4,dtype=np.float32); image.pixels.foreach_get(pixels); pixels=pixels.reshape((2048,2048,4))
rgb=np.zeros(3); area=0
for face in me.polygons:
    uv=sum((me.uv_layers.active.data[i].uv for i in face.loop_indices),start=__import__('mathutils').Vector((0,0)))/3
    rgb+=pixels[min(2047,int(uv.y*2048)),min(2047,int(uv.x*2048)),:3]*face.area; area+=face.area
rgb/=area
# Saved generated bake buffer is already encoded; do not apply a second sRGB transform.
report=json.loads((OUT/'textured_report.json').read_text()); report['area_weighted_srgb_approx']=[int(round(v*255)) for v in rgb]
(OUT/'textured_report.json').write_text(json.dumps(report,indent=2))
print('AVERAGE_SRGB',report['area_weighted_srgb_approx'],flush=True)
sys.path.insert(0,str(ROOT/'.agents/skills/blender-asset-qa/scripts'))
import review_shots
for mode in ('wire','random'):
    review_shots.run(objects=['SM_Chair'],out=str(OUT/f'textured_qa_{mode}.png'),views=['front','right','back','three_quarter'],mode=mode,res=480)
sc=bpy.data.scenes['CHAIR']; bpy.context.window.scene=sc
ob=bpy.data.objects['SM_Chair']; nt=ob.data.materials[0].node_tree; nt.nodes.clear()
uv=nt.nodes.new('ShaderNodeTexCoord'); checker=nt.nodes.new('ShaderNodeTexChecker'); checker.inputs['Scale'].default_value=64
checker.inputs['Color1'].default_value=(.04,.1,.15,1); checker.inputs['Color2'].default_value=(.7,.8,.9,1)
p=nt.nodes.new('ShaderNodeBsdfPrincipled'); out=nt.nodes.new('ShaderNodeOutputMaterial')
nt.links.new(uv.outputs['UV'],checker.inputs['Vector']); nt.links.new(checker.outputs['Color'],p.inputs['Base Color']); nt.links.new(p.outputs['BSDF'],out.inputs['Surface'])
sc.render.engine='BLENDER_EEVEE'; sc.camera=bpy.data.objects['CAM_ThreeQuarter']
sc.render.resolution_x=900; sc.render.resolution_y=1200; sc.render.resolution_percentage=100
sc.render.filepath=str(OUT/'textured_uv_checker.png'); bpy.ops.render.render(write_still=True)
