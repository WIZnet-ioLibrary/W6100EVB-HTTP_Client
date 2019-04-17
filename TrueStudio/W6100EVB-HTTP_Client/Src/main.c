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
#include "dhcpv6.h"
//#include "dhcp_cb.h"
#include "dns.h"
#include "httpClient.h"
#include "socket.h"
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

//for http client
// Example domain name
uint8_t Domain_IP[16]  = {0,};                  // Translated IP address by DNS Server
uint8_t Domain_name[] = "httpbin.org";
uint8_t URI_GET[] = "/get";
uint8_t URI_POST[] = "/post";
uint8_t body_data[] = "apple=11&banana=22&mango=33";

uint8_t Domain_name6[] = "ip6tools.com";
uint8_t URI[] = "/";

uint8_t flag_sent_http_request = DISABLE;

#define DATA_BUF_SIZE 2048
uint8_t g_send_buf[DATA_BUF_SIZE];
uint8_t g_recv_buf[DATA_BUF_SIZE];


uint8_t ip_ver = AS_IPV4;
//uint8_t ip_ver = AS_IPV6;
//uint8_t ip_ver = AS_IPDUAL;

uint8_t con_ver = AS_IPV4;
//uint8_t con_ver = AS_IPV6;

wiz_NetInfo	WIZCHIP_NetInfo	={
					{0x00, 0x08, 0xdc, 0x57, 0x57, 0x63},//mac
					{192, 168, 100, 27},			// <-need user ip
					{255, 255, 255, 0},				// <-need user subnet mask
					{192,168,100,1},				// <-need user gate way
					{0x00,0x00, 0x00,0x00,
					 0x00,0x00, 0x00,0x00,
					 0x00,0x00, 0x00,0x00,
					 0x00,0x00, 0x00, 0x00},	// <-need user lla
					{0x00,0x00,0x00,0x00,
					 0x00,0x00,0x00,0x00,
					 0x00,0x00, 0x00,0x00,
					 0x00,0x00, 0x00, 0x00},	// <-need user gua
					{0xff,0xff,0xff,0xff,
					0xff,0xff,0xff,0xff,
					0x00,0x00,0x00, 0x00,
					0x00,0x00,0x00,0x00},		// <-need user prefix
					{0x00, 0x00, 0x00,0x00,
					0x00,0x00,0x00,0x00,
					0x00,0x00, 0x00,0x00,
					0x00,0x00, 0x00,0x00},		// <-need user gate way 6


//					{0x8,0x8,0x8,0x8},			//google dns
					{168, 126, 63, 1},			//kt dns
					{0x20,0x01,0x48,0x60,
					 0x48,0x60,0x00,0x00,
					 0x00,0x00,0x00,0x00,
					 0x00,0x00,0x88,0x88},							//google dns6

					NETINFO_STATIC_V4,				//ipmode
};
uint16_t WIZCHIP_PORT = 5000;

#define ETH_MAX_BUF_SIZE	2048
uint8_t data_buf [ETH_MAX_BUF_SIZE]; // TX Buffer for applications
void print_network_information(void);
void delay(unsigned int count);
int8_t process_dhcp(void);
int8_t process_dns(void);
uint8_t IP_TYPE;

uint16_t i;
uint16_t len = 0;
uint16_t state_cnt=0;

int main(void)
{
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
			printf(" # DNS: %s => %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n", Domain_name6, Domain_IP[0], Domain_IP[1], Domain_IP[2], Domain_IP[3],
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

	if (ip_ver == AS_IPV4)	httpc_init(0, Domain_IP, 80, g_send_buf, g_recv_buf);
	else httpc_init(0, Domain_IP, 80, g_send_buf, g_recv_buf);
	state_cnt=0;

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
				if (ip_ver == AS_IPV4)
				{
					if (state_cnt==0)
					{
						// Send: HTTP request
						request.method = (uint8_t *)HTTP_GET;
						request.uri = (uint8_t *)URI_GET;
						request.host = (uint8_t *)Domain_name;

						// HTTP client example #1: Function for send HTTP request (header and body fields are integrated)
						{
							httpc_send(&request, g_recv_buf, g_send_buf, 0);
						}
					}
					if (state_cnt==1)
					{
						request.method = (uint8_t *)HTTP_POST;
						request.uri = (uint8_t *)URI_POST;
						request.host = (uint8_t *)Domain_name;
						request.content_type =  (uint8_t *)"application/x-www-form-urlencoded";
						request.content_length = 27;
						// HTTP client example #1: Function for send HTTP request (header and body fields are integrated)
						{
							httpc_send(&request, g_recv_buf, body_data, request.content_length);
						}
					}
					// HTTP client example #2: Separate functions for HTTP request - default header + body
					{
//						httpc_send_header(&request, g_recv_buf, NULL, len);
//						httpc_send_body(g_send_buf, len); // Send HTTP request message body
					}

					// HTTP client example #3: Separate functions for HTTP request with custom header fields - default header + custom header + body
					{
						//httpc_add_customHeader_field(tmpbuf, "Custom-Auth", "auth_method_string"); // custom header field extended - example #1
						//httpc_add_customHeader_field(tmpbuf, "Key", "auth_key_string"); // custom header field extended - example #2
						//httpc_send_header(&request, g_recv_buf, tmpbuf, len);
						//httpc_send_body(g_send_buf, len);
					}
				}
				if(ip_ver == AS_IPV6)
				{
					if (state_cnt==0)
					{
						// Send: HTTP request
						request.method = (uint8_t *)HTTP_GET;
						request.uri = (uint8_t *)URI;
						request.host = (uint8_t *)Domain_name6;

						// HTTP client example #1: Function for send HTTP request (header and body fields are integrated)
						{
							httpc_send(&request, g_recv_buf, g_send_buf, 0);
						}
					}
				}

				flag_sent_http_request = ENABLE;
			}

			// Recv: HTTP response
			if(httpc_isReceived > 0)
			{
				printf("httpc_isReceived: %d\r\n",httpc_isReceived);
				len = httpc_recv(g_recv_buf, httpc_isReceived);

				printf(" >> HTTP Response - Received len: %d\r\n", len);
				printf("======================================================\r\n");
				for(i = 0; i < len; i++) {
					printf("%c", g_recv_buf[i]);
				}
				printf("\r\n");
				printf("======================================================\r\n");
				delay(2);
				getsockopt(0,SO_RECVBUF,&len);
				if (!len)
				{
					flag_sent_http_request = DISABLE;
					state_cnt++;
				}
			}
		}

#ifdef __USE_DHCP__
		DHCP_run(); // DHCP renew
#endif
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
#ifdef __USE_DHCP__
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
        ret = DHCP_run2();

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
#endif

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
			if((ret = DNS_run(SOCK_DNS,WIZCHIP_NetInfo.dns, (uint8_t *)Domain_name, Domain_IP,AS_IPV4)) == 1)
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
			if((ret = DNS_run(SOCK_DNS,WIZCHIP_NetInfo.dns6, (uint8_t *)Domain_name6, Domain_IP,AS_IPV6)) == 1)
			{
#ifdef _MAIN_DEBUG_
				printf(" - DNS6 Success\r\n");
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


