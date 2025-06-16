# KallistiOS Tetris
Low-level tetris implementation in KallistiOS for the Sega Dreamcast

attempt/main.c is the main source file.

---------

Steps I took to configure Dreamcast software development (using Windows 11 and a serial cable)

- Install Debian on Windows Subsystem for Linux (WSL), a Linux virtual machine client integrated into windows
- Install KallistiOS (KOS), the Dreamcast development library/operating system, on the Debian WSL
- Resources for these steps:
  - http://gamedev.allusion.net/softprj/kos/setup-wsl.php (archive: https://web.archive.org/web/20230428185010/http://gamedev.allusion.net/softprj/kos/setup-wsl.php)
  - https://www.racketboy.com/forum/viewtopic.php?t=50699 (archive: https://web.archive.org/web/20211218060154/https://www.racketboy.com/forum/viewtopic.php?t=50699)
(Note: the racketboy tutorial is for Cygwin, a different Linux virtual machine client, so ignore that part and follow the WSL steps in the first link instead)

For testing on the dreamcast using a USB-to-Dreamcast-Serial cord (coder's cable):
- Here's where I bought mine, it's $85 (came to around $100 with shipping) so it's definitely pricey, but if you're writing your own software it makes your life 1000000 times easier.
  - https://www.retroonyx.com/product-page/dreamcast-usb-coders-cable
- You will need to allow the WSL virtual machine to see your computer's real USB port. To do this, you need the tool USBIP:
  - https://learn.microsoft.com/en-us/windows/wsl/connect-usb (archive: https://web.archive.org/web/20230426065559/https://learn.microsoft.com/en-us/windows/wsl/connect-usb)
  - That tutorial is for Ubuntu so refer to this to get it to install on debian:
  - https://github.com/dorssel/usbipd-win/issues/99 (archive: https://web.archive.org/web/20230428185629/https://github.com/dorssel/usbipd-win/issues/99)
- When you have that installed, open your Debian WSL, connect the serial cable to your computer and to the dreamcast.
- then open Powershell in admin mode, run "usbipd list" to show available devices; look at the BUSID for the serial cable (USB Serial Converter). For e.g. mine was 2-1.
- Run "usbipd bind --busid 2-1" (or whatever the busid is) to share the device.
- Then run "usbipd attach --wsl --busid 2-1 --wsl Debian" (or whatever your busid is) to attach it to the current WSL instance (you may need to do this everytime you start Debian and want to send your program to the Dreamcast).

- To run the program on the dreamcast through the serial cable, you will need the programs "dc-load" (runs on the Dreamcast) and "dc-tool" (runs on the computer). You will specifically need the serial versions of each if you have the serial cable like me (as opposed to the broadband/IP versions).
  - https://sizious.com/download/dreamcast/#dc-tool (archive: https://web.archive.org/web/20221207203500/https://sizious.com/download/dreamcast/)
- Burn the dc-load (serial) .cdi disk image onto a CD-R. (I use dcload-serial 1.0.6)
- Install dc-tool-ser on your WSL.
- After compiling your software with make to create an .elf, to send it to the dreamcast using dc-tool, I run this command:
  - sudo /opt/toolchains/dc/bin/dc-tool-ser -x YOUR_PROGRAM_ELF.elf -t /dev/ttyUSB0
  - (If you use the makefile template, you're supposed to be able to just do "make run" to accomplish this, but I couldn't get it to work so i just do this.)
  - -t is the device to send it to (ie the UBS port you have the cable plugged into). You might have to change /dev/ttyUSB0 if it doesn't work to try to send it to the right one. (try /dev/ttyUSB1, etc...)

If you don't have a cable, you can burn your own dreamcast software onto a disk instead:
- You first run "make" to turn it into an .elf file.
- Convert the .elf into a binary: sh-elf-objcopy -R .stack -O binary INPUT.elf OUTPUT.bin
- Scramble the binary: /opt/toolchains/dc/kos/utils/scramble/scramble OUTPUT.bin 1st_read.bin
  - (This step is necessary bc the Dreamcast will scramble data as it reads it from disk to RAM if it detects that the media is a CD-ROM as opposed to the proprietary
Dreamcast GD-ROM. This is an anti piracy measure. Using the 'scramble' program will reverse-scramble it in the other direction so that when the DC scrambles it it is
actually putting it back in the correct order.)
- Use Bootdreams to compile it into a disc image (follow the directions from the racketboy tutorial)
- Burn this disc image onto a CD-R, then pop it into a dreamcast.
- If you don't have a real dreamcast, or you don't want to burn it onto a disk, you can also use this disk image in a Dreamcast emulator.

TIP: For some reason, the Linux WSL can access files from the host (Windows) machine file system, but the Windows machine can't access the Linux file system. Therefore, to be able to work on code in a Windows IDE like Visual Studio Code, you should store your project files in the Windows file system, like I have mine stored in a directory C:/Data. From Linux, you can acces Windows' file system using /mnt/c (so for my directory I cd to /mnt/c/Data).
