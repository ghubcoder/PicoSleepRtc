

Raspberry Pico - waking the Pico using a DS3231
==========

This is a simple demo to show how you can place the pico into a low power dormant mode, which according to the docs will pull around 0.8mA and then wake it using an external DS3231. This should be useful to allow minimal power consumption and periodic waking to perform some work.

This is based off the code provided [here](https://github.com/raspberrypi/pico-playground/blob/master/sleep/hello_sleep/hello_sleep.c)

Layout as follows:

```
~/pico/pico-sdk/
~/pico/pico-extras/
~/pico/picosleeprtc/
```

Create a new build dir inside this repo:

```
mkdir ~/pico/picosleeprtc/build
```

Change into the directory and run the following:

```
export PICO_SDK_PATH=../../pico-sdk
export PICO_EXTRAS_PATH=../../pico-extras
cmake ..
make
```

This will create the `sleep.elf` file below. Alternatively drag the `sleep.uf2` file onto the pico via usb.

To deploy this using SWD and monitor using minicom, run the following once you have wired everything up as per the pico docs.

Start following in a terminal:
```
openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg
```

Also start minicom to listen for output in a new terminal:
```
minicom -b 115200 -o -D /dev/serial0
```

Enter gdb and load file:
```
gdb-multiarch sleep.elf
target remote localhost:3333
load
monitor reset init
continue
```

In the minicom terminal window you should see debug output and the led should blink on and off each minute.

For more details, such as wiring instructions please see this [post](https://ghubcoder.github.io/posts/waking-the-pico-external-trigger/).
