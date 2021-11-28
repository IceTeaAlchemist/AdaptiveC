#!/usr/bin/env python3          

# 20211124 weg, started looking at interrupts and here callback functions on 
# https://roboticsbackend.com/raspberry-pi-gpio-interrupts-tutorial/

import time, datetime
import signal                   
import sys
import RPi.GPIO as GPIO

BUTTON_GPIO = 16

def signal_handler(sig, frame):
    GPIO.cleanup()
    sys.exit(0)

def button_callback(channel):
    if not GPIO.input(BUTTON_GPIO):
        print("Button pressed! \t{}".format(datetime.datetime.now() ))
    else:
        print("Button released!\t{}".format(datetime.datetime.now() ))

if __name__ == '__main__':
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(BUTTON_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    
    GPIO.add_event_detect(BUTTON_GPIO, GPIO.BOTH, 
            callback=button_callback, bouncetime=50)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.pause()