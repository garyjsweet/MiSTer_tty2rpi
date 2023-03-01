
from PIL import Image, ImageDraw, ImageFont
import PIL
import sys
import os

for file in os.listdir('.'):
    if os.path.isfile(file) and (file.endswith(".png") or file.endswith(".jpg")):
        img = Image.open(file)
        aspect = img.width / img.height
        print(file)
        img = img.convert(mode="RGB")
        if img.width > 1920 or img.height > 480:
            if (aspect >= 1920 / 480):
                img = img.resize((1920, int(1920 / aspect)), PIL.Image.BILINEAR)
            else:
                img = img.resize((int(480 * aspect), 480), PIL.Image.BILINEAR)
        base = os.path.splitext(file)[0]
        img.save(base + ".png")