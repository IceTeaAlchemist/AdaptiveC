#!/usr/bin/env python3

# 20211124 weg, started looking at interrupts and here callback functions on 
# https://roboticsbackend.com/raspberry-pi-gpio-interrupts-tutorial/

import signal, time, datetime
import sys
import RPi.GPIO as GPIO

BUTTON_GPIO = 16

def signal_handler(sig, frame):
    GPIO.cleanup()
    sys.exit(0)

def button_pressed_callback(channel):
    print("Button pressed!\t{}".format(datetime.datetime.now() ) )

if __name__ == '__main__':

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(BUTTON_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    GPIO.add_event_detect(BUTTON_GPIO, GPIO.FALLING, 
            callback=button_pressed_callback, bouncetime=100)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.pause()

