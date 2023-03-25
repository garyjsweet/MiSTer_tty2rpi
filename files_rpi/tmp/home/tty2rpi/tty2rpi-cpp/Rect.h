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

#include <cstdint>
#include <string>

struct Rect
{
    int32_t x{}, y{}, w{}, h{};

    Rect() = default;

    Rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) :
        x(x), y(y), w(w), h(h)
    {}

    float Aspect() const { return (float)w / float(h); }

    Rect FitToContainer(const Rect &container, const std::string &horz,
                        const std::string &vert, bool keepSize)
    {
        Rect fit;
        float aspect = Aspect();
        float container_aspect = container.Aspect();

        fit = *this;

        if (!keepSize && aspect > container_aspect) // Fit width
        {
            fit.x = container.x;
            fit.w = container.w;
            fit.h = (uint32_t)((float)fit.w / aspect);
        }
        if (!keepSize && aspect <= container_aspect) // Fit height
        {
            fit.y = container.y;
            fit.h = container.h;
            fit.w = (uint32_t)((float)fit.h * aspect);
        }

        int32_t vertical_space = container.h - fit.h;
        if (vert == "m" || vert == "c")
            fit.y = container.y + (vertical_space / 2);
        else if (vert == "t")
            fit.y = container.y;
        else if (vert == "b")
            fit.y = container.y + vertical_space;

        int32_t horizontal_space = container.w - fit.w;
        if (horz == "m" || horz == "c")
            fit.x = container.x + (horizontal_space / 2);
        else if (horz == "l")
            fit.x = container.x;
        else if (horz == "r")
            fit.x = container.x + horizontal_space;

        return fit;
    }

    Rect Enlarge(int32_t pixels)
    {
        return Rect(x - pixels, y - pixels, w + pixels * 2, h + pixels * 2);
    }

    Rect Intersect(const Rect &r)
    {
        Rect ret;
        ret.x = std::max(x, r.x);
        ret.y = std::max(y, r.y);

        int32_t mr = x + w;
        int32_t mb = y + h;
        int32_t rr = r.x + r.w;
        int32_t rb = r.y + r.h;

        mr = std::min(mr, rr);
        mb = std::min(mb, rb);

        ret.w = mr - ret.x;
        ret.h = mb - ret.y;

        return ret;
    }
};
