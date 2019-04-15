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
  <img width="20%" src="https://wizwiki.net/wiki/lib/exe/fetch.php?w=600&tok=eabde4&media=products:w6100:w6100_evb:w6100-evb_callout.png" />
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
    <img width="60%" src="https://user-images.githubusercontent.com/34225062/56117492-21a0b900-5fa3-11e9-8a75-2e51866ded9c.png" />      
    <img width="60%" src="https://user-images.githubusercontent.com/34225062/56117531-3b420080-5fa3-11e9-83d3-175b29b9c18d.png" />    
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
    -Test packet capture file :  [HTTP_Client_Packet.zip](https://github.com/WIZnet-ioLibrary/W6100EVB-HTTP_Client/files/3071859/http_client_packet.zip)

