from PIL import ImageDraw, ImageFont

class Text:
    def __init__(self, rect, text, fill, lcr, tmb, bgCol = (0, 0, 0, 0)):
        self._rect   = rect
        self._text   = text
        self._fill   = fill
        self._lcr    = lcr
        self._tmb    = tmb
        self._bgCol  = bgCol

    def __str__(self):
        return  "`T:{} C:{} H:{} V:{} B:{} S:{}".format(str(self._rect),
                    str(self._fill), str(self._lcr), str(self._tmb),
                    str(self._bgCol), self._text)
