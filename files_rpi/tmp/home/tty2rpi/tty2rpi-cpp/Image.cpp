/*
 * The MIT License (MIT)

 * Copyright (c) 2023 Gary Sweet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "Image.h"

static uint32_t ReadUInt(FILE *fp)
{
    char buf[64] {};
    char c;
    do
    {
        c = getc(fp);
    }
    while (c == ' ' || c == '\t' || c == '\n');
    ungetc(c, fp);

    uint32_t i = 0;
    do
    {
        c = getc(fp);
        if (isdigit(c))
            buf[i++] = c;
    }
    while (isdigit(c));
    ungetc(c, fp);

    return atoi(buf);
}

Image::Image(uint32_t w, uint32_t h, FontManager *fm) : m_w(w), m_h(h), m_fontMan(fm)
{
    m_data.resize(w * h * BPP);
    memset(m_data.data(), 0, m_data.size());
}

Image::Image(uint32_t w, uint32_t h, uint8_t *data) :
    m_w(w), m_h(h)
{
    m_data.resize(w * h * BPP);
    memcpy(m_data.data(), data, m_data.size());
}

Image::Image(const std::string &ppmFile)
{
    FILE *fp = nullptr;
    try
    {
        fp = fopen(ppmFile.c_str(), "rb");
        if (!fp)
            throw("Couldn't open ppm file");

        uint32_t colsize;
        if (getc(fp) != 'P')
            throw "PPM header scan failed";
        if (getc(fp) != '6')
            throw "PPM header scan failed";

        m_w = ReadUInt(fp);
        m_h = ReadUInt(fp);
        colsize = ReadUInt(fp);

        if (getc(fp) != '\n')
            throw "PPM header scan failed";

        size_t b = m_w * m_h * BPP;
        m_data.resize(b);

        fread(m_data.data(), 1, b, fp);
    }
    catch(const char *s)
    {
        if (fp)
            fclose(fp);
        throw s;
    }

    if (fp)
        fclose(fp);
}

void Image::DrawRect(const Rect &rect, const Colour &col)
{
    Rect r = GetRect().Intersect(rect);

    if (col.a == 0)
        return;

    uint8_t *d = PixPtr(r.x, r.y);

    for (uint32_t y = 0; y < r.h; y++)
    {
        for (uint32_t x = 0; x < r.w; x++)
            Blend(d + x * BPP, col, 255);
        d += m_w * BPP;
    }
}

void Image::CopyInto(Image *dstImage, const Rect &dstRect) const
{
    Rect fit = GetRect().FitToContainer(dstRect, "c", "m", /*keepSize=*/true);

    for (uint32_t y = 0; y < fit.h; y++)
        memcpy(dstImage->PixPtr(fit.x, fit.y + y), PixPtr(0, y), m_w * BPP);
}

void Image::DrawText(const Text &dtext)
{
    const Colour &col = dtext.colour;

    m_fontMan->RestoreBaseSize();
    Rect bbox = m_fontMan->TextBound(dtext.text);
    Rect fit  = bbox.FitToContainer(dtext.rect, dtext.horz, dtext.vert, /*keepSize=*/false);

    float scale;
    if (bbox.Aspect() > fit.Aspect())
        scale = (float)fit.w / bbox.w;
    else
        scale = (float)fit.h / bbox.h;

    m_fontMan->NewSize(std::min(MAX_FONT_SIZE, (uint32_t)(scale * BASE_FONT_SIZE)));
    bbox = m_fontMan->TextBound(dtext.text);
    fit = bbox.FitToContainer(dtext.rect, dtext.horz, dtext.vert, /*keepSize=*/true);

    // Draw background rect
    if (dtext.bgCol.a != 0)
        DrawRect(fit.Enlarge(15), dtext.bgCol);

    // Pen is in 26.6 fixed-point coordinates
    FT_Vector pen;
    pen.x = fit.x * 64;
    pen.y = (m_h - fit.y) * 64;

    FT_GlyphSlot slot = m_fontMan->Slot();

    m_fontMan->PrepChar(pen, 'X', /*render=*/false);
    FT_Int charHeight = slot->bitmap.rows;
    FT_Int lineHeight = (charHeight + LINE_SPACING) * 64;

    for (size_t n = 0; n < dtext.text.size(); n++)
    {
        char c = dtext.text.at(n);

        if (c == '\n')
        {
            pen.y -= lineHeight;
            pen.x = fit.x * 64;
            continue;
        }

        if (!m_fontMan->PrepChar(pen, c, /*render=*/true))
            continue;

        FT_Bitmap bitmap = slot->bitmap;

        FT_Int  x = slot->bitmap_left;
        FT_Int  y = m_h - slot->bitmap_top + charHeight;
        FT_Int  i, j, p, q;
        FT_Int  x_max = x + bitmap.width;
        FT_Int  y_max = y + bitmap.rows;

        for (j = y, q = 0; j < y_max; j++, q++)
        {
            for (i = x, p = 0; i < x_max; i++, p++)
            {
                if (i < 0 || j < 0 || i >= m_w || j >= m_h)
                    continue;

                uint8_t greyVal = bitmap.buffer[q * bitmap.width + p];
                Blend(PixPtr(i, j), col, greyVal);
            }
        }

        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }
}

void Image::RBSwap()
{
    uint8_t *p = PixPtr(0, 0);

    for (uint32_t y = 0; y < m_h; y++)
    {
        for (uint32_t x = 0; x < m_w; x++)
        {
            std::swap(p[0], p[2]);
            p += BPP;
        }
    }
}

void Image::Clear()
{
    memset(m_data.data(), 0, m_data.size());
}
