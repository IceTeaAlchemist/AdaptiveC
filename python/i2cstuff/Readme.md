# Some of Bill's notes

## Exploring i2c on the Raspberry Pi 3 b+.

Following the Adafruit help files for the 16-bit ADC with i2c at 

https://www.adafruit.com/product/1085

and their recommendation of using the pypi / pip installs.

Circuit Python is Pip installable, https://pypi.org/search/?q=adafruit-circuitpython .

Blinka libraries https://circuitpython.org/blinka .

Found<br>
sudo pip3 install adafruit-circuitpython-ads1x15 <br>
and the website <br>
https://pypi.org/project/adafruit-circuitpython-ads1x15/

So far working fine.

Also worked through the interrupt vs polling that Franz suggested at<br>
https://roboticsbackend.com/raspberry-pi-gpio-interrupts-tutorial/

Says that these are not hardware interrupts in the user space but still software though they can be such that there is little or no CPU cycle used to check them until they are handled.  Tutorial suggests using a sleeping thread and wake it up when the "interrupt" hits.  Has callback routines aka interrupr service routines in the example.


