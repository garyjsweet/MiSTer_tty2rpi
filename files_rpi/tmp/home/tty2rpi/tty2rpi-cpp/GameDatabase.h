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

#include <string>
#include <vector>
#include <list>
#include <map>

class GameRecord
{
public:
    enum Field
    {
        ROM = 0,
        TITLE,
        YEAR,
        RATING,
        PUBLISHER,
        DEVELOPER,
        GENRE,
        SCORE,
        PLAYERS,
        DESCRIPTION,
        NUM_FIELDS
    };

    GameRecord(const std::string &system, const std::string &dbRow);

    const std::string &Picture() const { return m_picture; }
    void SetPicture(const std::string &pictureFile) { m_picture = pictureFile; }

    const std::string &System() const { return m_system; }
    const std::string &Field(Field f) const { return m_entries[f]; }

    std::string InfoStr() const;

private:
    std::string              m_picture;
    std::string              m_system;
    std::vector<std::string> m_entries;
};

class GameDatabase
{
public:
    GameDatabase(const std::string &dirname, const std::string &pictureDir,
                 const std::string &syspicDir, const std::string &ext, bool log);

    GameRecord Lookup(const std::string &key, std::string *system);
    std::string LookupSystemPic(const std::string &system) const;

private:
    void AddFile(const std::string &path);
    GameRecord *AddRecord(const std::string &system, const std::string &row);

    std::map<std::string, const GameRecord *>::iterator
    LookupVariants(const std::string &title);

private:
    std::list<GameRecord>                     m_games;
    std::map<std::string, const GameRecord *> m_romMap;
    std::map<std::string, const GameRecord *> m_titleMap;
    std::map<std::string, std::string>        m_pictures;
    std::map<std::string, std::string>        m_sysPictures;
    bool                                      m_log;
};