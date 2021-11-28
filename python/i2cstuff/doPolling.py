#!/usr/bin/env python3
# from https://roboticsbackend.com/raspberry-pi-gpio-interrupts-tutorial/
# 20211124 weg, started as tutorial on polling vs interrupt requests on RPi.

import time, datetime
import RPi.GPIO as GPIO

BUTTON_GPIO = 16

if __name__ == '__main__':
    GPIO.setmode(GPIO.BCM)  # vs GPIO.BOARD, BCM is Broadcom SOC channel
                            # number, i.e. next to the GPIO label.
    GPIO.setup(BUTTON_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    pressed = False

    while True:
        try:
            # button is pressed when pin is LOW
            if not GPIO.input(BUTTON_GPIO):
                if not pressed:
                    print("Button pressed!\t{}".format(datetime.datetime.now() ))
                    pressed = True
            # button not pressed (or released)
            else:
                pressed = False
            time.sleep(0.1)
        except KeyboardInterrupt:
            print('Caught a KeyboardInterrupt, quitting.')
            break

    print('Outside the main loop, bye.')