"""Planche de comparaison : une ligne par dossier (direction), une colonne par lettre d'image (A, B, C...).

    python planche.py <sortie.png> <dossier1> [<dossier2> ...] [--title "DA.1"] [--cell 512]

Dans chaque dossier, prend les images nommées comme dans art/style/PROMPTS.md (A-table.png, B-personnage.png,
A-table-2.png...), sans descendre dans _gen/. Une case vide si l'image manque.
"""
import argparse
import re
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

EXT = {".png", ".jpg", ".jpeg", ".webp"}


def font(size):
    for name in ("arial.ttf", "DejaVuSans.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def images(folder: Path):
    files = [p for p in folder.iterdir() if p.is_file() and p.suffix.lower() in EXT and re.match(r"^[A-Z]-", p.name)]
    return sorted(files, key=lambda p: p.name)


def fit(img: Image.Image, cell: int) -> Image.Image:
    img = img.convert("RGB")
    img.thumbnail((cell, cell), Image.LANCZOS)
    canvas = Image.new("RGB", (cell, cell), (28, 28, 28))
    canvas.paste(img, ((cell - img.width) // 2, (cell - img.height) // 2))
    return canvas


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("folders", nargs="+")
    ap.add_argument("--title", default="")
    ap.add_argument("--cell", type=int, default=512)
    a = ap.parse_args()

    rows = [(Path(f), images(Path(f))) for f in a.folders]
    cols = sorted({p.stem for _, ims in rows for p in ims})
    if not cols:
        sys.exit("Aucune image A-..., B-... dans ces dossiers.")
    cell, pad, label_w, head = a.cell, 16, 260, 70 if a.title else 40
    W = label_w + len(cols) * (cell + pad) + pad
    H = head + len(rows) * (cell + pad + 30) + pad
    sheet = Image.new("RGB", (W, H), (12, 12, 12))
    d = ImageDraw.Draw(sheet)
    if a.title:
        d.text((pad, 14), a.title, fill=(240, 240, 240), font=font(32))
    for j, c in enumerate(cols):
        d.text((label_w + j * (cell + pad), head - 30), c, fill=(200, 200, 200), font=font(20))
    for i, (folder, ims) in enumerate(rows):
        y = head + i * (cell + pad + 30)
        d.text((pad, y + cell // 2 - 12), folder.name, fill=(240, 240, 240), font=font(24))
        by_stem = {p.stem: p for p in ims}
        for j, c in enumerate(cols):
            x = label_w + j * (cell + pad)
            if c in by_stem:
                sheet.paste(fit(Image.open(by_stem[c]), cell), (x, y))
            else:
                d.rectangle([x, y, x + cell, y + cell], outline=(70, 70, 70))
                d.text((x + 12, y + 12), "absente", fill=(120, 120, 120), font=font(18))
    Path(a.out).parent.mkdir(parents=True, exist_ok=True)
    sheet.save(a.out)
    print(f"{a.out} : {len(rows)} lignes x {len(cols)} colonnes")


if __name__ == "__main__":
    main()
