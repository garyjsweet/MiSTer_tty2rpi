import PIL
from PIL import Image, ImageDraw, ImageFont
from rectangle import Rectangle
from text import Text

class BuilderImg:
    def __init__(self, r, img):
        self.rect = r
        self.img = img

class ImageBuilder:
    def __init__(self, w, h):
        self._images = []
        self._texts  = []
        self._w = w
        self._h = h

    def clear(self):
        self._images = []
        self._texts = []

    def clear_texts(self):
        self._texts = []

    def add_layout_image(self, rect, img):
        self._images.append(BuilderImg(rect, img))

    def add_text(self, rect, text, fill, lcr, tmb, bgCol = (0, 0, 0, 0)):
        self._texts.append(Text(rect, text, fill, lcr, tmb, bgCol))

    def get_result(self, filename):
        with open(filename, 'w') as fp:
            for img in self._images:
                fp.write("`I:" + str(img.rect) + " F:" + str(img.img) + "\n")

            for txt in self._texts:
                fp.write(str(txt) + "\n")
