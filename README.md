Steps I took to configure Dreamcast software development (using Windows 11 and a serial cable)

Install Debian on Windows Subsystem for Linux (WSL), a Linux virtual machine client integrated into windows
Install KallistiOS (KOS), the Dreamcast development library/operating system, on the Debian WSL
Resources for these steps:
http://gamedev.allusion.net/softprj/kos/setup-wsl.php (archive: https://web.archive.org/web/20230428185010/http://gamedev.allusion.net/softprj/kos/setup-wsl.php)
https://www.racketboy.com/forum/viewtopic.php?t=50699 (archive: https://web.archive.org/web/20211218060154/https://www.racketboy.com/forum/viewtopic.php?t=50699)
(Note: the racketboy tutorial is for Cygwin, a different Linux virtual machine client, so ignore that part and follow the WSL steps in the first link instead)

For testing on the dreamcast using a USB-to-Dreamcast-Serial cord (coder's cable):
You will need to allow the WSL virtual machine to see your computer's real USB port. To do this, you need the tool USBIP:
https://learn.microsoft.com/en-us/windows/wsl/connect-usb (archive: https://web.archive.org/web/20230426065559/https://learn.microsoft.com/en-us/windows/wsl/connect-usb)
That tutorial is for Ubuntu so refer to this to get it to install on debian:
https://github.com/dorssel/usbipd-win/issues/99 (archive: https://web.archive.org/web/20230428185629/https://github.com/dorssel/usbipd-win/issues/99)

When you have that installed, open your Debian WSL, connect the serial cable to your computer and to the dreamcast. then open Powershell in admin mode, run
"usbip wsl list" to show available devices; look at the BUSID for the serial cable (USB Serial Converter) and then run "usbip wsl attach --busid (busid)" (for me it was
usbip wsl attach --busid 2-2) to attach it to the current WSL instance (you will need to do this everytime you start Debian and want to send your program to the Dreamcast).

To run on the dreamcast, you will need the programs "dc-load" (runs on the Dreamcast) and "dc-tool" (runs on the computer). You will specifically need the serial versions
if you have the serial cable like me (as opposed to the broadband/IP versions)
https://sizious.com/download/dreamcast/#dc-tool (archive: https://web.archive.org/web/20221207203500/https://sizious.com/download/dreamcast/)
Burn dc-load (serial) .cdi disk image onto a CD-R. (I use dcload-serial 1.0.6) Install dc-tool-ser on your WSL.
After compiling your software with make to create an .elf, to send it to the dreamcast using dc-tool, I run this command:
sudo /opt/toolchains/dc/bin/dc-tool-ser -x YOUR_PROGRAM_ELF.elf -t /dev/ttyUSB0
(If you use the makefile template, you're supposed to be able to just do "make run" to accomplish this, but I couldn't get it to work so i just do this.)
-t is the device to send it to (ie the UBS port you have the cable plugged into) You might have to change /dev/ttyUSB0 if it doesn't work to try to send it to the right
one. (try /dev/ttyUSB1, etc...)

For burning your dreamcast game onto a disk, it's a little more complicated:
- You first run "make" to turn it into an .elf file.
- Convert the .elf into a binary: sh-elf-objcopy -R .stack -O binary INPUT.elf OUTPUT.bin
- Scramble the binary: /opt/toolchains/dc/kos/utils/scramble/scramble OUTPUT.bin 1st_read.bin
(This step is necessary bc the Dreamcast will scramble data as it reads it from disk to RAM if it detects that the media is a CD-ROM as opposed to the proprietary
Dreamcast GD-ROM. This is an anti piracy measure. Using the 'scramble' program will reverse-scramble it in the other direction so that when the DC scrambles it it is
actually putting it back in the correct order.)
- Use Bootdreams to compile it into a disc image (follow the directions from the racketboy tutorial)
- Burn this disc image onto a CD-R, pop it into a dreamcast.
