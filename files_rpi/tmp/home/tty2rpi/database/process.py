from PIL import Image, ImageDraw, ImageFont
import PIL
import sys
import subprocess
import time
import os.path
import threading
from fuzzywuzzy import fuzz
from fuzzywuzzy import process

DATAFILE = '/dev/shm/corename'

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
    def __init__(self, system, row):
        if '\n' in row:
            print(row)
            quit()
        self._entries = row.split('>')
        while len(self._entries) < NUM_ENTRIES:
            self._entries.append('')

    def field(self, f):
        return self._entries[f]

    def write(self, f):
        f.write('>'.join(self._entries) + "\n")

    def print(self):
        print('>'.join(self._entries))

class GameDatabase:
    _systems = {}

    def __init__(self, dirname):
        self.scandir(dirname)

    def add_file(self, dirname, filename):
        with open(dirname + "/" + filename, 'r') as fp:
            lines = fp.readlines()
            base = os.path.splitext(filename)[0]
            self._systems[base] = {}
            d = self._systems[base]
            for line in lines:
                gd = GameData(base, line.strip())
                rom = gd.field(ROM)
                if rom in d:
                    print("Clash ", rom, " ", base)
                else:
                    d[rom] = gd

    def scandir(self, dirname):
        for fname in os.listdir(dirname):
            if fname.endswith(".csv"):
                self.add_file(dirname, fname)

    def write(self, f):
        for sys in self._systems:
            self.write(sys)

    def write(self, f, system):
        for rom in self._systems[system]:
            self._systems[system][rom].write(f)

    def merge(self, systems, newsys):
        if not newsys in self._systems:
            self._systems[newsys] = {}
        ns = self._systems[newsys]
        for s in systems:
            for rom in self._systems[s]:
                if rom in ns:
                    print("Clash ", rom)
                else:
                    ns[rom] = self._systems[s][rom]
                    ns[rom].print()



print("Loading game database ...")
database = GameDatabase(".")
print("... done")

#database.merge(['mame174', 'Atari Classics', 'MAME_v1', 'Capcom Classics',
#               'Capcom Play System II', 'Capcom Play System III',
#                'Capcom Play System', 'Data East Classics',
#                'Irem Classics', 'Konami Classics', 'Konami e-AMUSEMENT',
#                'MiSFiT MAME', 'Midway Classics', 'Namco Classics',
#                'Namco System 22', 'Taito Classics', 'Taito Type X',
#                'Williams Classics', 'Sega Classics', 'hbmame'], "Arcade")


#with open("test.db", 'w') as fp:
#    database.write(fp, "Arcade")
    