
from PIL import Image, ImageDraw, ImageFont
import PIL
import sys
import os

for file in sys.argv:
    if os.path.isfile(file) and file.endswith(".png"):
        img = Image.open(file)
        print(file)
        img = img.convert(mode="RGB")
        base = os.path.splitext(file)[0]
        img.save(base + ".ppm")