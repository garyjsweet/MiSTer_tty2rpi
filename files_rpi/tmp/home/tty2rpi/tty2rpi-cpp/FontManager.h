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

#include "Rect.h"

#include <string>
#include <ft2build.h>
#include <freetype/ftsizes.h>
#include FT_FREETYPE_H

const uint32_t BASE_FONT_SIZE = 20;
const uint32_t LINE_SPACING   = 20;
const uint32_t MAX_FONT_SIZE  = 26;
const uint32_t DISPLAY_DPI    = 220;  // 1920 over 8.8 inches

class FontManager
{
public:
    FontManager(bool hasText);
    ~FontManager();

    void RestoreBaseSize();

    bool PrepChar(FT_Vector &pen, char c, bool render)
    {
        FT_Set_Transform(m_face, nullptr, &pen);

        // load glyph image into the slot (erase previous one)
        if (FT_Load_Char(m_face, c, render ? FT_LOAD_RENDER : FT_LOAD_NO_BITMAP))
            return false;

        return true;
    }

    FT_GlyphSlot Slot() { return m_slot; }

    void NewSize(uint32_t size);

    Rect TextBound(const std::string &str);

private:
    bool         m_hasText = false;
    FT_Library   m_library;
    FT_Face      m_face;
    FT_GlyphSlot m_slot;
    FT_Size      m_baseSize;
};
