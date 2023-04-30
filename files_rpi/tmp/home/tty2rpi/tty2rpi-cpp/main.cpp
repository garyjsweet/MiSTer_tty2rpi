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
#include "Image.h"
#include "Framebuffer.h"
#include "GameDatabase.h"
#include "Socket.h"
#include "StringManip.h"
#include "Buttons.h"

#include <iostream>
#include <fstream>
#include <filesystem>

#ifdef PI_BUILD
const std::string ROOT = "/home/pi/";
#else
const std::string ROOT = "/home/gary/Dev/MyTTY2RPi_fork/files_rpi/tmp/home/tty2rpi/";
#endif

enum UIMode
{
    UI_MAIN = 0,
    UI_CONTROLLER,
};

const Rect DISPLAY_R(0, 0, 1920, 480);

static void StartupMessage(Framebuffer &fb, Image &dstImage, const Image &bgImg,
                           const std::string &msg)
{
    bgImg.CopyInto(&dstImage, dstImage.GetRect());


    Image::Text txt;
    txt.rect = Rect(0, 0, DISPLAY_R.w, DISPLAY_R.h - 30);
    txt.text = msg;
    txt.colour = Colour(200, 200, 200, 200);
    txt.horz = "m";
    txt.vert = "b";
    txt.bgCol = Colour(0, 0, 0, 0); // No bg rect

    dstImage.DrawText(txt);
    dstImage.RBSwap();

    fb.SetToImage(dstImage);
}

static inline bool FileExists(const std::string &name)
{
    return std::filesystem::exists(name) && !std::filesystem::is_directory(name);
}

static void BuildMainUI(Image &dstImage, GameDatabase &db, const std::string &data, bool log)
{
    std::string system;
    GameRecord rec = db.Lookup(data, &system);

    std::string sysPicName = db.LookupSystemPic(system);
    std::string displayStr = rec.InfoStr();

    std::string picFile = rec.Picture();
    std::string sysFile = sysPicName;

    dstImage.Clear();

    if (log)
    {
        std::cout << "picFile = '" << picFile << "'\n";
        std::cout << "sysFile = '" << sysFile << "'\n";
    }

    bool hasSys   = false;
    bool needText = true;
    Rect rect     = DISPLAY_R;

    if (FileExists(picFile))
    {
        Image picImage(picFile);
        picImage.CopyInto(&dstImage, rect);
    }
    else
    {
        Rect textRect = DISPLAY_R;

        if (!FileExists(sysFile))
            sysFile = db.LookupSystemPic(displayStr);

        if (FileExists(sysFile))
        {
            Image sysImage(sysFile);
            sysImage.CopyInto(&dstImage, sysImage.GetRect());

            uint32_t sysW = sysImage.GetRect().w;
            textRect = Rect(sysW, 0, DISPLAY_R.w - sysW, DISPLAY_R.h);
        }

        Trim(Trim(Trim(displayStr, "_"), "@"), "_");

        Image::Text txt;
        txt.rect = textRect;
        txt.text = displayStr;
        txt.colour = Colour(230, 230, 230, 255);
        txt.horz = "m";
        txt.vert = "m";
        txt.bgCol = Colour(0, 0, 0, 0); // No bg rect

        dstImage.DrawText(txt);
    }
}

static void BuildControllerUI(Image &dstImage, GameDatabase &db, const std::string &data, bool log)
{
    std::string system;
    GameRecord rec = db.Lookup(data, &system);

    std::string sysFile = db.LookupSystemPic(system);

    Rect textRect = DISPLAY_R;

    dstImage.Clear();

    if (!FileExists(sysFile))
        sysFile = db.LookupSystemPic(rec.InfoStr());

    if (FileExists(sysFile))
    {
        Image sysImage(sysFile);
        sysImage.CopyInto(&dstImage, sysImage.GetRect());

        uint32_t sysW = sysImage.GetRect().w;
        textRect = Rect(sysW, 0, DISPLAY_R.w - sysW, DISPLAY_R.h);
    }

    std::string ctrlPic = db.LookupControllerPic(system);

    if (!ctrlPic.empty() && FileExists(ctrlPic))
    {
        Image ctrlImage(ctrlPic);
        ctrlImage.CopyInto(&dstImage, textRect, "c", "b");
    }
    else
    {
        BuildMainUI(dstImage, db, data, log);
        return;
    }

    Image::Text txt;
    txt.rect = textRect;
    txt.text = rec.Field(GameRecord::TITLE);
    txt.colour = Colour(230, 230, 230, 255);
    txt.horz = "m";
    txt.vert = "t";
    txt.bgCol = Colour(0, 0, 0, 0); // No bg rect

    dstImage.DrawText(txt);
}

int main(int argc, char **argv)
{
    // Any arguments will cause log output to be produced
    bool log = argc > 1;

    try
    {
        Framebuffer  framebuffer;
        FontManager  fontMan(true);
        Image        dstImage(DISPLAY_R.w, DISPLAY_R.h, &fontMan);
        Image        startupImage(ROOT + "marquee-pictures/STARTUP.ppm");

        // Display a loading message whilst loading the database and images
        StartupMessage(framebuffer, dstImage, startupImage, "Loading databases ...");

        GameDatabase db(ROOT + "database",
                        ROOT + "marquee-pictures",
                        ROOT + "marquee-pictures/systems",
                        ROOT + "marquee-pictures/systems/controllers",
                        ".ppm", log);

        StartupMessage(framebuffer, dstImage, startupImage, "Databases loaded");
        startupImage = Image(); // Return resources back to system

        // Start the button handling object
        Buttons buttons;

        // Start looking for socket connections
        Socket      socket(&buttons);
        std::string curData;
        UIMode      mode = UI_MAIN;

        while (true)
        {
            // Wait for data on the socket (or a pi menu button press)
            std::string data = socket.GetDataBlocking();
            if (data.empty())
            {
                // Button press - just toggle main vs controller ui regardless of button
                // Note: we must call GetPressedMask() to reset the state
                uint32_t mask = buttons.GetPressedMask();
                if (log)
                    std::cout << "BUTTONS PRESSED = " << mask << "\n";

                mode = mode == UI_MAIN ? UI_CONTROLLER : UI_MAIN;
            }
            else
            {
                // Socket data
                if (log)
                    std::cout << "Received socket data '" << data << "'\n";

                if (data == curData || data.empty())
                    continue;

                const std::string cmdTag = "CMDCOREXTRA,";
                if (data.substr(0, cmdTag.length()) != cmdTag)
                    continue;

                curData = data;
            }

            // Build UI image
            switch (mode)
            {
            case UI_MAIN :
                BuildMainUI(dstImage, db, curData, log);
                break;
            case UI_CONTROLLER :
                BuildControllerUI(dstImage, db, curData, log);
                break;
            }

            // Display UI image
            dstImage.RBSwap();
            framebuffer.SetToImage(dstImage);
        }
    }
    catch(const char *s)
    {
        std::cerr << "EXCEPTION: " << s << '\n';
        return -1;
    }

    return 0;
}