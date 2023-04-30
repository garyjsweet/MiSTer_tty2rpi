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

#pragma once

#include "FontManager.h"
#include "Rect.h"
#include "Colour.h"

#include <cstdint>
#include <vector>

class Image
{
public:
    struct Text
    {
        std::string text;
        Rect        rect;
        Colour      colour;
        Colour      bgCol;
        std::string horz; // "l", "c"|"m", "r"
        std::string vert; // "t", "c"|"m", "b"

    };

    Image() = default;
    Image(uint32_t w, uint32_t h, FontManager *fm);
    Image(uint32_t w, uint32_t h, uint8_t *data);
    Image(const std::string &ppmFile);

    uint32_t Width() const  { return m_w; }
    uint32_t Height() const { return m_h; }
    uint32_t BytesPerPixel() const { return BPP; }

    Rect GetRect() const { return Rect(0, 0, m_w, m_h); }

    inline uint8_t *PixPtr(uint32_t x, uint32_t y)
    {
        return m_data.data() + (BPP * (y * m_w + x));
    }

    inline const uint8_t *PixPtr(uint32_t x, uint32_t y) const
    {
        return m_data.data() + (BPP * (y * m_w + x));
    }

    inline void Blend(uint8_t *dst, const Colour &col, uint8_t opacity)
    {
        const float div = 1.0f / 255.0f;
        float alpha = (float)(opacity * div) * (col.a * div);

        *(dst + 0) = alpha * col.r + (1.0f - alpha) * *(dst + 0);
        *(dst + 1) = alpha * col.g + (1.0f - alpha) * *(dst + 1);
        *(dst + 2) = alpha * col.b + (1.0f - alpha) * *(dst + 2);
    }

    void DrawRect(const Rect &rect, const Colour &col);
    void CopyInto(Image *dstImage, const Rect &dstRect,
                  const std::string hAlign = "c", const std::string vAlign = "m") const;
    void DrawText(const Text &dtext);
    void RBSwap();
    void Clear();

private:
    static const uint32_t BPP = 3;

private:
    uint32_t             m_w = 0;
    uint32_t             m_h = 0;
    std::vector<uint8_t> m_data;
    FontManager         *m_fontMan = nullptr;
};