from PIL import Image, ImageDraw, ImageFont
from log import log

class Rectangle:
    def __init__(self, x, y, w, h):
        self.x = x
        self.y = y
        self.w = w
        self.h = h

    def fromImg(self, img):
        self.x = 0
        self.y = 0
        self.w = img.width
        self.h = img.height

    def __str__(self):
        return  "({0}, {1}, {2}, {3})".format(self.x, self.y, self.w, self.h)

    def fit_in_container(self, container):
        fit = Rectangle(0, 0, 0, 0)
        aspect = self.w / self.h
        container_aspect = container.w / container.h
        if aspect > container_aspect: # Fit width
            fit.x = container.x
            fit.w = container.w
            fit.h = fit.w / aspect
            vertical_space = container.h - fit.h
            fit.y = container.y + (vertical_space / 2)
        else:                         # Fit height
            fit.y = container.y
            fit.h = container.h
            fit.w = fit.h * aspect
            horizontal_space = container.w - fit.w
            fit.x = container.x + (horizontal_space / 2)
        return fit

    def calc_text_fit_font(self, text, font, basic_size, max_size, anchor, align):
        img = Image.new("RGB", (1, 1))
        draw = ImageDraw.Draw(img)
        f = font.font_variant(size=basic_size)
        (l,t,r,b) = draw.multiline_textbbox((0, 0), text, font=f, anchor=anchor, align=align)
        w = abs(r - l)
        h = abs(t - b)
        scale = 1
        if h != 0 and self.h != 0:
            if w / h > self.w / self.h:
                scale = self.w / w
            else:
                scale = self.h / h
        size = min(max_size, int(basic_size * scale))
        log("Font size = {}".format(size))
        return font.font_variant(size=size)
