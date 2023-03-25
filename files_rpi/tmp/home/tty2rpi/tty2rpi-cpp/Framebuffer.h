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

class Framebuffer
{
public:
    Framebuffer()
    {
#if PI_BUILD
        m_fbFP = fopen("/dev/fb0", "wb");
        if (!m_fbFP)
            throw "Couldn't open fb0 file";
#endif
    }

    ~Framebuffer()
    {
        if (m_fbFP)
            fclose(m_fbFP);
    }

    void SetToImage(const Image &img)
    {
#if PI_BUILD
        fseek(m_fbFP, 0, SEEK_SET);
        fwrite(img.PixPtr(0, 0), img.Width() * img.Height() * img.BytesPerPixel(),
               1, m_fbFP);
#endif
    }

private:
    FILE *m_fbFP = nullptr;
};