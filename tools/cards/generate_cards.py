"""Generate Dubito's opaque card textures and temporary review sheets.

Run from any directory: python tools/cards/generate_cards.py
Requires Pillow 12.x and the versioned Liberation Sans font (no system fonts).
Every run renders twice and checks exact PNG bytes before writing output.
"""

from __future__ import annotations

import hashlib
from io import BytesIO
import math
import os
from pathlib import Path
import tempfile

from PIL import Image, ImageChops, ImageDraw, ImageFont


# All layout coordinates and font sizes below are in final texture pixels.
WIDTH, HEIGHT = 504, 704
SUPERSAMPLE = 4
SAFE_MARGIN = 24
CREAM = "#F2E8D5"
FACE_BG = "#FAF6EC"
RED = "#C0182A"
BLACK = "#151515"
BURGUNDY = "#6E1423"
BACK_ACCENT = "#973448"
CORNER_X, CORNER_Y = 32, 32
CORNER_WIDTH = 112
CORNER_FONT_SIZE = 88
CENTER_FONT_SIZE = 264
CORNER_SUIT_SIZE = 48
CENTER_SUIT_SIZE = 140
ROOT = Path(__file__).resolve().parents[2]
FONT_PATH = Path(__file__).resolve().parent / "fonts/LiberationSans-Bold.ttf"
OUTPUT = ROOT / "unity/Assets/Dubito/Art/Cards/Faces"
REVIEW = Path(os.environ.get("TEMP", tempfile.gettempdir())) / "dubito-review/cards"
RANKS = (
    ("Ace", "A"), ("Two", "2"), ("Three", "3"), ("Four", "4"),
    ("Five", "5"), ("Six", "6"), ("Seven", "7"), ("Eight", "8"),
    ("Nine", "9"), ("Ten", "10"), ("Jack", "V"), ("Queen", "D"), ("King", "R"),
)
SUITS = ("Clubs", "Diamonds", "Hearts", "Spades")


def font(size: int, scale: int = SUPERSAMPLE) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(FONT_PATH), size * scale)


def centered_text(draw, center, text, face, color):
    """Center the visible ink, including the font's bearing offsets."""
    left, top, right, bottom = draw.textbbox((0, 0), text, font=face)
    x, y = center
    draw.text((round(x - (left + right) / 2), round(y - (top + bottom) / 2)),
              text, font=face, fill=color)


def suit(draw, name, box, color):
    """Vector shapes only; the four suits never depend on font glyph coverage."""
    x, y, w, h = box

    def points(coords):
        return [(round(x + a * w), round(y + b * h)) for a, b in coords]

    def polygon(coords):
        draw.polygon(points(coords), fill=color)

    def ellipse(coords):
        p = points(((coords[0], coords[1]), (coords[2], coords[3])))
        draw.ellipse((*p[0], *p[1]), fill=color)

    def heart(invert=False):
        # Sample a closed cubic outline, then rasterize the resulting polygon.
        segments = (
            ((.5, .19), (.19, -.18), (-.23, .29), (.16, .62)),
            ((.16, .62), (.27, .73), (.42, .89), (.5, 1)),
            ((.5, 1), (.58, .89), (.73, .73), (.84, .62)),
            ((.84, .62), (1.23, .29), (.81, -.18), (.5, .19)),
        )
        outline = []
        for a, b, c, d in segments:
            for i in range(33):
                t = i / 32
                u = 1 - t
                px = u**3*a[0] + 3*u*u*t*b[0] + 3*u*t*t*c[0] + t**3*d[0]
                py = u**3*a[1] + 3*u*u*t*b[1] + 3*u*t*t*c[1] + t**3*d[1]
                outline.append((px, (1 - py) * .82 if invert else py))
        polygon(outline)

    if name == "Diamonds":
        polygon(((.5, 0), (.96, .5), (.5, 1), (.04, .5)))
    elif name == "Hearts":
        heart()
    elif name == "Spades":
        heart(invert=True)
        polygon(((.46, .60), (.54, .60), (.59, .88), (.72, 1), (.28, 1), (.41, .88)))
    elif name == "Clubs":
        ellipse((.27, 0, .73, .47))
        ellipse((.02, .32, .51, .81))
        ellipse((.49, .32, .98, .81))
        polygon(((.37, .42), (.63, .42), (.55, .73), (.60, .9), (.73, 1),
                 (.27, 1), (.40, .9), (.45, .73)))
    else:
        raise ValueError(name)


def downsample(image):
    return image.resize((WIDTH, HEIGHT), Image.Resampling.LANCZOS)


def make_face(value, name):
    s = SUPERSAMPLE
    image = Image.new("RGB", (WIDTH*s, HEIGHT*s), FACE_BG)
    ink = Image.new("RGB", image.size, FACE_BG)
    draw = ImageDraw.Draw(ink)
    color = RED if name in ("Diamonds", "Hearts") else BLACK
    centered_text(draw, ((CORNER_X + CORNER_WIDTH/2)*s, (CORNER_Y + 32)*s),
                  value, font(CORNER_FONT_SIZE), color)
    suit(draw, name, ((CORNER_X + (CORNER_WIDTH-CORNER_SUIT_SIZE)/2)*s,
                     113*s, CORNER_SUIT_SIZE*s, CORNER_SUIT_SIZE*s), color)
    # Rotate the entire corner layer: exact placement and matching typography.
    image.paste(ink)
    flipped = ink.transpose(Image.Transpose.ROTATE_180)
    mask = ImageChops.difference(flipped, Image.new("RGB", image.size, FACE_BG)).convert("L")
    # Use a binary mask so color is not multiplied by antialiasing twice.
    image.paste(flipped, (0, 0), mask.point(lambda p: 255 if p else 0))
    draw = ImageDraw.Draw(image)
    centered_text(draw, (WIDTH*s/2, 284*s), value, font(CENTER_FONT_SIZE), color)
    suit(draw, name, ((WIDTH-CENTER_SUIT_SIZE)*s/2, 410*s,
                     CENTER_SUIT_SIZE*s, CENTER_SUIT_SIZE*s), color)
    result = downsample(image)
    # Including the Lanczos fringe, every piece of ink stays inside the safe area.
    bbox = ImageChops.difference(result, Image.new("RGB", result.size, FACE_BG)).getbbox()
    if not bbox or min(bbox[0], bbox[1], WIDTH-bbox[2], HEIGHT-bbox[3]) < SAFE_MARGIN:
        raise RuntimeError(f"Unsafe face content bounds: {value} {name}: {bbox}")
    return result


def make_back():
    s = SUPERSAMPLE
    image = Image.new("RGB", (WIDTH*s, HEIGHT*s), BURGUNDY)
    draw = ImageDraw.Draw(image)
    for inset, thickness in ((30, 5), (42, 2)):
        draw.rectangle((inset*s, inset*s, (WIDTH-inset)*s, (HEIGHT-inset)*s),
                       outline=CREAM, width=thickness*s)
    # Original staggered lattice of open diamonds and square knots.
    for row, cy in enumerate(range(88, HEIGHT-65, 48)):
        for cx in range(84 + (row % 2)*24, WIDTH-60, 48):
            coords = [(cx*s, (cy-17)*s), ((cx+17)*s, cy*s),
                      (cx*s, (cy+17)*s), ((cx-17)*s, cy*s), (cx*s, (cy-17)*s)]
            draw.line(coords, fill=BACK_ACCENT, width=3*s)
            draw.rectangle(((cx-3)*s, (cy-3)*s, (cx+3)*s, (cy+3)*s), fill=CREAM)
    image = downsample(image)
    # Mirror the top pixel rows after filtering. This construction guarantees
    # byte-exact 180-degree symmetry, including borders and antialiasing.
    top = image.crop((0, 0, WIDTH, HEIGHT//2))
    image.paste(top.transpose(Image.Transpose.ROTATE_180), (0, HEIGHT//2))
    if image.tobytes() != image.transpose(Image.Transpose.ROTATE_180).tobytes():
        raise RuntimeError("Back is not pixel-exact rotationally symmetric")
    return image


def png_bytes(image):
    buffer = BytesIO()
    image.save(buffer, format="PNG", optimize=False, compress_level=9)
    return buffer.getvalue()


def generate():
    result = {f"{rank}_{name}.png": png_bytes(make_face(value, name))
              for name in SUITS for rank, value in RANKS}
    result["Back.png"] = png_bytes(make_back())
    return result


def contact(images, path, card_width, labels):
    columns = 13
    card_height = round(HEIGHT*card_width/WIDTH)
    gap, pad = 12, 18
    label_height = 24 if labels else 0
    cell_width, cell_height = card_width+gap, card_height+gap+label_height
    sheet = Image.new("RGB", (pad*2+columns*cell_width-gap,
                              pad*2+math.ceil(len(images)/columns)*cell_height-gap), "#D1CDC5")
    draw = ImageDraw.Draw(sheet)
    for index, (name, image) in enumerate(images.items()):
        x = pad+(index % columns)*cell_width
        y = pad+(index // columns)*cell_height
        sheet.paste(image.resize((card_width, card_height), Image.Resampling.LANCZOS), (x, y))
        if labels:
            draw.text((x, y+card_height+3), name.removesuffix(".png"), font=font(11, 1), fill=BLACK)
    sheet.save(path)


def fan(images, path):
    # Five overlapping cards, staggered down as in a hand. Crop to the upper
    # exposed indices to inspect the worst case where central values are hidden.
    names = ("Ace_Spades.png", "Ten_Clubs.png", "Jack_Diamonds.png",
             "Queen_Hearts.png", "King_Spades.png")
    canvas = Image.new("RGB", (1096, 824), "#343C39")
    for index, name in enumerate(names):
        x, y = 24 + 136*index, 24 + 6*index
        canvas.paste(images[name], (x, y))
    canvas.crop((0, 0, 1096, 220)).save(path)


def main():
    if not FONT_PATH.is_file():
        raise FileNotFoundError(f"Required versioned font missing: {FONT_PATH}")
    first, second = generate(), generate()
    expected = {f"{rank}_{name}.png" for rank, _ in RANKS for name in SUITS} | {"Back.png"}
    if len(first) != 53 or set(first) != expected:
        raise RuntimeError("Expected exactly the 53 enum-aligned filenames")
    if first != second:
        raise RuntimeError("Second generation differs from first (PNG bytes)")
    if len({hashlib.sha256(data).digest() for data in first.values()}) != 53:
        raise RuntimeError("Duplicate PNG hashes")
    images = {name: Image.open(BytesIO(data)).copy() for name, data in first.items()}
    if any(im.size != (WIDTH, HEIGHT) or im.mode != "RGB" for im in images.values()):
        raise RuntimeError("All textures must be 504 x 704 RGB (opaque)")
    back = images["Back.png"]
    if back.tobytes() != back.transpose(Image.Transpose.ROTATE_180).tobytes():
        raise RuntimeError("Encoded back symmetry check failed")
    OUTPUT.mkdir(parents=True, exist_ok=True)
    unexpected = {p.name for p in OUTPUT.glob("*.png")} - expected
    if unexpected:
        raise RuntimeError(f"Unexpected PNGs in output directory (left untouched): {unexpected}")
    for name, data in first.items():
        (OUTPUT/name).write_bytes(data)
    if {p.name for p in OUTPUT.glob("*.png")} != expected:
        raise RuntimeError("Output filenames differ from expected")
    if any((OUTPUT/name).read_bytes() != data for name, data in first.items()):
        raise RuntimeError("Written PNG bytes differ from generated bytes")
    REVIEW.mkdir(parents=True, exist_ok=True)
    contact(images, REVIEW/"contact.png", 126, True)
    contact(images, REVIEW/"contact_64px.png", 64, False)
    fan(images, REVIEW/"fan.png")
    print(f"PASS: 53 exact filenames; 53 distinct SHA-256 hashes; 504x704 opaque RGB.")
    print("PASS: two generations have identical PNG bytes; back equals its 180-degree rotation.")
    print(f"PASS: face ink stays >= {SAFE_MARGIN}px from edges; written files verified.")
    print(f"Textures: {OUTPUT}")
    print(f"Review sheets: {REVIEW}")


if __name__ == "__main__":
    main()
