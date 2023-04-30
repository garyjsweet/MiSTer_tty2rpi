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

#include "GameDatabase.h"
#include "StringManip.h"

#include <filesystem>
#include <iostream>

using namespace std;

map<string, string> s_systemAltMap = {
    { "acornarchimedes", "archie" },
    { "acornatom", "acornatom" },
    { "acornelectron", "acornelectron" },
    { "adam", "avision" },
    { "adventurevision", "avision" },
    { "amiga", "minimig" },
    { "ao486", "ao486" },
    { "arcadia2001", "co2650" },
    { "archie", "archie" },
    { "astrocade", "astrocade" },
    { "atari2600", "atari2600" },
    { "atari5200", "atari5200" },
    { "atari7800", "atari7800" },
    { "atariest/ste", "atarist" },
    { "atarilynx", "atarilynx" },
    { "atarist", "atarist" },
    { "atom", "acornatom" },
    { "avision", "avision" },
    { "ballyastrocade", "astrocade" },
    { "bbcmicro", "bbcmicro" },
    { "bbcmicro/master", "bbcmicro" },
    { "c128", "c128" },
    { "c64", "c64" },
    { "channelf", "channelf" },
    { "coleco", "coleco" },
    { "colecovision", "coleco" },
    { "commodore64", "c64" },
    { "electron", "acornelectron" },
    { "genesis", "genesis" },
    { "lynx48", "lynx48" },
    { "lynx48/96k", "lynx48" },
    { "macintoshplus", "macplus" },
    { "macplus", "macplus" },
    { "mega-cd", "megacd" },
    { "megacd", "megacd" },
    { "megadrive", "genesis" },
    { "megaduck", "megaduck" },
    { "minimig", "minimig" },
    { "mister_rbfs", "arcade" },
    { "neogeo", "neogeo" },
    { "neogeomvs/aes", "neogeo" },
    { "nes", "nes" },
    { "pc(486sx)", "ao486" },
    { "pc/xt", "pcxt" },
    { "pcengineduo", "tgfx16" },
    { "pcxt", "pcxt" },
    { "playstation", "psx" },
    { "psx", "psx" },
    { "ql", "ql" },
    { "s32x", "s32x" },
    { "sgb", "sgb" },
    { "sinclairql", "ql" },
    { "sms", "sms" },
    { "snes", "snes" },
    { "spectrum", "spectrum" },
    { "super32x", "s32x" },
    { "supergameboy", "sgb" },
    { "tgfx16", "tgfx16" },
    { "vectrex", "vectrex" },
    { "wonderswan color", "wonderswancolor" },
    { "wonderswan", "wonderswan" },
    { "zxnext", "zxnext" },
    { "zxspectrum", "spectrum" },
    { "zxspectrumnext", "zxnext" },
};

static string FindSystem(const string &str)
{
    string lower = ToLower(str);

    auto Lookup = [&](const string &s){
        auto lookup = s_systemAltMap.find(s);
        if (lookup != s_systemAltMap.end())
            return lookup->second;
        return string();
    };

    auto TokenizedFind = [&](char sep){
        vector<string> parts = Tokenize(lower, sep);

        string ret;
        for (const string &p : parts)
        {
            ret = Lookup(p);
            if (!ret.empty())
                return ret;
        }
        return string();
    };

    string sys, ret;

    vector<string> parts = Tokenize(lower, ' ');
    for (const string &p : parts)
        sys += p;
    ret = Lookup(sys);
    if (!ret.empty())
        return ret;

    ret = TokenizedFind('/');
    if (ret.size() > 0)
        return ret;

    ret = TokenizedFind('_');
    if (ret.size() > 0)
        return ret;

    ret = TokenizedFind(' ');
    if (ret.size() > 0)
        return ret;

    ret = "";
    for (const string &p : parts)
        ret += p;

    return ret;
}

GameRecord::GameRecord(const string &system, const string &dbRow)
{
    m_system = FindSystem(system);

    m_entries = Tokenize(dbRow, '>');
    m_entries.resize(Field::NUM_FIELDS);

    for (auto &e : m_entries)
        Trim(e);
}

string GameRecord::InfoStr() const
{
    string s, t;

    auto add = [&](uint32_t f, const std::string &name) {
        if (!m_entries[f].empty())
        {
            s += name + ": ";
            s += m_entries[f];
            s += "\n";
        }
    };

    if (!m_entries[TITLE].empty())
        t += m_entries[TITLE];

    add(YEAR, "Year");
    add(GENRE, "Genre");
    add(PLAYERS, "Players");
    add(DEVELOPER, "Developer");
    if (m_entries[DEVELOPER] != m_entries[PUBLISHER])
        add(PUBLISHER, "Publisher");
    add(SCORE, "Score");

    if (!s.empty())
        s = "Title: " + t + "\n" + s;
    else
        s = t;

    return Trim(s);
}

template <typename T>
void ScanPicDir(const string &dir, const string &ext, T *container)
{
    for (const auto &file : filesystem::directory_iterator(dir))
    {
        const string &p = file.path();
        if (EndsWith(p, ext))
        {
            vector<string> parts = Tokenize(p.substr(0, p.size() - ext.size()), '/');
            const string &base = ToLower(parts.back());
            (*container)[base] = p;
        }
    }
}

GameRecord *GameDatabase::AddRecord(const string &system, const string &row)
{
    m_games.emplace_back(system, row);

    GameRecord *gr = &m_games.back();

    const string &rom   = ToLower(gr->Field(GameRecord::ROM));
    const string &title = ToLower(gr->Field(GameRecord::TITLE));

    m_romMap[rom]     = gr;
    m_titleMap[title] = gr;

    const auto iter = m_pictures.find(rom);
    if (iter != m_pictures.end())
        gr->SetPicture(iter->second);

    return gr;
}

void GameDatabase::AddFile(const string &path)
{
    const uint32_t BUFSZ = 32 * 1024;

    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
        throw "Failed to open database file";

    vector<string> parts = Tokenize(path.substr(0, path.size() - 4), '/');
    const string &system = parts.back();

    char buf[BUFSZ];
    while (fgets(buf, BUFSZ, fp))
        AddRecord(system, buf);

    fclose(fp);
}

GameDatabase::GameDatabase(const string &dirname, const string &pictureDir,
                           const string &syspicDir, const string &controllerDir,
                           const string &ext, bool log) :
    m_log(log)
{
    // Scan the pictures
    ScanPicDir(pictureDir, ext, &m_pictures);
    ScanPicDir(syspicDir, ext, &m_sysPictures);
    ScanPicDir(controllerDir, ext, &m_controllerPictures);

    // Scan the database files
    for (const auto &file : filesystem::directory_iterator(dirname))
    {
        const string &p = file.path();
        if (EndsWith(p, ".csv"))
            AddFile(p);
    }
    // Always use the MISTER rbfs as the priority
    AddFile(dirname + "/MISTER_rbfs.csv");

    // Add a dummy menu entry
    GameRecord *rec = AddRecord("MENU", "MENU>MENU");
    rec->SetPicture("MENU" + ext);
}

string GameDatabase::LookupSystemPic(const string &system) const
{
    auto iter = m_sysPictures.find(FindSystem(system));
    if (iter != m_sysPictures.end())
        return iter->second;

    return string();
}

string GameDatabase::LookupControllerPic(const string &system) const
{
    auto iter = m_controllerPictures.find(FindSystem(system));
    if (iter != m_controllerPictures.end())
        return iter->second;

    return string();
}

static string Decode(const string &str)
{
    string s = str;
    Trim(s);

    ReplaceInPlace(s, "%20", " ");
    ReplaceInPlace(s, "%2C", ",");

    return s;
}

static string ExtractName(const vector<string> &strs, uint32_t indx)
{
    if (strs.size() >= indx + 1)
        return Decode(strs[indx]);
    return string();
}

static string TitleFromStartPath(const string &sp)
{
    vector<string> parts = Tokenize(sp, '/');
    string s = parts.back();
    StripExtension(s);
    return s;
}

std::map<std::string, const GameRecord *>::iterator
GameDatabase::LookupVariantsWithPic(const string &title)
{
    // Try shortened versions of the title
    vector<string> parts = Tokenize(ToLower(title), ' ');

    for (size_t p = parts.size() - 1; p > 1; p--)
    {
        string s;
        for (size_t pp = 0; pp < p; pp++)
            s += parts[pp] + " ";
        s[s.size() - 1] = '\0';

        if (m_log)
            cout << "Trying alternate: '" << s << "'\n";

        auto iter = m_titleMap.find(s);
        if (iter != m_titleMap.end())
        {
            const GameRecord &rc = *(iter->second);
            if (!rc.Picture().empty())
                return iter;
        }
    }

    return m_titleMap.end();
}

GameRecord GameDatabase::Lookup(const string &key, string *sys)
{
    vector<string> strs = Tokenize(key, ',');

    string corename  = ExtractName(strs, 1);
    string fullpath  = ExtractName(strs, 2);
    string curpath   = ExtractName(strs, 3);
    string startpath = ExtractName(strs, 4);
    string select    = ExtractName(strs, 5);
    string samgame   = ExtractName(strs, 6);

    StripExtension(curpath);

    string core, title, altTitle, system;
    if (startpath == "/tmp/SAM_game.mgl" && select != "active")
    {
        // SAM is active : use corename & samgame
        if (m_romMap.find(corename) != m_romMap.end())
            system = "arcade";
        else
            system = FindSystem(corename);

        core  = corename;
        title = samgame;
    }
    // "selected" is to ensure the menu doesn't change when the game is loading
    else if (select == "active" || select == "selected")
    {
        string lfp = ToLower(fullpath);
        if (lfp == "_console" || lfp == "_computer")
            lfp = ToLower(curpath);

        // Menu is active, use curpath & fullpath
        if (lfp.find("arcade") != string::npos)
            system = "arcade";
        else
            system = FindSystem(lfp);

        core = "";
        title = curpath;
    }
    else if (select == "cancelled")
    {
        // Game is active, use corename & startpath
        if (m_romMap.find(corename) != m_romMap.end())
            system = "arcade";
        else
            system = FindSystem(corename);

        core = corename;
        title = TitleFromStartPath(startpath);
    }

    if (m_log)
        cout << "Sys = '" << system << "', Core = '" << core <<
                "', Title = '" << title << "', Alt = '" << altTitle << "'\n";

    std::map<std::string, const GameRecord *>::iterator coreLookup     = m_romMap.end();
    std::map<std::string, const GameRecord *>::iterator titleLookup    = m_titleMap.end();
    std::map<std::string, const GameRecord *>::iterator altTitleLookup = m_titleMap.end();

    if (core != "" && core != "menu")
        coreLookup = m_romMap.find(ToLower(core));

    if (title != "")
        titleLookup = m_titleMap.find(ToLower(title));

    if (altTitle != "")
        altTitleLookup = m_titleMap.find(ToLower(altTitle));

    if (titleLookup == m_titleMap.end() && altTitleLookup == m_titleMap.end() &&
        (title != "" || altTitle != ""))
    {
        if (title != "")
            titleLookup = LookupVariantsWithPic(title);
        if (titleLookup == m_titleMap.end() && altTitle != "")
            altTitleLookup = LookupVariantsWithPic(altTitle);
    }

    if (m_log)
    {
        if (coreLookup != m_romMap.end())
            cout << "Core Lookup = \n" << coreLookup->second->InfoStr() << "\n";

        if (titleLookup != m_titleMap.end())
            cout << "Title Lookup = \n" << titleLookup->second->InfoStr() << "\n";

        if (altTitleLookup != m_titleMap.end())
            cout << "Alt Lookup = \n" << altTitleLookup->second->InfoStr() << "\n";
    }

    *sys = system;

    if (coreLookup != m_romMap.end())
        return *coreLookup->second;
    if (titleLookup != m_titleMap.end())
        return *titleLookup->second;
    if (altTitleLookup != m_titleMap.end())
        return *altTitleLookup->second;

    // Make a fallback db entry from the lookup key
    if (m_log)
        cout << "FALLBACK '" << system << "' '" << corename << "' '" << title << "'\n";

    return GameRecord(system, corename + ">" + title + ">");
}
