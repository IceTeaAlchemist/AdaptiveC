#!/usr/bin/env python3

# 20211124 weg, started looking at interrupts on 
# https://roboticsbackend.com/raspberry-pi-gpio-interrupts-tutorial/


import time, datetime
import RPi.GPIO as GPIO

BUTTON_GPIO = 16

if __name__ == '__main__':
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(BUTTON_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    try:

        while True:
            # Below blocks the whole thread it is launched.
            GPIO.wait_for_edge(BUTTON_GPIO, GPIO.FALLING)
            print("Button pressed!\t{}".format(datetime.datetime.now() ))

    except KeyboardInterrrupt:

        print('Caught a KeyboardInterrupt, quitting.')

    print('Outside the main loop, bye.')