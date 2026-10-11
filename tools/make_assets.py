"""Builds the plugin's image assets from Resources/original.png.

Outputs (all in Resources/):
  background.png  - the artwork with the painted knob indicator removed
  glow_dim.png    - dark overlay for crown + $$$ (used at "Cheap")
  glow_crisp.png  - brightened crown + $$$ (fades in as the knob turns)
  glow_halo.png   - soft blurred bloom around crown + $$$
The glow layers are cropped to GLOW_BOX; the editor draws them at that offset.
"""
import math
import os

from PIL import Image, ImageChops, ImageEnhance, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
RES = os.path.join(HERE, "..", "Resources")

KNOB_CENTRE = (621.0, 634.0)
INDICATOR = ((699, 591), (747, 561))      # painted indicator segment
GLOW_BOX = (440, 0, 820, 345)             # region holding crown + $$$
CROWN_BOX = (578, 44, 680, 146)
DOLLAR_BOX = (482, 196, 774, 311)


def dist_to_segment(p, a, b):
    ax, ay = a
    bx, by = b
    px, py = p
    dx, dy = bx - ax, by - ay
    t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy)))
    return math.hypot(px - (ax + t * dx), py - (ay + t * dy))


def rotate_sample(img, x, y, degrees):
    cx, cy = KNOB_CENTRE
    a = math.radians(degrees)
    rx = cx + (x - cx) * math.cos(a) - (y - cy) * math.sin(a)
    ry = cy + (x - cx) * math.sin(a) + (y - cy) * math.cos(a)
    return img.getpixel((int(round(rx)), int(round(ry))))


def remove_indicator(img):
    out = img.copy()
    px = out.load()
    (x0, y0), (x1, y1) = INDICATOR
    for y in range(min(y0, y1) - 14, max(y0, y1) + 15):
        for x in range(min(x0, x1) - 14, max(x0, x1) + 15):
            d = dist_to_segment((x, y), *INDICATOR)
            if d > 11:
                continue
            # The face is concentrically brushed, so pixels at the same radius
            # a few degrees either side are a near-perfect replacement.
            a = rotate_sample(img, x, y, -14)
            b = rotate_sample(img, x, y, 14)
            fill = tuple((a[i] + b[i]) // 2 for i in range(3))
            w = 1.0 if d < 8 else (11 - d) / 3.0  # feather the edge
            o = px[x, y]
            px[x, y] = tuple(int(o[i] * (1 - w) + fill[i] * w) for i in range(3))
    return out


def gold_mask(img):
    """Alpha mask of the gold crown and dollar signs inside GLOW_BOX."""
    crop = img.crop(GLOW_BOX)
    mask = Image.new("L", crop.size, 0)
    src = crop.load()
    m = mask.load()
    gx, gy = GLOW_BOX[0], GLOW_BOX[1]
    for y in range(crop.height):
        for x in range(crop.width):
            ox, oy = x + gx, y + gy
            inside = any(b[0] <= ox < b[2] and b[1] <= oy < b[3] for b in (CROWN_BOX, DOLLAR_BOX))
            if not inside:
                continue
            r, g, b = src[x, y]
            lum = 0.3 * r + 0.59 * g + 0.11 * b
            if r - b > 25 and lum > 45:
                m[x, y] = int(min(255, (lum - 45) * 4))
    return crop, mask.filter(ImageFilter.GaussianBlur(0.6))


def main():
    original = Image.open(os.path.join(RES, "original.png")).convert("RGB")
    background = remove_indicator(original)
    background.save(os.path.join(RES, "background.png"), optimize=True)

    crop, mask = gold_mask(original)

    # Dim layer: darkens the gold so "Cheap" looks tarnished.
    dim = Image.new("RGBA", crop.size, (12, 8, 2, 0))
    dim.putalpha(mask)
    dim.save(os.path.join(RES, "glow_dim.png"), optimize=True)

    # Crisp layer: the gold itself, brighter, warmer and more saturated.
    bright = ImageEnhance.Brightness(crop).enhance(1.45)
    bright = ImageEnhance.Color(bright).enhance(1.25)
    bright = ImageChops.add(bright, Image.new("RGB", crop.size, (30, 20, 0)))
    crisp = bright.convert("RGBA")
    crisp.putalpha(mask)
    crisp.save(os.path.join(RES, "glow_crisp.png"), optimize=True)

    # Halo layer: wide + tight blurred blooms in warm gold.
    wide = mask.filter(ImageFilter.GaussianBlur(22)).point(lambda v: min(255, int(v * 1.6)))
    mid = mask.filter(ImageFilter.GaussianBlur(9)).point(lambda v: min(255, int(v * 1.3)))
    tight = mask.filter(ImageFilter.GaussianBlur(3))
    alpha = ImageChops.lighter(ImageChops.lighter(wide, mid), tight)
    halo = Image.new("RGBA", crop.size, (255, 190, 60, 0))
    halo.putalpha(alpha)
    halo.save(os.path.join(RES, "glow_halo.png"), optimize=True)
    print("assets written to", os.path.abspath(RES))


if __name__ == "__main__":
    main()
