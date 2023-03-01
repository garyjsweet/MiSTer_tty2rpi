from PIL import Image, ImageFont
import subprocess
import time
import os.path
import threading
import socket
from game_database import *
from log import log
from rectangle import Rectangle
from text import *
from image_builder import ImageBuilder

if __name__ != "__main__":
    exit

#PI_FOLDER = os.path.abspath("/home/pi")
PI_FOLDER  = os.path.abspath("..")

TMP_IMAGE = "/dev/shm/tty2rpi.ppm"

PORT = 6666

DISPLAY_W = 1920
DISPLAY_H = 480

SYS_PIC_W = 480
SYS_PIC_H = 480

def showImg():
    return subprocess.Popen(['feh', '--quiet', '--fullscreen', TMP_IMAGE])

def kill_old_show(kill_proc):
    time.sleep(4)
    kill_proc.terminate()
    kill_proc.wait() # Ensure no defunct processes are left

socket_data = ''

def socket_reader(event_obj):
    global socket_data
    global PORT
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    log("Socket successfully created")

    s.bind(('', PORT))
    log("socket bound to %s" %(PORT))

    s.listen(5)
    log("socket is listening")

    while True:
        c, addr = s.accept()
        data = c.recv(4096).decode()
        if data:
            socket_data = data
            event_obj.set()

def startup_screen(message):
    old_proc = None
    startup_builder = ImageBuilder(DISPLAY_W, DISPLAY_H)
    startup_img_file = PI_FOLDER + "/marquee-pictures/STARTUP.png"
    if os.path.isfile(startup_img_file):
        rect = Rectangle(0, 0, DISPLAY_W, DISPLAY_H - 80)
        startup_img = Image.open(startup_img_file)
        startup_builder.add_layout_image(rect, startup_img)
        startup_builder.add_text(rect, (rect.w / 2, DISPLAY_H - 10),
                                message, "white", font=font, align="left",
                                anchor="md",  bgCol=(0, 0, 0, 160))
        resimg = startup_builder.get_result()
        resimg.save(TMP_IMAGE, compress_level=0)
        old_proc = showImg()
    return old_proc

old_proc = startup_screen("Loading game and image database ...")

log("Loading game and image database ...")
database = GameDatabase(PI_FOLDER + "/database", PI_FOLDER + "/marquee-pictures",
                        PI_FOLDER + "/marquee-pictures/systems")
log("... done")

if old_proc != None:
    threading.Thread(target=kill_old_show, args=(old_proc,)).start()
old_proc = startup_screen("Databases loaded")

# Make an event for cross-thread waiting
event_obj = threading.Event()
event_obj.clear()

# Create socket reading thread
thread = threading.Thread(target=socket_reader, args=(event_obj,))
thread.start()

old_data = ''
first_loop = True

while True:
    data = socket_data

    if data != old_data and data:
        log("data = {}".format(data))
        old_data = data

        entries = data.split(',')
        command = ''
        if len(entries) > 0:
            command = entries[0]

        log("Command = {}".format(command))

        pic_name = "MENU.png"
        if command == 'CMDCOREXTRA':
            gd, system = database.lookup(data)
            pic_name = gd.picture()
            sys_pic_name = database.lookup_system_pic(system)
            if not sys_pic_name:
                sys_pic_name = gd.system_picture()
            gd.log()
        else:
            log("UNKNOWN COMMAND {}".format(command))
            continue

        if gd:
            display_str = gd.info_str()
        elif len(entries) > 1:
            display_str = entries[1]
        else:
            display_str = entries[1]

        file = PI_FOLDER + '/marquee-pictures/' + pic_name
        sys_file = PI_FOLDER + '/marquee-pictures/systems/' + sys_pic_name

        if socket_data != data:
            continue

        builder = ImageBuilder(DISPLAY_W, DISPLAY_H)

        has_sys = False
        need_text = True
        rect = Rectangle(0, 0, DISPLAY_W, DISPLAY_H)
        if os.path.isfile(sys_file) and (not os.path.isfile(file) or gd.system() != 'arcade'):
            img = Image.open(sys_file)
            builder.add_layout_image(Rectangle(0, 0, SYS_PIC_W, SYS_PIC_H), img)
            rect = Rectangle(SYS_PIC_W, 0, DISPLAY_W - SYS_PIC_W, DISPLAY_H)
            has_sys = True

        if socket_data != data:
            continue

        if os.path.isfile(file):
            img = Image.open(file)
            builder.add_layout_image(rect, img)
            need_text = False
        elif not has_sys:
            sys_pic_name = database.lookup_system_pic(find_system(display_str))
            if sys_pic_name:
                img = Image.open(PI_FOLDER + '/marquee-pictures/systems/' + sys_pic_name)
                builder.add_layout_image(rect, img)
                need_text = False

        if socket_data != data:
            continue

        if need_text:
            display_str = display_str.strip("_").strip("@").strip("_")
            log("Using string : {}".format(display_str))
            builder.add_text(rect, (rect.x + rect.w / 2, rect.h / 2), display_str,
                             "white", font=font, align="left", anchor="mm")

        if socket_data != data:
            continue

        start_time = time.time()
        resimg = builder.get_result()
        end_time = time.time()
        log("Builder took: {}".format(end_time-start_time))

        start_time = time.time()
        resimg.save(TMP_IMAGE, compress_level=0)
        end_time = time.time()
        log("Save took: {}".format(end_time-start_time))

        if socket_data != data:
            continue

        proc = showImg()

        builder = None

        if old_proc != None:
            threading.Thread(target=kill_old_show, args=(old_proc,)).start()
        old_proc = proc

    event_obj.wait(5)
    event_obj.clear()
