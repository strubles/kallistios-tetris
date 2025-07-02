# Dev setup
Steps I took to configure Dreamcast software development (using Windows 11 and a serial cable)

## Toolchain setup
- Install Debian on Windows Subsystem for Linux (WSL), a Linux virtual machine client integrated into windows
- Install KallistiOS (KOS), the Dreamcast development library/operating system, on the Debian WSL
- Resources for these steps:
  - http://gamedev.allusion.net/softprj/kos/setup-wsl.php (archive: https://web.archive.org/web/20230428185010/http://gamedev.allusion.net/softprj/kos/setup-wsl.php)
  - https://www.racketboy.com/forum/viewtopic.php?t=50699 (archive: https://web.archive.org/web/20211218060154/https://www.racketboy.com/forum/viewtopic.php?t=50699)
(Note: the racketboy tutorial is for Cygwin, a different Linux virtual machine client, so ignore that part and follow the WSL steps in the first link instead)

## Running on the dreamcast using a USB-to-Dreamcast serial cord (coder's cable)
- Here's where I bought mine, it's $85 (came to around $100 with shipping) so it's definitely pricey, but if you're writing your own software it makes your life 1000000 times easier.
  - https://www.retroonyx.com/product-page/dreamcast-usb-coders-cable
- If you are using WSL like me, then you will need to allow the WSL virtual machine to see your computer's real USB port. To do this, you need the tool USBIP:
  - https://learn.microsoft.com/en-us/windows/wsl/connect-usb (archive: https://web.archive.org/web/20230426065559/https://learn.microsoft.com/en-us/windows/wsl/connect-usb)
  - That tutorial is for Ubuntu so refer to this to get it to install on debian:
  - https://github.com/dorssel/usbipd-win/issues/99 (archive: https://web.archive.org/web/20230428185629/https://github.com/dorssel/usbipd-win/issues/99)
- To run the program on the dreamcast through the serial cable, you will need the programs "dc-load" (runs on the Dreamcast) and "dc-tool" (runs on the computer). You will specifically need the serial versions of each if you have the serial cable like me (as opposed to the broadband/IP versions).
  - https://sizious.com/download/dreamcast/#dc-tool (archive: https://web.archive.org/web/20221207203500/https://sizious.com/download/dreamcast/)
- Burn the dc-load (serial) .cdi disk image onto a CD-R. (I use dcload-serial 1.0.6)
- Install dc-tool-ser on your WSL.
- After compiling your software with make to create an .elf:
  1. plug in serial cable into usb port
  2. open terminal in admin mode
  3. open WSL in admin mode
  4. run `usbipd list` in terminal
  5. find the USB Serial Converter and note the BUSID
  6. share it with `usbipd bind --busid (busid)` in terminal if it's not already
  7. attach it to WSL with `usbipd attach --busid (busid) --wsl --persist` in terminal
  8. in WSL, run `lsusb` to check connected usb devices
  9. to find the dev it's mounted on, in WSL, run `dmesg | tail -n 50` and look for a line like `[ 1922.025552] usb 1-1: FTDI USB Serial Device converter now attached to ttyUSB0`
      * If it doesn't appear, in WSL, reload the drivers with `sudo modprobe usbserial` then `sudo modprobe ftdi_sio` then try again

## Running on emulator or on hardware from disk
- Run "make" to turn it into an .elf file.
  - Many emulators support running the .elf without needing to package it into a .cdi, so you might not need to follow the following steps.
- Convert the .elf into a binary: `sh-elf-objcopy -R .stack -O binary INPUT.elf OUTPUT.bin`
- Encrypt the binary: `/opt/toolchains/dc/kos/utils/scramble/scramble OUTPUT.bin 1st_read.bin`
  - This step is necessary bc the Dreamcast will decrypt data as it reads it from disk to RAM if it detects that the media is a CD-ROM as opposed to the proprietary Dreamcast GD-ROM. This was intended to prevent running ripped/backed up games. Using the 'scramble' program will reverse-scramble it in the other direction so that when the DC scrambles it it is actually putting it back in the correct order.
- Use Bootdreams to compile it into a disc image (follow the directions from the racketboy tutorial).
- Burn this disc image onto a CD-R then insert into dreamcast, or load it into an emulator.

TIP: For some reason, the Linux WSL can access files from the host (Windows) machine file system, but the Windows machine can't access the Linux file system. Therefore, to be able to work on code in a Windows IDE like Visual Studio Code, you should store your project files in the Windows file system, like I have mine stored in a directory C:/Data. From Linux, you can acces Windows' file system using /mnt/c (so for my directory I cd to /mnt/c/Data).
