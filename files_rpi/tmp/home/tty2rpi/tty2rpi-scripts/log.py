import logging

LOG_FILE = "/tmp/display.log"

logging.basicConfig(filename=LOG_FILE, level=logging.INFO, filemode='w')

def log(s):
    print(s)
    logging.info(s)
