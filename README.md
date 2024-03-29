# Index
- [HTTP Client Example for W6100-EVB](#HTTP-Client-Example-for-W6100-EVB)
- [Hardware Environment](#Hardware-Environment)
- [Software Environments](#Software-Environment)
- [Run](#Run)
- [Code review](#Code-review)
  - [Test packet capture file](#Test-packet-capture-file)



------
# HTTP Client Example for W6100-EVB
Common to Any MCU, Easy to Add-on. Internet Offload co-Processor, HW TCP/IP chip,
best fits for low-end Non-OS devices connecting to Ethernet for the Internet of Things. These will be updated continuously.

## Hardware Environment
* W6100EVB
  - connecting Micro usb.
  - connecting Ethernet cable. <br>
<p align="center">
  <img width="60%" src="https://docs.wiznet.io/assets/images/w6100-evb_callout-d5d88d99555cd8b78d6a8327b849cd58.png" />
</p>

## Software Environment
In case of used to TrueSTUDIO, it is the same as HTTP Server example.
 - Link : [Software Environment of W6100EVB-HTTP_Server](https://github.com/WIZnet-ioLibrary/W6100EVB-HTTP_Server#Software-Environment)

## Limitation
This project requires an environment that runs DHCP and DNS.

## Run
* Demo Environment & Program <br>

  - Windows 10 <br>
  - web browser(internet explorer or crome)
  - Hercules <br>

* Demo Result <br>
  - Power On and push Reset button to start Program<br>
  - Program Run Serial display <br>
  <p align="center">
    <img width="60%" src="https://user-images.githubusercontent.com/34225062/56174744-f06dca80-602e-11e9-931c-c2dd459e16a1.png" />      
    <img width="60%" src="https://user-images.githubusercontent.com/34225062/56174745-f1066100-602e-11e9-9f1a-9cd9f0febdc7.png" />    
  </p>
  
  - If you want to work over IP version 6, set the ip_ver value which is in main.c from AS_IPV4 to AS_IPV6.
  <p align="center">  
    <img width="30%" src="https://user-images.githubusercontent.com/34225062/56016283-b0f56480-5d36-11e9-95f2-134cfa103c80.png" />       
  </p>

  ## Code review
  * main.c code flow <br>
  <p align="center">
    <img width="40%" src="https://user-images.githubusercontent.com/34225062/56019764-74c70180-5d40-11e9-9e36-8867417a16c7.jpg" />
  </p>

   ## Test packet capture file
    -Test packet capture file :  [HTTP_Client_Packet.zip](https://github.com) will be update, if DHCP is OK

