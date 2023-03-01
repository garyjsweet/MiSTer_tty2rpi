
from PIL import Image, ImageDraw, ImageFont
import PIL
import sys
import subprocess
import time
import inotify.adapters
import os.path
import threading
import logging
import socket

BASIC_FONT_SIZE = 20
MAX_FONT_SIZE   = 70

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

font = ImageFont.truetype('/usr/share/fonts/truetype/freefont/FreeSansBoldOblique', BASIC_FONT_SIZE)

logging.basicConfig(filename='/home/pi/display.log', level=logging.INFO, filemode='w', encoding='utf-8')

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

def info(*args):
    print(*args)
    logging.info(*args)

class GameData:
    _picture = ''

    def __init__(self, system, row):
        self._system  = find_system(system)
        self._entries = row.split('>')
        while len(self._entries) < NUM_ENTRIES:
            self._entries.append('')
        for i in range(0, NUM_ENTRIES - 1):
            self._entries[i] = self._entries[i].strip()

    def field(self, f):
        return self._entries[f]

    def system(self):
        return self._system

    def set_picture(self, p):
        self._picture = p

    def picture(self):
        return self._picture

    def system_picture(self):
        global sys_pics
        print("System = {}".format(self.system()))
        if self.system() in sys_pics:
            return sys_pics[self.system()]
        else:
            return ''

    def info(self):
        info(self._system + ': ' + '>'.join(self._entries))

    def info_str(self):
        s = []
        if len(self._entries) >= TITLE:
            s.append("Title: ")
            s.append(self.field(TITLE))
        if len(self._entries) >= YEAR and self.field(YEAR):
            s.append("Year: ")
            s.append( self.field(YEAR))
        if len(self._entries) >= DEVELOPER and self.field(DEVELOPER):
            s.append("Developer: ")
            s.append(self.field(DEVELOPER))
        if len(self._entries) >= GENRE and self.field(GENRE):
            s.append("Genre: ")
            s.append(self.field(GENRE))
        if len(self._entries) >= PLAYERS and self.field(PLAYERS):
            s.append("Players: ")
            s.append(self.field(PLAYERS))
        if len(self._entries) >= SCORE and self.field(SCORE):
            s.append("Score: ")
            s.append(self.field(SCORE))

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
    _pictures = {}

    def __init__(self, dirname, pictures):
        self._pictures = pictures
        self.scandir(dirname)
        self._active_curpath = ''
        gd = GameData("MENU", "MENU>MENU>MENU")
        gd.set_picture("MENU.png")
        self._add(gd)

    def _add(self, gd):
        self._systems[gd.system()] = gd.system()
        self._roms[gd.field(ROM)] = gd
        self._titles[gd.field(TITLE)] = gd
        if gd.field(ROM) in pictures:
            gd.set_picture(pictures[gd.field(ROM)])

    def add_file(self, dirname, filename):
        with open(dirname + "/" + filename, 'r') as fp:
            lines = fp.readlines()
            base = os.path.splitext(filename)[0]
            for line in lines:
                gd = GameData(base, line.strip())
                self._add(gd)

    def scandir(self, dirname):
        for fname in os.listdir(dirname):
            if fname.endswith(".csv"):
                self.add_file(dirname, fname)
        # Always use the MISTER rbfs as the priority
        self.add_file(dirname, "MISTER_rbfs.csv")

    def systems(self):
        return self._systems

    def roms(self):
        return self._roms

    def titles(self):
        return self._titles

    def _decode(self, s):
        return s.replace("%20", " ").replace("%2C", ",").strip()

    def title_from_startpath(self, sp):
        return os.path.splitext(os.path.basename(os.path.normpath(sp)))[0]

    def lookup(self, s):
        strs = s.split(",")
        corename, fullpath, curpath, startpath, select, samgame = '', '', '', '', '', ''
        if len(strs) >= 2:
            corename = self._decode(strs[1])
        if len(strs) >= 3:
            fullpath = self._decode(strs[2])
        if len(strs) >= 4:
            curpath = self._decode(strs[3])
        if len(strs) >= 5:
            startpath = self._decode(strs[4])
        if len(strs) >= 6:
            select = self._decode(strs[5])
        if len(strs) >= 7:
            samgame = self._decode(strs[6])
        info("corename = {}, fullpath = {}, curpath = {}, startpath = {}, select = {}, samgame = {}".
             format(corename, fullpath, curpath, startpath, select, samgame))

        if curpath.endswith(".mra") or \
           curpath.endswith(".rbf") or \
           curpath.endswith(".nes"):
            curpath = os.path.splitext(curpath)[0]

        core, title, alt_title, system = '', '', '', ''

        if startpath == '/tmp/SAM_game.mgl' and select != 'active':
            # SAM is active : use corename & samgame
            if corename in database.roms():
                system = "arcade"
            else:
                system = find_system(corename)
            core = corename
            title = samgame
        elif select == 'active':
            # Menu is active, use curpath & fullpath
            core = ''
            title = curpath
            if "arcade" in fullpath.lower():
                system = "arcade"
            else:
                print("FIND SYS : {}".format(fullpath.lower()))
                system = find_system(fullpath.lower())
        elif select == 'selected':
            # Game is active, use corename & startpath and if needed curpath
            core = corename
            title = self.title_from_startpath(startpath)
            alt_title = curpath
            self._active_curpath = curpath
            if "arcade" in startpath.lower() or "arcade" in fullpath.lower():
                system = "arcade"
            else:
                system = find_system(corename)
        elif select == 'cancelled':
            # Game is active, use corename & startpath and if needed _active_curpath
            if corename in database.roms():
                system = "arcade"
            else:
                system = find_system(corename)
            core = corename
            title = self.title_from_startpath(startpath)
            alt_title = self._active_curpath

        info("System = '{}', Core = '{}', Title = '{}', AltTitle = '{}'".format(system, core, title, alt_title))

        core_lookup, title_lookup, alt_title_lookup = None, None, None
        if core and core != "MENU" and core in self._roms:
            core_lookup = self._roms[core]
        if title and title in self._titles:
            title_lookup = self._titles[title]
        if alt_title and alt_title in self._titles:
            alt_title_lookup = self._titles[alt_title]

        # Keep trying shortened versions of the title
        if not title_lookup and not alt_title_lookup and (title or alt_title):
            if title:
                tit = title.split()
                best = None
                for w in range(len(tit) - 1, 0, -1):
                    t = ' '.join(tit[0:w])
                    print("Trying '{}'".format(t))
                    look = None
                    if t in self._titles:
                        look = self._titles[t]
                    if look:
                        if look.picture:
                            title_lookup = look
                            break
                        elif not title_lookup:
                            title_lookup = look

            if not title_lookup and alt_title:
                tit = alt_title.split()
                for w in range(len(tit) - 1, 0, -1):
                    t = ' '.join(tit[0:w])
                    print("Trying '{}'".format(t))
                    look = None
                    if t in self._titles:
                        look = self._titles[t]
                    if look:
                        if look.picture:
                            alt_title_lookup = look
                            break
                        elif not title_lookup:
                            alt_title_lookup = look

        if core_lookup:
            info("Core  Lookup = {}".format(core_lookup.info_str()))
        if title_lookup:
            info("Title Lookup = {}".format(title_lookup.info_str()))
        if alt_title_lookup:
            info("ALT   Lookup = {}".format(alt_title_lookup.info_str()))

        if core_lookup:
            return core_lookup, system
        if title_lookup:
            return title_lookup, system
        if alt_title_lookup:
            return alt_title_lookup, system

        # Make a db entry from the lookup key
        print("FALLBACK '{}' '{}' '{}'".format(system, corename, title))
        gd = GameData(system, corename + ">" + title + ">")
        return gd, system

class Rectangle:
    def __init__(self, x, y, w, h):
        self.x = x
        self.y = y
        self.w = w
        self.h = h

    def fromImg(self, img):
        self.x = 0
        self.y = 0
        self.w = img.width
        self.h = img.height

    def __str__(self):
        return  "({0}, {1}, {2}, {3})".format(self.x, self.y, self.w, self.h)

    def from_percent(self, pix_w, pix_h):
        return Rectangle(self.x * pix_w / 100,
                         self.y * pix_h / 100,
                         self.w * pix_w / 100,
                         self.h * pix_h / 100)

    def fitting_size(self, container):
        fit = Rectangle(0, 0, 0, 0)
        aspect = self.w / self.h
        container_aspect = container.w / container.h
        if aspect > container_aspect: # Fit width
            fit.x = container.x
            fit.w = container.w
            fit.h = fit.w / aspect
            vertical_space = container.h - fit.h
            fit.y = container.y + (vertical_space / 2)
        else:                         # Fit height
            fit.y = container.y
            fit.h = container.h
            fit.w = fit.h * aspect
            horizontal_space = container.w - fit.w
            fit.x = container.x + (horizontal_space / 2)
        return fit

def calc_font(rw, rh, text, font, anchor=None, align='left'):
    img = Image.new("RGB", (1, 1))
    draw = ImageDraw.Draw(img)
    f = font.font_variant(size=BASIC_FONT_SIZE)
    (l,t,r,b) = draw.multiline_textbbox((0, 0), text, font=f, anchor=anchor, align=align)
    w = abs(r - l)
    h = abs(t - b)
    scale = 1
    if h != 0 and rh != 0:
        if w / h > rw / rh:
            scale = rw / w
        else:
            scale = rh / h
    size = min(MAX_FONT_SIZE, int(BASIC_FONT_SIZE * scale))
    info("Font size = {}".format(size))
    return font.font_variant(size=size)

class Text:
    def __init__(self, origin, text, fill, font, align, anchor, bgCol = None):
        self._origin = origin
        self._text   = text
        self._fill   = fill
        self._font   = font
        self._align  = align
        self._anchor = anchor
        self._bgCol  = bgCol

    def draw(self, img):
        draw = ImageDraw.Draw(img, 'RGBA')
        bord = 10
        if self._bgCol != None:
            (l,t,r,b) = draw.multiline_textbbox(self._origin, self._text, font=self._font, align=self._align, anchor=self._anchor)
            draw.rectangle((l - bord, t - bord, r + bord, b + bord), fill=self._bgCol)
        draw.text(self._origin, self._text, fill=self._fill, font=self._font, align=self._align, anchor=self._anchor)

class BuilderImg:
    def __init__(self, r, img):
        self.rect = r
        self.img = img

class ImageBuilder:
    def __init__(self, w, h, color = (0, 0, 0, 255)):
        self._images = []
        self._texts  = []
        self._w = w
        self._h = h
        self._img = Image.new("RGB", (w, h), color)

    def clear(self, colour):
        self._img = Image.new("RGB", (w, h), color)

    def add_layout_image(self, rect, img):
        self._images.append(BuilderImg(rect, img))

    def add_text(self, origin, text, fill, font, align, anchor, bgCol = None):
        self._texts.append(Text(origin, text, fill, font, align, anchor, bgCol))

    def get_result(self):
        final_aspect = self._w / self._h
        aspect_sum = 0

        for img in self._images:
            imgR = Rectangle(0, 0, 0, 0)
            imgR.fromImg(img.img)
            fit = imgR.fitting_size(img.rect)
            i = img.img.resize((int(fit.w), int(fit.h)), PIL.Image.BILINEAR)
            self._img.paste(i, (int(fit.x), int(fit.y)))

        for txt in self._texts:
            txt.draw(self._img)

        return self._img

def showImg():
    return subprocess.Popen(['feh', '--quiet', '--fullscreen', '/dev/shm/test.ppm'])

def kill_old_show(kill_proc):
    time.sleep(4)
    kill_proc.terminate()
    kill_proc.wait() # Ensure no defunct processes are left

socket_data = ''
def socket_reader(event_obj):
    global socket_data
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    port = 6666

    info("Socket successfully created")

    s.bind(('', port))
    info("socket bound to %s" %(port))

    s.listen(5)
    info("socket is listening")

    while True:
        c, addr = s.accept()
        data = c.recv(4096).decode()
        if data:
            socket_data = data
            event_obj.set()
            #info("Received {}".format(data))

pictures = {}

info("Scanning pictures ...")
for fname in os.listdir("/home/pi/marquee-pictures"):
    if fname.endswith(".png"):
        base = os.path.splitext(fname)[0]
        pictures[base] = fname
for fname in os.listdir("/home/pi/marquee-pictures/systems"):
    if fname.endswith(".png"):
        base = os.path.splitext(fname)[0]
        sys_pics[base] = fname
info("... done")
info("Found {} pictures".format(len(pictures)))

info("Loading game database ...")
database = GameDatabase("/home/pi/database", pictures)
info("... done")

info("Loaded {} systems, {} roms".format(len(database.systems()), len(database.roms())))

# Make an event for cross-thread waiting
event_obj = threading.Event()
event_obj.clear()

# Create socket reading thread
thread = threading.Thread(target=socket_reader, args=(event_obj,))
thread.start()

old_data = ''
old_proc = None
first_loop = True

while True:
    data = socket_data

    if data != old_data and data:
        info("data = {}".format(data))
        info("old_data = {}".format(old_data))

        old_data = data

        entries = data.split(',')
        command = ''
        if len(entries) > 0:
            command = entries[0]

        info("Command = {}".format(command))

        pic_name = "MENU.png"
        if command == 'CMDCOREXTRA':
            start_time = time.time()
            gd, system = database.lookup(data)
            end_time = time.time()
            info("Lookup took: {}".format(end_time-start_time))
            pic_name = gd.picture()
            if system in sys_pics:
                sys_pic_name = "systems/" + sys_pics[system]
            else:
                sys_pic_name = "systems/" + gd.system_picture()
            gd.info()
        else:
            info("UNKNOWN COMMAND {}".format(command))
            continue

        if gd:
            display_str = gd.info_str()
        elif len(entries) > 1:
            display_str = entries[1]
        else:
            display_str = entries[1]

        file = '/home/pi/marquee-pictures/' + pic_name
        sys_file = '/home/pi/marquee-pictures/' + sys_pic_name

        builder = ImageBuilder(1920, 480)

        has_sys = False
        need_text = True
        rect = Rectangle(0, 0, 1920, 480)
        if os.path.isfile(sys_file) and (not os.path.isfile(file) or gd.system() != 'arcade'):
            img = Image.open(sys_file)
            builder.add_layout_image(Rectangle(0, 0, 480, 480), img)
            rect = Rectangle(480, 0, 1920 - 480, 480)
            has_sys = True

        if os.path.isfile(file):
            img = Image.open(file)
            builder.add_layout_image(rect, img)
            need_text = False
        elif not has_sys:
            maybe_just_sys = find_system(display_str)
            if maybe_just_sys in sys_pics:
                img = Image.open('/home/pi/marquee-pictures/systems/' + sys_pics[maybe_just_sys])
                builder.add_layout_image(rect, img)
                need_text = False

        if need_text:
            print("Using string : {}".format(display_str))
            align = "left"
            anchor = "mm"
            scaled_font = calc_font(rect.w, rect.h, display_str, font, anchor=anchor, align=align)
            builder.add_text((rect.x + rect.w / 2, rect.h / 2), display_str, "white", font=scaled_font,
                            align=align, anchor=anchor, bgCol=(0, 0, 0, 160))

        start_time = time.time()
        resimg = builder.get_result()
        end_time = time.time()
        info("Builder took: {}".format(end_time-start_time))

        start_time = time.time()
        resimg.save("/dev/shm/test.ppm", compress_level=0)
        end_time = time.time()
        info("Save took: {}".format(end_time-start_time))

        proc = showImg()

        builder = None

        if old_proc != None:
            threading.Thread(target=kill_old_show, args=(old_proc,)).start()
        old_proc = proc

    event_obj.wait(5)
    event_obj.clear()
