import sys

from PIL import Image, ImageDraw, ImageFont, ImageChops, ImageFilter
import imgtools

font = ImageFont.truetype("/home/jamesb/.fonts/IBMPlexSans-Medium.otf", 18)

def extract(im):
    im = im.copy()
    (w, h) = im.size
    a = im
    blank = Image.new("L", im.size, 0)
    ap = im.load()
    i = 0
    subs = []
    while True:
        b = a.tobytes()
        if not 255 in b:
            break
        ii = b.index(255)
        (x, y) = (ii % w, ii // w)
        ImageDraw.floodfill(a, (x, y), 128)
        matte = a.point(lambda x: [0,255][x == 128])
        subs.append(matte)
        # pop = sum(matte.tobytes()) // 255
        # print(i, pop, (x,y))
        ImageDraw.floodfill(a, (x, y), 0)
        i += 1
    return subs

if __name__ == "__main__":
    def shrink(im):
        return im
        return im.resize((1280, 960))
    hicon = shrink(Image.open("face-full-hicon.png")).convert("1").convert("L")
    groups = shrink(Image.open("face-full-hicon-groups.png")).split()[3].point(lambda x: 255 * (x != 0)).convert("1").convert("L")
    els = extract(hicon)
    grp = extract(groups)

    # Merge elements that are in the same group
    gr = [Image.new("L", hicon.size, 0) for _ in grp]

    lit = []
    for el in els:
        if ImageChops.multiply(el, groups).getbbox():
            print('hit', el)
            for i,g in enumerate(grp):
                if ImageChops.multiply(el, g).getbbox():
                    gr[i] = ImageChops.add(gr[i], el)
        else:
            lit.append(el)
    lit += gr

    for i,im in enumerate(lit):
        im.resize((640,480), Image.BILINEAR).save(f"seg-{i:02}.png")

    final = ImageChops.multiply(hicon.point(lambda x: 255 - x).convert("RGB"), Image.new("RGB", hicon.size, (32, 32, 40)))

    dr = ImageDraw.Draw(final)
    for i,im in enumerate(lit):
        (x0, y0, x1, y1) = im.getbbox()
        (x, y) = ((x0 + x1) // 2, (y0 + y1) // 2)
        s = str(i)
        (w, h) = dr.textsize(s, font=font)
        dr.text((x - w // 2, y - h // 2), s, font=font, fill=(255, 255, 255))
    for im in gr:
        dr.rectangle(im.filter(ImageFilter.MaxFilter(7)).getbbox(), fill=None, outline=(40, 40, 40))

    final.save("out.png")
    # imgtools.view([final])

    print(i)
