#!/usr/bin/env python3
#
# 20211127 weg, started and want to read the two ADCs, 0x48 has the thermocouple on
#               channel A0 single ended.
# See page 16 of adafruit-4-channel-adc-breakouts.pdf .
# From newer CircuitPython libraries
# https://pypi.org/project/adafruit-circuitpython-ads1x15/ 

import time, datetime
import board
import busio
import adafruit_ads1x15.ads1015 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

# Create the I2C bus
i2c = busio.I2C(board.SCL, board.SDA)
print('board.SCL, board.SDA are {}'.format( (board.SCL, board.SDA) ) )

# Create the ADC object using the I2C bus
ads = ADS.ADS1015(i2c)

# Create single-ended input on channel 0
chan = AnalogIn(ads, ADS.P0)
chan1 = AnalogIn(ads, ADS.P1)

# Create differential input between channel 0 and 1
# chan = AnalogIn(ads, ADS.P0, ADS.P1)

print("{:>5}\t{:>5}\t{:>5}\t{:>5}".format('raw', 'v', 'deg C', 'deg F'))

while True:
    val, volt, val1, volt1 = (chan.value, chan.voltage, chan1.value, chan1.voltage)
    tempC = (volt - 1.25)/0.005
    tempF = tempC*1.8 + 32.0
    atime = time.strftime( '%H%M%S.%u', time.localtime() )
    print("{:>5}\t{:>5}\t{:>5.3f}\t{:>5.3}\t{:>5.3f}\t{:>5.3f}\t{}"
    .format(val, val1, volt, volt1, tempC, tempF, atime) )
    time.sleep(5)

