from PIL import ImageDraw, ImageFont

BASIC_FONT_SIZE = 20
MAX_FONT_SIZE   = 70
LINE_SPACING    = 15
font = ImageFont.truetype('/usr/share/fonts/truetype/freefont/FreeSansBoldOblique', BASIC_FONT_SIZE)

class Text:
    def __init__(self, rect, origin, text, fill, font, align, anchor, bgCol = None):
        self._rect   = rect
        self._origin = origin
        self._text   = text
        self._fill   = fill
        self._font   = font
        self._align  = align
        self._anchor = anchor
        self._bgCol  = bgCol

    def draw(self, img):
        draw = ImageDraw.Draw(img, 'RGBA')
        bord = 10

        scaled_font = self._rect.calc_text_fit_font(
                        self._text, self._font, BASIC_FONT_SIZE, MAX_FONT_SIZE,
                        anchor=self._anchor, align=self._align)

        if self._bgCol != None:
            (l,t,r,b) = draw.multiline_textbbox(self._origin, self._text, font=self._font,
                                                align=self._align, anchor=self._anchor,
                                                spacing=LINE_SPACING)
            draw.rectangle((l - bord, t - bord, r + bord, b + bord), fill=self._bgCol)

        draw.text(self._origin, self._text, fill=self._fill,
                  font=scaled_font, align=self._align, anchor=self._anchor,
                  spacing=LINE_SPACING)
