import PIL
from PIL import Image, ImageDraw, ImageFont
from rectangle import Rectangle
from text import Text

class BuilderImg:
    def __init__(self, r, img):
        self.rect = r
        self.img = img

class ImageBuilder:
    def __init__(self, w, h, color = (0, 0, 0, 255)):
        self._images = []
        self._texts  = []
        self._w = w
        self._h = h
        self._img = Image.new("RGB", (w, h), color)

    def clear(self, color):
        self._img = Image.new("RGB", (self._w, self._h), color)
        self._images = []
        self._texts = []

    def clear_texts(self):
        self._texts = []

    def add_layout_image(self, rect, img):
        self._images.append(BuilderImg(rect, img))

    def add_text(self, rect, origin, text, fill, font, align, anchor, bgCol = None):
        self._texts.append(Text(rect, origin, text, fill, font, align, anchor, bgCol))

    def get_result(self):
        for img in self._images:
            imgR = Rectangle(0, 0, 0, 0)
            imgR.fromImg(img.img)
            fit = imgR.fit_in_container(img.rect)
            i = img.img.resize((int(fit.w), int(fit.h)), PIL.Image.BILINEAR)
            self._img.paste(i, (int(fit.x), int(fit.y)))

        for txt in self._texts:
            txt.draw(self._img)

        return self._img
