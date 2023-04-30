
from PIL import Image, ImageDraw, ImageFont
import PIL
import sys
import os

W = 1920
H = 400

for file in os.listdir('.'):
    if os.path.isfile(file) and (file.endswith(".png") or file.endswith(".jpg")):
        img = Image.open(file)
        aspect = img.width / img.height
        print(file)
        img = img.convert(mode="RGB")
        if img.width > W or img.height > H:
            if (aspect >= W / H):
                img = img.resize((W, int(W / aspect)), PIL.Image.BILINEAR)
            else:
                img = img.resize((int(H * aspect), H), PIL.Image.BILINEAR)
        base = os.path.splitext(file)[0]
        img.save(base + ".ppm")