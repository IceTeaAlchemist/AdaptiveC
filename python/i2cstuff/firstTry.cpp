// SOURCE: https://www.kernel.org/doc/Documentation/i2c/dev-interface */
// 2021 weg, started
// This is a code snippet from Kernel.Org .

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

int file;
int adapter_nr = 2;
char filename[20];snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);

file = open(filename, O_RDWR);

if (file < 0) {
  exit(1);
}  

int addr = 0x40;

if (ioctl(file, I2C_SLAVE, addr) < 0) {
  exit(1);
}

__u8 reg = 0x10;
__s32 res;
char buf[10];

res = i2c_smbus_read_word_data(file, reg);
if (res < 0) {
  /* ERROR HANDLING: i2c transaction failed */
} else {
  /* res contains the read word */
}

buf[0] = reg;
buf[1] = 0x43;
buf[2] = 0x65;
if (write(file, buf, 3) != 3) {
  /* ERROR HANDLING: i2c transaction failed */
}
