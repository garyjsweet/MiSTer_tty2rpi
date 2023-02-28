
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
        self._system  = system
        self._entries = row.split('>')
        while len(self._entries) < NUM_ENTRIES:
            self._entries.append('')

    def field(self, f):
        return self._entries[f]

    def system(self):
        return self._system

    def set_picture(self, p):
        self._picture = p

    def picture(self):
        return self._picture

    def info(self):
        info(self._system + ' ' + '>'.join(self._entries))

class GameDatabase:
    _systems = {}
    _roms = {}
    _titles = {}
    _pictures = {}

    def __init__(self, dirname, pictures):
        self._pictures = pictures
        self.scandir(dirname)
        gd = GameData("MENU", "MENU>MENU>MENU")
        gd.set_picture("MENU.png")
        self._add(gd)

    def _add(self, gd):
        self._systems[gd.system()] = gd
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

    def systems(self):
        return self._systems

    def roms(self):
        return self._roms

    def titles(self):
        return self._titles

    def lookup(self, s):
        strs = s.split(",")
        corename, fullpath, curpath, startpath = '', '', '', ''
        if len(strs) >= 2:
            corename = strs[1].replace("%20", " ").replace("%2C", ",")
        if len(strs) >= 3:
            fullpath = strs[2].replace("%20", " ").replace("%2C", ",")
        if len(strs) >= 4:
            curpath = strs[3].replace("%20", " ").replace("%2C", ",")
        if len(strs) >= 5:
            startpath = strs[4].replace("%20", " ").replace("%2C", ",")
        info("corename = {}, fullpath = {}, curpath = {}, startpath = {}".format(corename, fullpath, curpath, startpath))

        core_lookup, cur_lookup = '', ''
        if corename != '' and corename != "MENU" and corename in self._roms:
            core_lookup = self._roms[corename]
        if curpath != '' and curpath in self._titles:
            cur_lookup = self._titles[curpath]

        info("Core Lookup = {}".format(str(core_lookup)))
        info("Curr Lookup = {}".format(str(cur_lookup)))

        if core_lookup != '':
            return core_lookup
        if cur_lookup != '':
            return cur_lookup

        #if s in self._systems:
        #   return self._systems[s]
        return None

class Rectangle:
    def __init__(self, x, y, w, h):
        self.x = x
        self.y = y
        self.w = w
        self.h = h

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

class ImageBuilder:
    def __init__(self, w, h, color = (0, 0, 0, 255)):
        self._images = []
        self._texts  = []
        self._w = w
        self._h = h
        self._img = Image.new("RGB", (w, h), color)

    def clear(self, colour):
        self._img = Image.new("RGB", (w, h), color)

    def add_layout_image(self, img):
        self._images.append(img)

    def add_text(self, origin, text, fill, font, align, anchor, bgCol = None):
        self._texts.append(Text(origin, text, fill, font, align, anchor, bgCol))

    def get_result(self):
        final_aspect = self._w / self._h
        aspect_sum = 0

        for img in self._images:
            aspect = img.width / img.height
            aspect_sum += aspect

        if aspect_sum > final_aspect:
            # Images are too wide, so no horizontal fillers needed
            xpos = 0
            for img in self._images:
                aspect = img.width / img.height
                x = self._w * aspect / aspect_sum
                y = x / aspect

                i = img.resize((int(x), int(y)), PIL.Image.BILINEAR)
                extra_h = self._h - i.height
                self._img.paste(i, (int(xpos), int(extra_h / 2)))
                xpos += i.width
        else:
            # Fillers needed between images and at edges
            filler = self._w * ((final_aspect - aspect_sum) / final_aspect) / (len(self._images) + 1)
            xpos = filler
            for img in self._images:
                scale = self._h / img.height
                i = img.resize((int(img.width * scale), int(img.height * scale)),
                                PIL.Image.BILINEAR)
                self._img.paste(i, (int(xpos), int(0)))
                xpos += i.width + filler

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
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
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
            info("Received {}".format(data))

pictures = {}
info("Scanning pictures ...")
for fname in os.listdir("/home/pi/marquee-pictures"):
    if fname.endswith(".png"):
        base = os.path.splitext(fname)[0]
        pictures[base] = fname
info("... done")
info("Found {} pictures".format(len(pictures)))

info("Loading game database ...")
database = GameDatabase("/home/pi/database", pictures)
info("... done")

info("Loaded {} systems, {} roms".format(len(database.systems()), len(database.roms())))

# Make an event for cross-thread waiting
event_obj = threading.Event()

# Create socket reading thread
thread = threading.Thread(target=socket_reader, args=(event_obj,))
thread.start()

old_data = ''
old_proc = None
first_loop = True

font = ImageFont.truetype('/usr/share/fonts/truetype/freefont/FreeSansBoldOblique', 34)
#font = ImageFont.load_default() # TODO

while True:
    data = socket_data

    if data != old_data and data != '':
        old_data = data

        entries = data.split(',')
        command = ''
        if len(entries) > 0:
            command = entries[0]

        print("Command = ", command)

        pic_name = "MENU.png"
        if command == 'CMDCOREXTRA':
            start_time = time.time()
            gd = database.lookup(data)
            end_time = time.time()
            info("Lookup took: {}".format(end_time-start_time))
            if gd:
                pic_name = gd.picture()
                gd.info()
        else:
            info("UNKNOWN COMMAND {}".format(command))
            pass

        file = '/home/pi/marquee-pictures/' + pic_name

        builder = ImageBuilder(1920, 480)

        if os.path.isfile(file):
            start_time = time.time()
            img = Image.open(file)
            end_time = time.time()
            info("Loader took: {}".format(end_time-start_time))
            builder.add_layout_image(img)
        else:
            builder.add_text((1920 / 2, 480 - 20), data, "white", font, align="left", anchor="md", bgCol=(0, 0, 0, 160))

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

    event_obj.wait(100)
    event_obj.clear()


## FILES WRITTEN TO /tmp FOR EVERY MENU FILE HIGHLIGHTED
##
## more /tmp/CURRENTPATH
## OutRun 2019 (Europe).md
##
## more /tmp/FULLPATH
## games/Genesis/@Genesis - MegaSD Mega EverDrive 2022-05-18.zip/2 Europe - A-Z/Europe - M-R

