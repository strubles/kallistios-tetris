

Connecting WSL to dreamcast over serial

1. plug in serial cable into usb port
2. open terminal in admin mode
3. open WSL in admin mode
4. run `usbipd list` in terminal
5. find the USB Serial Converter and note the BUSID
6. share it with `usbipd bind --busid (busid)` in terminal if it's not already
7. attach it to WSL with `usbipd attach --busid (busid) --wsl --persist` in terminal
8. in WSL, run `lsusb` to check connected usb devices
9. to find the dev it's mounted on, in WSL, run `dmesg | tail -n 50` and look for a line like "[ 1922.025552] usb 1-1: FTDI USB Serial Device converter now attached to ttyUSB0"
    * If it doesn't appear, in WSL, reload the drivers with `sudo modprobe usbserial` then `sudo modprobe ftdi_sio`


* [KallistiOS maple/controller.c](https://github.com/KallistiOS/KallistiOS/blob/50322ad313f8c46db7e1aa1391a153ae14f9f003/kernel/arch/dreamcast/hardware/maple/controller.c)
* https://cadcdev.sourceforge.net/docs/kos-2.0.0/group__controller__buttons.html
* http://gamedev.allusion.net/docs/kos-2.0.0/structcont__state__t.html