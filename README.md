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
  <img width="60%" src="https://wizwiki.net/wiki/lib/exe/fetch.php?w=600&tok=eabde4&media=products:w6100:w6100_evb:w6100-evb_callout.png" />
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
    <img width="60%" src="https://user-images.githubusercontent.com/34225062/56016137-45ab9280-5d36-11e9-9142-68a57b3f6b9a.png" />    
  </p>
  - If you want to work over IP version 6, set the ip_ver value from AS_IPV4 to AS_IPV6.
    <img width="60%" src="https://user-images.githubusercontent.com/34225062/56016283-b0f56480-5d36-11e9-95f2-134cfa103c80.png" />       

  ## Code review
  * main.c code flow <br>
  <p align="center">
    <img width="50%" src="https://user-images.githubusercontent.com/9648281/55851411-67204900-5b93-11e9-988c-4d1b3d38d744.jpg" />
  </p>

   ## Test packet capture file
     <p align="center">
    <img width="100%" src="https://user-images.githubusercontent.com/9648281/55846455-bd37c100-5b80-11e9-91bd-4516bc67eac2.JPG" />
  </p>

    
    -Test packet capture file :  [HTTP_Client_Packet.zip](https://github.com/WIZnet-ioLibrary/W6100EVB-HTTP_Client/files/3071859/http_client_packet.zip)

