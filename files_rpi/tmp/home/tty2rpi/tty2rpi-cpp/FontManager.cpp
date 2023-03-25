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

#include "FontManager.h"

FontManager::FontManager(bool hasText) : m_hasText(hasText)
{
    if (!m_hasText)
        return;

    if (FT_Init_FreeType(&m_library))
        throw "FT_Init_FreeType failed";

    if (FT_New_Face(m_library, "/usr/share/fonts/truetype/freefont/FreeSansBoldOblique.ttf",
                    0, &m_face))
        throw "Failed to create new freetype face";

    if (FT_Select_Charmap(m_face, FT_ENCODING_UNICODE))
        throw "Failed to select charmap";

    if (FT_New_Size(m_face, &m_baseSize))
        throw "Failed to make new font size";

    if (FT_Activate_Size(m_baseSize))
        throw "Failed to activate size";

    if (FT_Set_Char_Size(m_face, BASE_FONT_SIZE * 64, 0, DISPLAY_DPI, 0))
        throw "Can't set char size";

    m_slot = m_face->glyph;
}

FontManager::~FontManager()
{
    if (m_hasText)
    {
        FT_Done_Face(m_face);
        FT_Done_FreeType(m_library);
    }
}

void FontManager::RestoreBaseSize()
{
    if (FT_Activate_Size(m_baseSize))
        throw "Failed to activate size";

    if (FT_Set_Char_Size(m_face, BASE_FONT_SIZE * 64, 0, DISPLAY_DPI, 0))
        throw "Can't set char size";
}

void FontManager::NewSize(uint32_t size)
{
    FT_Size ftSize;
    if (FT_New_Size(m_face, &ftSize))
        throw "Failed to make new font size";

    if (FT_Activate_Size(ftSize))
        throw "Failed to activate size";

    if (FT_Set_Char_Size(m_face, size * 64, 0, DISPLAY_DPI, 0))
        throw "Can't set char size";
}

Rect FontManager::TextBound(const std::string &str)
{
    FT_Pos maxX = 0, maxY = 0;

    FT_Vector pen;
    pen.x = 0;
    pen.y = 0;

    FT_GlyphSlot slot = m_slot;

    PrepChar(pen, 'X', /*render=*/false);
    FT_Int charHeight = slot->bitmap.rows;
    FT_Int lineHeight = (charHeight + LINE_SPACING) * 64;

    for (size_t n = 0; n < str.size(); n++)
    {
        char c = str.at(n);

        if (c == '\n')
        {
            pen.y += lineHeight;
            pen.x = 0;
            continue;
        }

        if (!PrepChar(pen, c, /*render=*/false))
            continue;

        pen.x += slot->advance.x;
        pen.y += slot->advance.y;

        maxX = std::max(maxX, pen.x);
        maxY = std::max(maxY, pen.y);
    }

    return Rect(0, 0, maxX / 64, maxY / 64 + charHeight);
}