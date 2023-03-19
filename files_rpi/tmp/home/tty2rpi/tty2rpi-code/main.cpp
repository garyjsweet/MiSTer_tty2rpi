#include <stdio.h>
#include <memory.h>

#include <cstdint>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>

#include <ft2build.h>
#include <freetype/ftsizes.h>
#include FT_FREETYPE_H

// fbset -depth 24
// Raw FB is BGR, not RGB

#if PI_BUILD
static const std::string picsRoot = "/home/pi/marquee-pictures";
#else
static const std::string picsRoot = "/home/gary/Dev/MyTTY2RPi_fork/files_rpi/tmp/home/tty2rpi/marquee-pictures";
#endif

const uint32_t BASE_FONT_SIZE = 20;
const uint32_t LINE_SPACING   = 20;
const uint32_t MAX_FONT_SIZE  = 26;
const uint32_t DISPLAY_DPI    = 220;  // 1920 over 8.8 inches

struct Point
{
    uint32_t x{}, y{};

    Point() = default;
};

struct Rect
{
    uint32_t x{}, y{}, w{}, h{};

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

        uint32_t vertical_space = container.h - fit.h;
        if (vert == "m" || vert == "c")
            fit.y = container.y + (vertical_space / 2);
        else if (vert == "t")
            fit.y = container.y;
        else if (vert == "b")
            fit.y = container.y + vertical_space;

        uint32_t horizontal_space = container.w - fit.w;
        if (horz == "m" || horz == "c")
            fit.x = container.x + (horizontal_space / 2);
        else if (horz == "l")
            fit.x = container.x;
        else if (horz == "r")
            fit.x = container.x + horizontal_space;

        return fit;
    }
};

struct Colour
{
    uint32_t r, g, b, a;
};

struct Drawable
{
    bool isImage {};

    struct Image
    {
        Rect        rect;
        std::string file;
    } image;

    struct Text
    {
        Rect        rect;
        std::string text;
        Colour      colour;
        std::string horz;
        std::string vert;
        Colour      bgCol;
    } text;
};

class FontManager
{
public:
    FontManager(bool hasText) : m_hasText(hasText)
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

    ~FontManager()
    {
        if (m_hasText)
        {
            FT_Done_Face(m_face);
            FT_Done_FreeType(m_library);
        }
    }

    void RestoreBaseSize()
    {
        if (FT_Activate_Size(m_baseSize))
            throw "Failed to activate size";

        if (FT_Set_Char_Size(m_face, BASE_FONT_SIZE * 64, 0, DISPLAY_DPI, 0))
            throw "Can't set char size";
    }

    bool PrepChar(FT_Vector &pen, char c)
    {
        FT_Set_Transform(m_face, nullptr, &pen);

        // load glyph image into the slot (erase previous one)
        if (FT_Load_Char(m_face, c, FT_LOAD_RENDER))
            return false;

        return true;
    }

    FT_GlyphSlot Slot() { return m_slot; }

    void NewSize(uint32_t size)
    {
        FT_Size ftSize;
        if (FT_New_Size(m_face, &ftSize))
            throw "Failed to make new font size";

        if (FT_Activate_Size(ftSize))
            throw "Failed to activate size";

        if (FT_Set_Char_Size(m_face, size * 64, 0, DISPLAY_DPI, 0))
            throw "Can't set char size";
    }

    Rect TextBound(const std::string &str)
    {
        FT_Pos maxX = 0, maxY = 0;

        FT_Vector pen;
        pen.x = 0;
        pen.y = 0;

        FT_GlyphSlot slot = m_slot;

        PrepChar(pen, 'X');
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

            if (!PrepChar(pen, c))
                continue;

            /* increment pen position */
            pen.x += slot->advance.x;
            pen.y += slot->advance.y;

            maxX = std::max(maxX, pen.x);
            maxY = std::max(maxY, pen.y);
        }

        return Rect(0, 0, maxX / 64, maxY / 64 + charHeight);
    }

private:
    bool         m_hasText = false;
    FT_Library   m_library;
    FT_Face      m_face;
    FT_GlyphSlot m_slot;
    FT_Size      m_baseSize;
};

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

class Image
{
public:
    Image(uint32_t w, uint32_t h, FontManager *fm) : m_w(w), m_h(h), m_fontMan(fm)
    {
        m_data.resize(w * h * 3);
        memset(m_data.data(), 0, m_data.size());
    }

    Image(uint32_t w, uint32_t h, uint8_t *data) :
        m_w(w), m_h(h)
    {
        m_data.resize(w * h * 3);
        memcpy(m_data.data(), data, m_data.size());
    }

    Image(const std::string &ppmFile)
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

            printf("w = %u, h = %u\n", m_w, m_h);

            size_t b = m_w * m_h * 3;
            m_data.resize(b);

            size_t bytes = fread(m_data.data(), 1, b, fp);
            //if (bytes != b)
            //    throw "PPM byte mismatch";
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

    uint32_t Width() const  { return m_w; }
    uint32_t Height() const { return m_h; }

    Rect GetRect() const { return Rect(0, 0, m_w, m_h); }

    uint8_t *Data() { return m_data.data(); }
    const uint8_t *Data() const { return m_data.data(); }

    void CopyInto(Image *dstImage, const Rect &dstRect)
    {
        Rect fit = GetRect().FitToContainer(dstRect, "c", "m", /*keepSize=*/true);

        uint8_t *s = Data();
        uint8_t *d = dstImage->Data();

        for (uint32_t y = 0; y < fit.h; y++)
        {
            uint32_t yy = fit.y + y;
            memcpy(d + yy * 1920 * 3 + fit.x * 3, s + y * m_w * 3, m_w * 3);
        }
    }

    void Blend(uint8_t *dst, const Colour &col, uint8_t opacity)
    {
        const float div = 1.0f / 255.0f;
        float alpha = (float)(opacity * div) * (col.a * div);

        *(dst + 0) = alpha * col.r + (1.0f - alpha) * *(dst + 0);
        *(dst + 1) = alpha * col.b + (1.0f - alpha) * *(dst + 1);
        *(dst + 2) = alpha * col.g + (1.0f - alpha) * *(dst + 2);
    }

    void DrawText(const Drawable::Text &dtext)
    {
        const Colour &col = dtext.colour;

        m_fontMan->RestoreBaseSize();
        Rect bbox = m_fontMan->TextBound(dtext.text);
        printf("BBOX = %u, %u, %u, %u\n", bbox.x, bbox.y, bbox.w, bbox.h);

        Rect fit = bbox.FitToContainer(dtext.rect, dtext.horz, dtext.vert, /*keepSize=*/false);
        printf("CONT = %u, %u, %u, %u\n", dtext.rect.x, dtext.rect.y, dtext.rect.w, dtext.rect.h);
        printf("FIT  = %u, %u, %u, %u\n", fit.x, fit.y, fit.w, fit.h);

        float scale;
        if (bbox.Aspect() > fit.Aspect())
            scale = (float)fit.w / bbox.w;
        else
            scale = (float)fit.h / bbox.h;

        printf("Scale = %f\n", scale);

        m_fontMan->NewSize(std::min(MAX_FONT_SIZE, (uint32_t)(scale * BASE_FONT_SIZE)));
        bbox = m_fontMan->TextBound(dtext.text);
        fit = bbox.FitToContainer(dtext.rect, dtext.horz, dtext.vert, /*keepSize=*/true);
        printf("NEW FIT = %u, %u, %u, %u\n", fit.x, fit.y, fit.w, fit.h);

        // Pen is in 26.6 cartesian space coordinates
        FT_Vector pen;
        pen.x = fit.x * 64;
        pen.y = (m_h - fit.y) * 64;

        printf("Start = %ld, %ld\n", pen.x / 64, pen.y / 64);

        FT_GlyphSlot slot = m_fontMan->Slot();

        m_fontMan->PrepChar(pen, 'X');
        FT_Int charHeight = slot->bitmap.rows;
        FT_Int lineHeight = (charHeight + LINE_SPACING) * 64;

        uint8_t *dst = m_data.data();

        for (size_t n = 0; n < dtext.text.size(); n++)
        {
            char c = dtext.text.at(n);

            if (c == '\n')
            {
                pen.y -= lineHeight;
                pen.x = fit.x * 64;
                continue;
            }

            if (!m_fontMan->PrepChar(pen, c))
                continue;

            FT_Bitmap bitmap = slot->bitmap;

            /* now, draw to our target surface (convert position) */
            FT_Int  x = slot->bitmap_left;
            FT_Int  y = m_h - slot->bitmap_top + charHeight;
            FT_Int  i, j, p, q;
            FT_Int  x_max = x + bitmap.width;
            FT_Int  y_max = y + bitmap.rows;

            for (i = x, p = 0; i < x_max; i++, p++)
            {
                for (j = y, q = 0; j < y_max; j++, q++)
                {
                    if (i < 0 || j < 0 || i >= m_w || j >= m_h)
                        continue;

                    uint8_t greyVal = bitmap.buffer[q * bitmap.width + p];
                    Blend(dst + j * m_w * 3 + i * 3, col, greyVal);
                }
            }

            /* increment pen position */
            pen.x += slot->advance.x;
            pen.y += slot->advance.y;
        }

        printf("End = %ld, %ld\n", pen.x / 64, pen.y / 64);
    }

    void RBSwap()
    {
        uint8_t *p = Data();

        for (uint32_t y = 0; y < m_h; y++)
        {
            for (uint32_t x = 0; x < m_w; x++)
            {
                std::swap(p[0], p[2]);
                p += 3;
            }
        }
    }

private:
    uint32_t             m_w;
    uint32_t             m_h;
    std::vector<uint8_t> m_data;
    FontManager         *m_fontMan = nullptr;
};

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
        fwrite(img.Data(), img.Width() * img.Height(), 3, m_fbFP);
#endif
    }

private:
    FILE *m_fbFP = nullptr;
};

std::vector<Drawable> LoadDesc(const std::string &filename, bool *hasText)
{
    std::vector<Drawable> res;

    *hasText = false;

    FILE *fp = fopen(filename.c_str(), "r");
    if (!fp)
        return res;

    char c;
    while (!feof(fp) && (c = getc(fp)) != EOF)
    {
        char buf[4096];
        Drawable d;

        if (c == '`')
            c = getc(fp);

        if (c == EOF)
            break;

        if (c == 'I')
        {
            Drawable::Image &i = d.image;
            d.isImage = true;
            fscanf(fp, ":(%u, %u, %u, %u) F:%s\n", &i.rect.x, &i.rect.y, &i.rect.w, &i.rect.h,
                buf);
            i.file = buf;
        }
        else if (c == 'T')
        {
            Drawable::Text &t = d.text;
            d.isImage = false;
            char hstr[48];
            char vstr[48];
            fscanf(fp, ":(%u, %u, %u, %u) C:(%u, %u, %u, %u) H:%s V:%s B:(%u, %u, %u, %u) S:",
                &t.rect.x, &t.rect.y, &t.rect.w, &t.rect.h,
                &t.colour.r, &t.colour.g, &t.colour.b, &t.colour.a,
                hstr, vstr,
                &t.bgCol.r, &t.bgCol.g, &t.bgCol.b, &t.bgCol.a);

            d.text.horz = hstr;
            d.text.vert = vstr;

            // Read to end of line
            char *s = fgets(buf, 4096, fp);
            if (!s)
                break;
            t.text = buf;

            while (true)
            {
                char cc = getc(fp);
                if (cc == EOF)
                    break;

                if (cc != '`')
                {
                    if (cc == '\n')
                        t.text += "\n";
                    else
                    {
                        char *s = fgets(buf, 4096, fp);
                        if (!s)
                            break;
                        t.text += cc + std::string(buf);
                    }
                }
                else
                    break;
            }

            if (t.text.at(t.text.size() - 1) == '\n')
                t.text.pop_back();

            printf("T = %s\n", t.text.c_str());

            *hasText = true;
        }
        else
            fprintf(stderr, "Error parsing desc (%c)\n", c);

        res.push_back(d);
    }

    fclose(fp);

    return res;
}

int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
            throw "Usage: tty2rpi-cpp <descfile>";

        bool                  hasText = false;
        Framebuffer           framebuffer;
        std::vector<Drawable> drawables = LoadDesc(argv[1], &hasText);

        FontManager fontMan(hasText);

        Image dstImage(1920, 480, &fontMan);

        for (const Drawable &dbl : drawables)
        {
            printf("Drawable\n");
            if (dbl.isImage)
            {
                Image curImage(dbl.image.file);
                curImage.CopyInto(&dstImage, dbl.image.rect);
            }
            else
            {
                printf("Text\n");
                dstImage.DrawText(dbl.text);
            }
        }

        // Framebuffer is BGR, our images are RGB
        dstImage.RBSwap();

        framebuffer.SetToImage(dstImage);
    }
    catch(const char *s)
    {
        std::cerr << "EXCEPTION: " << s << '\n';
        return -1;
    }

    return 0;
}