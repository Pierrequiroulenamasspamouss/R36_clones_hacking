# R36_clones_hacking
A guide on how to make your R36 clone connect via ssh through USB

You bought an R36 clone and you don't know what to do with it ? Don't worry, I was in the same situation than you a few days ago. 
Because I like to tinker, and thanks to the post of u/Careful-Ad3182 and u/Ill-Improvement-2003 on reddit, I was able to connect to my console via UART, and by that I mean that I have access to the internal 8gb storage chip of my R36S (pro in my case). Using UART gives a direct shell root access, so it was not real hacking, but I felt very proud. I was somewhere I shouldn't be, and I had the control to modify what I wanted inside the console ! What a boost of adrenaline ! 

WARNING : this is not a tutorial for beginners. it is a proof of concept that having a ssh connection to your machine via OTG is possible.

** I need someone to help me run the script directly via the homescreen , or with an easy modifications tool. (adding a new system called homebrews is possible, to bypass the need for UART and just use the serial via UART once.

What I need is a script to modify the file /storage/.emulationstation/es_systems.cfg and add the following lines inbetween the other systems : 
```
  <system>
    <name>homebrews</name>
    <fullname>Homebrews</fullname>
    <path>/storage/roms/homebrews</path>
    <extension>.sh .SH</extension>
    <command>sh %ROM%</command>
    <platform>homebrews</platform>
    <theme>homebrews</theme>
  </system>
```
like that you could run the server from the emuElec homescreen. 

)**

# Prerequisites
- UART to USB interface. Any controller that can go up to 1 500 000 BAUD can do the job. I repurposed an old ESP32 from which I removed the chip and kept the interfacing chip ![like this one](https://www.waveshare.com/media/catalog/product/c/p/cp2102-usb-uart-board-type-a-1_1.jpg)
- An R36S (pro, but R36S should be the same, probably works for K36s and R36 MAX too with some adjustments ? )
- USB-OTG compatibe cables
- some wires to connect RX, TX and GND to the UART-USB interface
- working [PuTTy ](https://www.putty.org/)  

# Setup the SD card : 
Download the latest release and unzip it to the root of your SD card. There should be no overwrites. Put the sd back in the console

# Open your console
Remove the six screws on the back of your console and remove the back shell. Then, disconnect the battery. All the clones that have a builtin storage have this feature : exposed and working UART pads. Their position can vary a lot, but they are often either near the CPU or near the SD card. Do not use the 4 pads connections, those are just for a custom install on an SD card ( like if you put the OS on the sd card instead of the builtin flash ) 
example of the pins just down below :
![1](https://github.com/Pierrequiroulenamasspamouss/R36_clones_hacking/blob/main/Capture%20d%E2%80%99%C3%A9cran%202025-05-28%20065427.png)
![2](https://github.com/Pierrequiroulenamasspamouss/R36_clones_hacking/blob/main/Capture%20d%E2%80%99%C3%A9cran%202025-05-28%20065449.png)
![3](https://github.com/Pierrequiroulenamasspamouss/R36_clones_hacking/blob/main/Capture%20d%E2%80%99%C3%A9cran%202025-05-28%20065605.png)
![4](https://github.com/Pierrequiroulenamasspamouss/R36_clones_hacking/blob/main/Capture%20d%E2%80%99%C3%A9cran%202025-05-28%20065710.png)

# Setup the mod
connect the corresponding pins from your UART to USB adapter TX and RX to RX and TX of the board (this is inverted) like such : 
![image](https://github.com/user-attachments/assets/4ff4f2b5-4fa9-4179-ba17-49f4fb70d99e)

This UART process's only reason to exist is because you need to run a bash script to launch the ssh server. I will maybe later try to make a script to automate the launch at startup, but I'm not sure it would be a good idea, since the OTG is used as a host mainly without that. That's why I would suggest to expose those pins to the outside of the shell in order to easily reconnect the uart to usb interfaceif you have to reboot the system. 
Open PuTTy and int the "connection type" use "Serial". Set the speed to 1500000 ( "15" and five zeroes ) and instead of COM1, set the serial line to the COM of your device ( mine was COM9, and you should know that COM 1/2 are often used internally ). You should have your console connected to your computer via UART by now. If it's the wrong COM, use the Device manager to discover where your device is. If the configuration works, you can give it a name (e.g. "R36" ) and click on "Save" before clicking on "Open" to open the connection.   


# Running the script
Once you boot the console, you have first the logging of the device, and then, you will be greeted with a shell :
```shell
[   35.227069] (sd-umount)[1617]: Failed to unmount /flash: Device or resource busy
[   35.237675] systemd-shutdown[1]: Failed to finalize file systems, loop devices, ignoring.
[   35.319002] reboot: Restarting system with command 'now'
DDR V2.08 20220817
In
SRX
LP3,1024MB,333MHz
bw      col     bk      row     cs      dbw
32      10      8       14      2       32
cs1 row:14
OUT
Boot1 Release Time: Jan 17 2022 10:48:40, version: 1.35
ROM VER:0x56313030, 18
hamming_distance:3326 3330 3
chip_id:524b3326_0,0
ChipType = 0x12, 778
No.1 FLASH ID:1 ff ff ff ff ff
mmc2:cmd19,100


(...)
[    3.557073] rockchip-dmc dmc: failed to get vop bandwidth to dmc rate
[    3.557156] rockchip-dmc dmc: failed to get vop pn to msch rl
[    3.586562] devfreq dmc: Couldn't update frequency transition information.
RuiSuo:~ #
```
You now have access to the machine as a root user ( no need for sudo ) 

you can now run de command 
`./roms/tools/ssh_over_usb/ethernet_over_usb.sh `

This will launch a dropbear ssh server as well as a custom usb-otg script that puts the console into a listening state instead of a "USB host" state.
No need for binaries, they are included in the package (this is not my binary, nor my code, I made chatGPT rewrite my code because it was too bad)

Now, if you connect your device via OTG to your computer, you should hear the sound of a peripheral connected to the computer. You can now use the command for windows
```cmd
netsh interface ip set address "Ethernet 2" static 192.168.7.2 255.255.255.0
```
and then try to ping it
```cmd
ping 192.168.7.2
```
if your ping was successful, you can now try to run the command
```cmd
ssh root@192.168.7.2
```

It will ask for a password. Mine was "emuelec" but if it doesn't work, you could just try a blank password. 

On first boot, it might ask to generate keys. follow the instrucions on the terminal and BOOM, there you go. You now can browse the filesystem as you wish

# Issues you could have :
- readonly file system
  -> solution : find your partition ( with df -h ) and do
  `mount -o remount,rw /dev/mmcblk0p3`
  where mmcblk0p3 is the partition's name

  example of df -h :
  ```shell
  RuiSuo:~ # df -h
  Filesystem                Size      Used Available Use% Mounted on
  devtmpfs                416.9M      4.0K    416.9M   0% /dev
  tmpfs                   193.8M      2.1M    191.7M   1% /run
  /dev/mmcblk0p3         1023.7M   1009.4M     14.4M  99% /flash
  /dev/loop0              978.5M    978.5M         0 100% /
  /dev/mmcblk0p5            4.5G    484.2M      4.0G  11% /storage
  tmpfs                   484.5M         0    484.5M   0% /dev/shm
  tmpfs                     4.0M         0      4.0M   0% /sys/fs/cgroup
  tmpfs                   484.5M     32.0K    484.5M   0% /var
  tmpfs                   484.5M         0    484.5M   0% /tmp
  /dev/mmcblk2p1           87.9G     48.3G     39.5G  55% /storage/roms
  /dev/mmcblk2p1           87.9G     48.3G     39.5G  55% /storage/.update
  none                      4.5G    484.2M      4.0G  11% /tmp/assets
  none                      4.5G    484.2M      4.0G  11% /tmp/cores
  none                      4.5G    484.2M      4.0G  11% /tmp/database
  none                      4.5G    484.2M      4.0G  11% /tmp/joypads
  none                      4.5G    484.2M      4.0G  11% /tmp/overlays
  none                      4.5G    484.2M      4.0G  11% /tmp/shaders
  ```
  
  flash is where a lot of icons are stored, and the system too. Don't forget to backup everything before manipulating stuff ! 
  You can change the boot logo by replacing both logo_kernel.bmp and logo.bmp from the /flash directory

- Issues when trying to reconnect to ssh :
  ```cmd
  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  @    WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!     @
  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  IT IS POSSIBLE THAT SOMEONE IS DOING SOMETHING NASTY!
  Someone could be eavesdropping on you right now (man-in-the-middle attack)!
  It is also possible that a host key has just been changed.
  The fingerprint for the ED25519 key sent by the remote host is
  SHA256:CEQJa/t5qcg7vdVJHMRoA5KngqS+q05PuMAQzFQBrpc.
  Please contact your system administrator.
  Add correct host key in C:\\Users\\Pierr/.ssh/known_hosts to get rid of this message.
  Offending ECDSA key in C:\\Users\\Pierr/.ssh/known_hosts:3
  Host key for 192.168.0.204 has changed and you have requested strict checking.
  Host key verification failed.
  ```
  -> solution : `ssh-keygen -R 192.168.7.2` will solve this issue

- unable to ping ?
    try to check if you're able to access the shell. if not, there is an issue. try
    `netstat -lntu`
  where you should get an answer like
  ```
  tcp        0      0 127.0.0.1:1234          0.0.0.0:*               LISTEN
  tcp        0      0 192.168.7.1:22          0.0.0.0:*               LISTEN
  etc...
  ```
  You need the two tcp connections. because one is local and the other is shared with the computer, allowing the ssh server to be connected to from the computer



# What's next ? 
- An easy installer for ArkOS for that system to replace EmuElec
- An easy launch to bypass the need for uart ( use some kind of builtin way to launch a bash script ? the shortcut F1 is disabled, so it's a bummer...)
- I need help from you guys because i currently do not have a lot of time to improve this project, but I hope this will help people control their game console !  


  

