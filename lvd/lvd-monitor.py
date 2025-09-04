import gpiod
import os
import logging
import configparser
import signal
import sys

G_TRIGG_SCIRPT = "/usr/sbin/lvd-callback.sh"
G_CONF_INI = "/etc/lvd/config.ini"

logging.basicConfig(level=logging.INFO, format='%(levelname)s - %(message)s')

class GPIO_ext:

    def __init__(self):
        self._chip = None
        self._line = None

    def find_ini(self):
        if os.path.exists(G_CONF_INI):
            config = configparser.ConfigParser()
            config.read(G_CONF_INI)
            chip_name = config.get('gpio', 'gpiochip', fallback="").strip()
            offset = config.get('gpio', 'line', fallback="").strip()
            chip_path = None

            if len(chip_name) == 0 or len(offset) == 0:
                return
            if chip_name.isdigit():
                chip_path = f"/dev/gpiochip{chip_name}"
            elif chip_name.startswith("/dev/"):
                chip_path=chip_name
            else:
                chip_path = f"/dev/{chip_name}"

            logging.info(f"config.ini: {chip_name}-{offset}.")

            if not os.path.exists(chip_path):
                logging.error(f"{chip_path} not exist.")
                return
            if not offset.isdigit():
                logging.error(f"line must be a number.[{offset}]")
                return
            try:
                self._chip = gpiod.Chip(chip_path)
                self._line = self._chip.get_line(int(offset))
            except Exception as e:
                logging.error(f"Control {chip_name}-{offset} failed.{str(e)}")
        else:
            logging.info(f"{G_CONF_INI} not exist.")

    def find_lvd(self):
        logging.info("Auto find LVD...")
        self._line = gpiod.find_line("LVD")

    def get_line(self):
        if self._line is None:
            self.find_ini()
        if self._line is None:
            self.find_lvd()
        return self._line

    def close(self):
        if self._line:
            self._line.release()
            self._line = None
        if self._chip:
            self._chip.close()
            self._chip = None


gpio_ext = GPIO_ext()

def sigint_handler(signum, frame):
    gpio_ext.close()
    sys.exit(0)

signal.signal(signal.SIGINT, sigint_handler)
signal.signal(signal.SIGTERM, sigint_handler)

def trigger_callback(v: str):
    os.system(f"{G_TRIGG_SCIRPT} {v}")


def edge_event_callback(event):
    if event.type == gpiod.LineEvent.RISING_EDGE:
        logging.info("Detected rising edge")
        trigger_callback("rising")
    elif event.type == gpiod.LineEvent.FALLING_EDGE:
        logging.info("Detected falling edge")
        trigger_callback("falling")

def daemon():
    line = gpio_ext.get_line()
    try:
        if line is None:
            logging.error("Not found LVD pin.")
            return
        try:
            line.request(consumer='edge_detector', type=gpiod.LINE_REQ_EV_BOTH_EDGES)
            logging.info("Start monitoring LVD...")
            while True:
                if line.event_wait(sec=1):
                    event = line.event_read()
                    edge_event_callback(event)
        except KeyboardInterrupt:
            logging.info("\nexit")
        except Exception as e:
            logging.error(str(e))
    finally:
        gpio_ext.close()

if __name__ == "__main__":
    daemon()