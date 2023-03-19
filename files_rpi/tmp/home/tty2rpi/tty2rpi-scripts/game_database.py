import os.path
from log import log

system_map = {
    "adam" : "avision",
    "avision" : "avision",
    "adventurevision" : "avision",
    "arcadia2001" : "co2650",
    "atari2600" : "atari2600",
    "atari5200" : "atari5200",
    "atari7800" : "atari7800",
    "atarilynx" : "atarilynx",
    "ballyastrocade" : "astrocade",
    "astrocade" : "astrocade",
    "genesis" : "genesis",
    "megadrive" : "genesis",
    "coleco" : "coleco",
    "colecovision" : "coleco",
    "channelf" : "channelf",
    "mega-cd" : "megacd",
    "megacd" : "megacd",
    "megaduck" : "megaduck",
    "mister_rbfs" : "arcade",
    "neogeomvs/aes" : "neogeo",
    "neogeo" : "neogeo",
    "nes" : "nes",
    "snes" : "snes",
    "pcengineduo" : "tgfx16",
    "tgfx16" : "tgfx16",
    "playstation" : "psx",
    "psx" : "psx",
    "super32x" : "s32x",
    "s32x" : "s32x",
    "sms" : "sms",
    "supergameboy" : "sgb",
    "sgb" : "sgb",
    "acornarchimedes" : "archie",
    "archie" : "archie",
    "amiga" : "minimig",
    "minimig" : "minimig",
    "atariest/ste" : "atarist",
    "atarist" : "atarist",
    "atom" : "acornatom",
    "acornatom" : "acornatom",
    "bbcmicro/master" : "bbcmicro",
    "bbcmicro" : "bbcmicro",
    "commodore64" : "c64",
    "c64" : "c64",
    "c128" : "c128",
    "electron" : "acornelectron",
    "acornelectron" : "acornelectron",
    "lynx48/96k" : "lynx48",
    "lynx48" : "lynx48",
    "macintoshplus" : "macplus",
    "macplus" : "macplus",
    "pc(486sx)" : "ao486",
    "ao486" : "ao486",
    "pc/xt" : "pcxt",
    "pcxt" : "pcxt",
    "sinclairql" : "ql",
    "ql" : "ql",
    "zxspectrum" : "spectrum",
    "spectrum" : "spectrum",
    "zxspectrumnext" : "zxnext",
    "zxnext" : "zxnext",
    "vectrex" : "vectrex",
    "wonderswan" : "wonderswan",
    "wonderswan color" : "wonderswancolor"
}

def find_system(s):
    words = s.lower().split()
    sys = ''.join(words)
    if sys in system_map:
        return system_map[sys]

    parts = s.lower().split("/")
    for p in parts:
        if p in system_map:
            return system_map[p]

    parts = s.lower().split("_")
    for p in parts:
        if p in system_map:
            return system_map[p]

    for w in words:
        if w in system_map:
            return system_map[w]
    return sys

sys_pics = {}

# rom>title>year>rating>publisher>developer>genre>score>players>description>
ROM = 0
TITLE = 1
YEAR = 2
RATING = 3
PUBLISHER = 4
DEVELOPER = 5
GENRE = 6
SCORE = 7
PLAYERS = 8
DESCRIPTION = 9
NUM_ENTRIES = 10

class GameData:
    _picture = ''

    def __init__(self, system, row):
        self._system  = find_system(system)
        self._entries = row.split('>')
        while len(self._entries) < NUM_ENTRIES:
            self._entries.append('')
        for i in range(0, NUM_ENTRIES - 1):
            self._entries[i] = self._entries[i].strip()

    def _field(self, f):
        return self._entries[f]

    def system(self):
        return self._system

    def set_picture(self, p):
        self._picture = p

    def picture(self):
        return self._picture

    def system_picture(self):
        global sys_pics
        if self.system() in sys_pics:
            return sys_pics[self.system()]
        else:
            return ''

    def log(self):
        log(self._system + ': ' + '>'.join(self._entries))

    def _entry_info(self, s, name, indx):
        if len(self._entries) >= indx and self._field(indx):
            s.append(name + ": ")
            s.append(self._field(indx))

    def info_str(self):
        s = []
        self._entry_info(s, "Title", TITLE)
        self._entry_info(s, "Year", YEAR)
        self._entry_info(s, "Developer", DEVELOPER)
        self._entry_info(s, "Genre", GENRE)
        self._entry_info(s, "Players", PLAYERS)
        self._entry_info(s, "Score", SCORE)

        if len(s) == 2: # No label for just one line
            return s[1].strip()

        str = ''
        for a in range(0, int(len(s) / 2)):
            str = str + s[a * 2] + s[a * 2 + 1] + "\n"

        return str.strip()

class GameDatabase:
    _systems = {}
    _roms = {}
    _titles = {}

    def _scan_pics(self, dir, ext):
        pics = {}
        for fname in os.listdir(dir):
            if fname.endswith("." + ext):
                base = os.path.splitext(fname)[0]
                pics[base] = fname
        return pics

    def __init__(self, dirname, picture_dir, syspic_dir, ext):
        self._pictures = self._scan_pics(picture_dir, ext)
        self._sys_pictures = self._scan_pics(syspic_dir, ext)
        self._scandir(dirname)
        self._active_curpath = ''
        gd = GameData("MENU", "MENU>MENU>MENU")
        gd.set_picture("MENU." + ext)
        self._add(gd)

    def _add(self, gd):
        self._systems[gd.system()] = gd.system()
        self._roms[gd._field(ROM)] = gd
        self._titles[gd._field(TITLE)] = gd
        if gd._field(ROM) in self._pictures:
            gd.set_picture(self._pictures[gd._field(ROM)])

    def _add_file(self, dirname, filename):
        with open(dirname + "/" + filename, 'r') as fp:
            lines = fp.readlines()
            base = os.path.splitext(filename)[0]
            for line in lines:
                gd = GameData(base, line.strip())
                self._add(gd)

    def _scandir(self, dirname):
        for fname in os.listdir(dirname):
            if fname.endswith(".csv"):
                self._add_file(dirname, fname)
        # Always use the MISTER rbfs as the priority
        self._add_file(dirname, "MISTER_rbfs.csv")

    def _decode(self, s):
        return s.replace("%20", " ").replace("%2C", ",").strip()

    def _title_from_startpath(self, sp):
        return os.path.splitext(os.path.basename(os.path.normpath(sp)))[0]

    def lookup_system_pic(self, sys):
        if sys in self._sys_pictures:
            return self._sys_pictures[sys]
        return None

    def _extract_name(self, strs, indx):
        if len(strs) >= indx + 1:
            return self._decode(strs[indx])
        return ''

    def _lookup_title_variants(self, title):
        # Keep trying shortened versions of the title
        tit = title.split()
        title_lookup = None
        for w in range(len(tit) - 1, 0, -1):
            t = ' '.join(tit[0:w])
            log("Trying title '{}'".format(t))
            look = None
            if t in self._titles:
                look = self._titles[t]
            if look:
                if look.picture:
                    title_lookup = look
                    break
                elif not title_lookup:
                    title_lookup = look
        return title_lookup

    def lookup(self, s):
        strs = s.split(",")
        corename  = self._extract_name(strs, 1)
        fullpath  = self._extract_name(strs, 2)
        curpath   = self._extract_name(strs, 3)
        startpath = self._extract_name(strs, 4)
        select    = self._extract_name(strs, 5)
        samgame   = self._extract_name(strs, 6)

        log("corename = {}, fullpath = {}, curpath = {}, startpath = {}, select = {}, samgame = {}".
             format(corename, fullpath, curpath, startpath, select, samgame))

        if curpath.endswith(".mra") or \
           curpath.endswith(".rbf") or \
           curpath.endswith(".nes"):
            curpath = os.path.splitext(curpath)[0]

        core, title, alt_title, system = '', '', '', ''

        if startpath == '/tmp/SAM_game.mgl' and select != 'active':
            # SAM is active : use corename & samgame
            system = "arcade" if corename in self._roms else find_system(corename)
            core = corename
            title = samgame
        elif select == 'active' or select == 'selected':
            # Menu is active, use curpath & fullpath
            core = ''
            title = curpath
            system = "arcade" if "arcade" in fullpath.lower() else find_system(fullpath.lower())
 #       elif select == 'selected':
 #           # Game is active, use corename & startpath and if needed curpath
 #           core = corename
 #           title = self._title_from_startpath(startpath)
 #           alt_title = curpath
 #           self._active_curpath = curpath
 #           if "arcade" in startpath.lower() or "arcade" in fullpath.lower():
 #               system = "arcade"
 #           else:
 #               system = find_system(corename)
        elif select == 'cancelled':
            # Game is active, use corename & startpath and if needed _active_curpath
            system = "arcade" if corename in self._roms else find_system(corename)
            core = corename
            title = self._title_from_startpath(startpath)
            alt_title = self._active_curpath

        log("System = '{}', Core = '{}', Title = '{}', AltTitle = '{}'".format(system, core, title, alt_title))

        core_lookup, title_lookup, alt_title_lookup = None, None, None
        if core and core != "MENU" and core in self._roms:
            core_lookup = self._roms[core]
        if title and title in self._titles:
            title_lookup = self._titles[title]
        if alt_title and alt_title in self._titles:
            alt_title_lookup = self._titles[alt_title]

        if not title_lookup and not alt_title_lookup and (title or alt_title):
            if title:
                title_lookup = self._lookup_title_variants(title)
            if not title_lookup and alt_title:
                alt_title_lookup = self._lookup_title_variants(alt_title)

        if core_lookup:
            log("Core  Lookup = \n{}".format(core_lookup.info_str()))
        if title_lookup:
            log("Title Lookup = \n{}".format(title_lookup.info_str()))
        if alt_title_lookup:
            log("ALT   Lookup = \n{}".format(alt_title_lookup.info_str()))

        if core_lookup:
            return core_lookup, system
        if title_lookup:
            return title_lookup, system
        if alt_title_lookup:
            return alt_title_lookup, system

        # Make a db entry from the lookup key
        log("FALLBACK '{}' '{}' '{}'".format(system, corename, title))
        gd = GameData(system, corename + ">" + title + ">")
        return gd, system
