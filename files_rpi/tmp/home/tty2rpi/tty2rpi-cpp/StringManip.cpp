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

#include "StringManip.h"

#include <sstream>

using namespace std;

string &RTrim(string &s, const char *t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

string &LTrim(string &s, const char *t)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

string &Trim(string &s, const char *t)
{
    return LTrim(RTrim(s, t), t);
}

bool EndsWith(const string &value, const string &ending)
{
    if (ending.size() > value.size())
        return false;

    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

string ToLower(const string &str)
{
    string ret(str.size(), ' ');
    for (uint32_t i = 0; i < str.size(); i++)
        ret[i] = tolower(str[i]);
    return ret;
}

vector<string> Tokenize(const string &s, char separator)
{
    vector<string> tokens;
    istringstream strm(s);
    string token;
    while (getline(strm, token, separator))
        tokens.push_back(token);
    return tokens;
}

void ReplaceInPlace(string &subject, const string &search, const string &replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
         subject.replace(pos, search.size(), replace);
         pos += replace.size();
    }
}

void StripExtension(string &s)
{
    if (s.size() <= 4)
        return;
    if (s[s.size() - 4] == '.')
        s = s.substr(0, s.size() - 4);
}
