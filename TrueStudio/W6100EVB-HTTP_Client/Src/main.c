#include <stdio.h>
#include "HAL_Config.h"
#include "wizchip_conf.h"
#include "inttypes.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "W6100RelFunctions.h"
#include "serialCommand.h"
#include "stm32f10x_rcc.h"
#include "loopback.h"
#include "dhcp.h"
//#include "dhcp_cb.h"
#include "dns.h"
#include "httpClient.h"
#include "configData.h"
#include "HALInit.h"

//for DHCP, DNS
//#define __USE_DHCP__
#define __USE_DNS__
#define _MAIN_DEBUG_
#define SOCK_DHCP               3
#define SOCK_DNS                4
typedef enum
{
    OFF = 0,
    ON  = 1
} OnOff_State_Type;
uint8_t flag_process_dhcp_success = OFF;
uint8_t flag_process_dns_success = OFF;
//uint8_t dns_server[4] = {8, 8, 8, 8};           // Secondary DNS server IP
uint8_t dns_server[4] = {168, 126, 63, 1};           // Secondary DNS server IP
uint8_t dns_server_ip6[16] = {0x20,0x01,0x48,0x60,
								0x48,0x60,0x00,0x00,
								0x00,0x00,0x00,0x00,
								0x00,0x00,0x88,0x88
							 };

//for http client
// Example domain name
uint8_t Domain_IP[16]  = {0,};                  // Translated IP address by DNS Server
//uint8_t Domain_name[] = "www.kma.go.kr";
//uint8_t URI[] = "/wid/queryDFSRSS.jsp?zone=4113552000";
uint8_t Domain_name[] = "www.google.com";
uint8_t URI[] = "/search?ei=BkGsXL63AZiB-QaJ8KqoAQ&q=W6100&oq=W6100&gs_l=psy-ab.3...0.0..6208...0.0..0.0.0.......0......gws-wiz.eWEWFN8TORw";

uint8_t flag_sent_http_request = DISABLE;

#define DATA_BUF_SIZE 2048
uint8_t g_send_buf[DATA_BUF_SIZE];
uint8_t g_recv_buf[DATA_BUF_SIZE];


//uint8_t ip_ver = AS_IPV4;
uint8_t ip_ver = AS_IPV6;
//uint8_t ip_ver = AS_IPDUAL;

uint8_t con_ver = AS_IPV4;
//uint8_t con_ver = AS_IPV6;

wiz_NetInfo	WIZCHIP_NetInfo	={
					{0x00, 0x08, 0xdc, 0x57, 0x57, 0x63},//mac
//					{192, 168, 127, 103},			//ip
//					{255, 255, 255, 0},				//sn
//					{222,98,173,254},					//gw
					{222, 98, 173, 209},			//ip
					{255, 255, 255, 192},				//sn
					{222,98,173,254},					//gw
					{0xfe,0x80, 0x00,0x00,
					 0x00,0x00, 0x00,0x00,
					 0x02,0x08, 0xdc,0xff,
					 0xfe,0x57, 0x57, 0xab},	//lla
					{0x20,0x01,0x02,0xb8,
					 0x00,0x10,0x00,0x01,
					 0x02,0x08, 0xdc,0xff,
					 0xfe,0x57, 0x57, 0x67},	//gua
					{0xff,0xff,0xff,0xff,
					0xff,0xff,0xff,0xff,
					0x00,0x00,0x00, 0x00,
					0x00,0x00,0x00,0x00},		//sn6
					{0xfe, 0x80, 0x00,0x00,
					0x00,0x00,0x00,0x00,
					0x02,0x00, 0x87,0xff,
					0xfe,0x08, 0x4c,0x81},		//gw6
					{0x0,},//dns
					{0x0,},//dns6
					NETINFO_STATIC_V4,				//ipmode
};
uint16_t WIZCHIP_PORT = 5000;

wiz_NetInfo	Destination_NetInfo	={
					{0xF4, 0x4D, 0x30, 0xAC, 0xB6, 0x1C},//mac
					{192, 168, 0, 219},			//ip
					{0x0,},				//sn
					{0x0,},					//gw
					{0xfe,0x80, 0x00,0x00,
					0x00,0x00, 0x00,0x00,
					0x18,0x15, 0x27,0xd3,
					0x1c,0xa5, 0xe9, 0x0e},	//lla
					{0x20, 0x01, 0x02, 0xb8,
                    0x00, 0x10, 0xff, 0xfe,
                    0x18, 0x15, 0x27, 0xd3,
                    0x1c, 0xa5, 0xe9, 0x0e},	//gua
					{0x0,},		//sn6
					{0x0,},		//gw6
					{0x0,},//dns
					{0x0,},//dns6
					NETINFO_STATIC_V4,				//ipmode
};



#define ETH_MAX_BUF_SIZE	2048
uint8_t data_buf [ETH_MAX_BUF_SIZE]; // TX Buffer for applications
void print_network_information(void);
void delay(unsigned int count);
uint8_t UartGetc(void);
int8_t process_dhcp(void);
int8_t process_dns(void);
uint8_t IP_TYPE;

int main(void)
{
	volatile int i;
	volatile int j,k;
	uint16_t len = 0;

	uint8_t syslock = SYS_NET_LOCK;

	RCC_ClocksTypeDef RCCA_TypeDef;
	RCCInitialize();
	gpioInitialize();
	usartInitialize();
	timerInitialize();
	printf("System start.\r\n");

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
	/* SPI method callback registration */
	#if defined SPI_DMA
		reg_wizchip_spi_cbfunc(spiReadByte, spiWriteByte,spiReadBurst,spiWriteBurst);
	#else
		reg_wizchip_spi_cbfunc(spiReadByte, spiWriteByte,0,0);
	#endif
	/* CS function register */
	reg_wizchip_cs_cbfunc(csEnable,csDisable);
#else
	/* Indirect bus method callback registration */
	#if defined BUS_DMA
			reg_wizchip_bus_cbfunc(busReadByte, busWriteByte,busReadBurst,busWriteBurst);
	#else
			reg_wizchip_bus_cbfunc(busReadByte, busWriteByte,0,0);
	#endif
#endif
#if _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_
	FSMCInitialize();
#else
	spiInitailize();
#endif

	resetAssert();
	delay(10);
	resetDeassert();
	delay(10);

	W6100Initialze();
	ctlwizchip(CW_SYS_UNLOCK,& syslock);

	/* Network Configuration - DHCP client */
	// Initialize Network Information: DHCP or Static IP allocation
#ifdef __USE_DHCP__
	if(process_dhcp() == DHCP_IP_LEASED) // DHCP success
	{
		flag_process_dhcp_success = ON;
	}
	else // DHCP failed
	{
		ctlnetwork(CN_SET_NETINFO,&WIZCHIP_NetInfo); // Set default static IP settings
	}
#else
	ctlnetwork(CN_SET_NETINFO,&WIZCHIP_NetInfo); // Set default static IP settings
#endif
	printf("Register value after W6100 initialize!\r\n");
	print_network_information();

	/* DNS client */
#ifdef __USE_DNS__
	if(process_dns()) // DNS success
	{
		flag_process_dns_success = ON;
	}
#endif
	// Debug UART: DNS results print out
#ifdef __USE_DHCP__
	if(flag_process_dhcp_success == ENABLE)
	{
		printf(" # DHCP IP Leased time : %u seconds\r\n", getDHCPLeasetime());
	}
	else
	{
		printf(" # DHCP Failed\r\n");
	}
#endif

#ifdef __USE_DNS__
	if(flag_process_dns_success == ENABLE)
	{
		if (ip_ver == AS_IPV4) printf(" # DNS: %s => %d.%d.%d.%d\r\n", Domain_name, Domain_IP[0], Domain_IP[1], Domain_IP[2], Domain_IP[3]);
		else
		{
			printf(" # DNS: %s => %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n", Domain_name, Domain_IP[0], Domain_IP[1], Domain_IP[2], Domain_IP[3],
																														 Domain_IP[4], Domain_IP[5], Domain_IP[6], Domain_IP[7],
																														 Domain_IP[8], Domain_IP[9], Domain_IP[10], Domain_IP[11],
																														 Domain_IP[12], Domain_IP[13], Domain_IP[14], Domain_IP[15]);
		}
	}
	else
	{
		printf(" # DNS Failed\r\n");
	}
#endif


	while(1)
	{
		if (ip_ver == AS_IPV4)	httpc_init(0, Domain_IP, 80, g_send_buf, g_recv_buf);
		else httpc_init(0, Domain_IP, 80, g_send_buf, g_recv_buf);

		while(1)
		{
			////////////////////////////////////////////
			// HTTP socket process handler
			////////////////////////////////////////////
			if(ip_ver == AS_IPV4) httpc_connection_handler(4);
			else httpc_connection_handler(16);

			if(httpc_isSockOpen)
			{
				if (ip_ver == AS_IPV4)	httpc_connect(4);
				else httpc_connect(16);
			}

			// HTTP client example
			if(httpc_isConnected)
			{
				if(!flag_sent_http_request)
				{
					// Send: HTTP request
					request.method = (uint8_t *)HTTP_GET;
					request.uri = (uint8_t *)URI;
					request.host = (uint8_t *)Domain_name;

					// HTTP client example #1: Function for send HTTP request (header and body fields are integrated)
					{
						httpc_send(&request, g_recv_buf, g_send_buf, 0);
					}

					// HTTP client example #2: Separate functions for HTTP request - default header + body
					{
						//httpc_send_header(&request, g_recv_buf, NULL, len);
						//httpc_send_body(g_send_buf, len); // Send HTTP requset message body
					}

					// HTTP client example #3: Separate functions for HTTP request with custom header fields - default header + custom header + body
					{
						//httpc_add_customHeader_field(tmpbuf, "Custom-Auth", "auth_method_string"); // custom header field extended - example #1
						//httpc_add_customHeader_field(tmpbuf, "Key", "auth_key_string"); // custom header field extended - example #2
						//httpc_send_header(&request, g_recv_buf, tmpbuf, len);
						//httpc_send_body(g_send_buf, len);
					}

					flag_sent_http_request = ENABLE;
				}

				// Recv: HTTP response
				if(httpc_isReceived > 0)
				{
					len = httpc_recv(g_recv_buf, httpc_isReceived);

					printf(" >> HTTP Response - Received len: %d\r\n", len);
					printf("======================================================\r\n");
					for(i = 0; i < len; i++) printf("%c", g_recv_buf[i]);
					printf("\r\n");
					printf("======================================================\r\n");
				}
			}

#ifdef __USE_DHCP__
			DHCP_run(); // DHCP renew
#endif
		}
	}
}

void delay(unsigned int count)
{
	int temp;
	temp = count + TIM2_gettimer();
	while(temp > TIM2_gettimer()){}
}

void print_network_information(void)
{


    uint8_t tmp_array[16];
	wizchip_getnetinfo(&WIZCHIP_NetInfo);
	printf("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n\r",WIZCHIP_NetInfo.mac[0],WIZCHIP_NetInfo.mac[1],WIZCHIP_NetInfo.mac[2],WIZCHIP_NetInfo.mac[3],WIZCHIP_NetInfo.mac[4],WIZCHIP_NetInfo.mac[5]);
	printf("IP address : %d.%d.%d.%d\n\r",WIZCHIP_NetInfo.ip[0],WIZCHIP_NetInfo.ip[1],WIZCHIP_NetInfo.ip[2],WIZCHIP_NetInfo.ip[3]);
	printf("SM Mask	   : %d.%d.%d.%d\n\r",WIZCHIP_NetInfo.sn[0],WIZCHIP_NetInfo.sn[1],WIZCHIP_NetInfo.sn[2],WIZCHIP_NetInfo.sn[3]);
	printf("Gate way   : %d.%d.%d.%d\n\r",WIZCHIP_NetInfo.gw[0],WIZCHIP_NetInfo.gw[1],WIZCHIP_NetInfo.gw[2],WIZCHIP_NetInfo.gw[3]);
	printf("DNS Server : %d.%d.%d.%d\n\r",WIZCHIP_NetInfo.dns[0],WIZCHIP_NetInfo.dns[1],WIZCHIP_NetInfo.dns[2],WIZCHIP_NetInfo.dns[3]);
	getGA6R(tmp_array);
	printf("\r\nGW6 : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X",
	                                   tmp_array[0], tmp_array[1], tmp_array[2], tmp_array[3],
	                                   tmp_array[4], tmp_array[5], tmp_array[6], tmp_array[7],
	                                   tmp_array[8], tmp_array[9], tmp_array[10],tmp_array[11],
	                                   tmp_array[12],tmp_array[13],tmp_array[14],tmp_array[15]);

	    getLLAR(tmp_array);
	    printf("\r\nLLA : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X",
	                                   tmp_array[0], tmp_array[1], tmp_array[2], tmp_array[3],
	                                   tmp_array[4], tmp_array[5], tmp_array[6], tmp_array[7],
	                                   tmp_array[8], tmp_array[9], tmp_array[10],tmp_array[11],
	                                   tmp_array[12],tmp_array[13],tmp_array[14],tmp_array[15]);
	    getGUAR(tmp_array);
	    printf("\r\nGUA : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X",
	                                   tmp_array[0], tmp_array[1], tmp_array[2], tmp_array[3],
	                                   tmp_array[4], tmp_array[5], tmp_array[6], tmp_array[7],
	                                   tmp_array[8], tmp_array[9], tmp_array[10],tmp_array[11],
	                                   tmp_array[12],tmp_array[13],tmp_array[14],tmp_array[15]);

	    getSUB6R(tmp_array);
	    printf("\r\n6SM : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X",
	                                   tmp_array[0], tmp_array[1], tmp_array[2], tmp_array[3],
	                                   tmp_array[4], tmp_array[5], tmp_array[6], tmp_array[7],
	                                   tmp_array[8], tmp_array[9], tmp_array[10],tmp_array[11],
	                                   tmp_array[12],tmp_array[13],tmp_array[14],tmp_array[15]);


	     printf("\r\nNETCFGLOCK : %x\r\n", getNETLCKR());
}
int8_t process_dhcp(void)
{
    uint8_t ret = 0;
    uint8_t dhcp_retry = 0;

#ifdef _MAIN_DEBUG_
    printf(" - DHCP Client running\r\n");
#endif
    DHCP_init(SOCK_DHCP, data_buf);
    //reg_dhcp_cbfunc(w6100_dhcp_assign, w6100_dhcp_assign, w6100_dhcp_conflict);

    //set_device_status(ST_UPGRADE);
    while(1)
    {
        ret = DHCP_run();

        if(ret == DHCP_IP_LEASED)
        {
#ifdef _MAIN_DEBUG_
            printf(" - DHCP Success\r\n");
#endif
            break;
        }
        else if(ret == DHCP_FAILED)
        {
            dhcp_retry++;
#ifdef _MAIN_DEBUG_
            if(dhcp_retry <= 3) printf(" - DHCP Timeout occurred and retry [%d]\r\n", dhcp_retry);
#endif
        }

        if(dhcp_retry > 3)
        {
#ifdef _MAIN_DEBUG_
            printf(" - DHCP Failed\r\n\r\n");
#endif
            DHCP_stop();
            break;
        }

//        do_segcp(); // Process the requests of configuration tool during the DHCP client run.
    }

//    set_device_status(ST_OPEN);

    return ret;
}


int8_t process_dns(void)
{
    int8_t ret = 0;
    uint8_t dns_retry = 0;

#ifdef _MAIN_DEBUG_
    printf(" - DNS Client running\r\n");
#endif
    DNS_init(data_buf);
//    set_device_status(ST_UPGRADE);

    while(1)
    {
    	if(ip_ver == AS_IPV4)
    	{
    		IP_TYPE = 0x01;
			if((ret = DNS_run(SOCK_DNS,dns_server, (uint8_t *)Domain_name, Domain_IP,AS_IPV4)) == 1)
			{
#ifdef _MAIN_DEBUG_
				printf(" - DNS Success\r\n");
#endif
				break;
			}
			else
			{
				dns_retry++;
#ifdef _MAIN_DEBUG_
				if(dns_retry <= 2) printf(" - DNS Timeout occurred and retry [%d]\r\n", dns_retry);
#endif
			}
    	}
    	else
    	{
    		IP_TYPE = 0x1c;
			if((ret = DNS_run(SOCK_DNS,dns_server_ip6, (uint8_t *)Domain_name, Domain_IP,AS_IPV6)) == 1)
			{
#ifdef _MAIN_DEBUG_
				printf(" - DNS Success\r\n");
#endif
				break;
			}
			else
			{
				dns_retry++;
#ifdef _MAIN_DEBUG_
				if(dns_retry <= 2) printf(" - DNS Timeout occurred and retry [%d]\r\n", dns_retry);
#endif
			}
    	}

        if(dns_retry > 2) {
#ifdef _MAIN_DEBUG_
            printf(" - DNS Failed\r\n\r\n");
#endif
            break;
        }

        //do_segcp(); // Process the requests of configuration tool during the DNS client run.
#ifdef __USE_DHCP__
        DHCP_run();
#endif
    }
//    set_device_status(ST_OPEN);
    return ret;
}
uint8_t UartGetc(void)
{
    while( USART_GetFlagStatus(PRINTF_USART, USART_FLAG_RXNE)==RESET);
    return USART_ReceiveData(PRINTF_USART);
}


