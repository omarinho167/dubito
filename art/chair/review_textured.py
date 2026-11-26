"""After bake_chair.py, run with system Python (Pillow) for reference sheets."""
from pathlib import Path
import os, json
from PIL import Image, ImageDraw, ImageOps
ROOT=Path(__file__).resolve().parents[2]
OUT=Path(os.environ['TEMP'])/'dubito-review/chair'
refs=ROOT/'art/chair/ref'
front=refs/'Wooden_dining_chair_product_photo_35b59d42_2K.jpg'
side=refs/'Side_profile_of_chair_49429f02_2K.jpg'
for view,ref,label in [('Front',front,'Reference face'),('ThreeQuarter',side,'Reference profil (angle different)')]:
    sheet=Image.new('RGB',(1800,1260),'#eeeeee'); draw=ImageDraw.Draw(sheet)
    for x,path,title in [(0,ref,label),(900,OUT/f'textured_{view}.png','Rendu texture - '+view)]:
        im=ImageOps.contain(Image.open(path).convert('RGB'),(900,1200))
        sheet.paste(im,(x+(900-im.width)//2,60+(1200-im.height)//2)); draw.text((x+30,22),title,fill='#202020',font_size=24)
    sheet.save(OUT/f'textured_comparison_{view}.png')
report=json.loads((OUT/'textured_report.json').read_text())
print(json.dumps({k:report[k] for k in ('device','backend','bake_seconds','triangles','geometry_unchanged','uv_audit')},indent=2))
print('Reference sheets:',OUT)
