#!/usr/bin/env python3
#
# 20211127 weg, started and want to read the two ADCs, 0x48 has the thermocouple on
#               channel A0 single ended.
# See page 16 of adafruit-4-channel-adc-breakouts.pdf .
# From newer CircuitPython libraries
# https://pypi.org/project/adafruit-circuitpython-ads1x15/ 
#
# 20211128 weg, hardware 2 ADCS at 0x48 with A0 A1 each side fo the thermistor and 0x49
# with A0 the voltage divided batter pack voltage: 6.06V is 0.555V on the divider, resistors
# are 9.81KOhm, 0.988K and sum to 10.80K...ideal divider ratio is 0.988/10.80 = 0.915, so 
# that 6.06V becomes 0.554V.  Looks pretty close!!

import datetime
import time

import adafruit_ads1x15.ads1115 as ADS  # ADS1015 is 12-bit, ADS1115 is 16-bit (using these)
import board
import busio
from adafruit_ads1x15.analog_in import AnalogIn

# Create the I2C bus
i2c = busio.I2C(board.SCL, board.SDA)
print('board.SCL, board.SDA are {}'.format( (board.SCL, board.SDA) ) )

# Create the ADC object using the I2C bus
ads0 = ADS.ADS1115(i2c, address=0x48)
ads1 = ADS.ADS1115(i2c, address=0x49)

# Create single-ended input on channel 0
chan00 = AnalogIn(ads0, ADS.P0)
chan01 = AnalogIn(ads0, ADS.P1)

chan10 = AnalogIn(ads1, ADS.P0)
chan11 = AnalogIn(ads1, ADS.P1)

# Create differential input between channel 0 and 1
# chan = AnalogIn(ads, ADS.P0, ADS.P1)

print("{:>5}\t{:>5}\t{:>5}\t{:>5}\t{:>5}\t{:>5}\t{:>5}".format(
    'raw', 'raw1', 'v', 'v1',  'deg C', 'deg F', 'time'))

icnt = 0
printAds0 = True; nxttext = '0x48'
print('========== {} ==============='.format(nxttext))

while True:
    if printAds0:
        a0 = chan00; a1 = chan01
        nxttext = '0x49'
    else:
        a0 = chan10; a1 = chan11
        nxttext = '0x48'
    
    val0, volt0, val1, volt1 = (a0.value, a0.voltage, 
                                    a1.value, a1.voltage)
    tempC = (volt0 - 1.25)/0.005
    tempF = tempC*1.8 + 32.0
    atime = time.strftime( '%H%M%S.%u', time.localtime() )
    print("{:>5}\t{:>5}\t{:>5.3f}\t{:>5.3}\t{:>5.3f}\t{:>5.3f}\t{}"
    .format(val0, val1, volt0, volt1, tempC, tempF, atime) )
    time.sleep(5)

    if icnt%3 == 0 and icnt != 0:
        printAds0 = not printAds0
        print('========== {} ==============='.format(nxttext))
    icnt += 1
       

