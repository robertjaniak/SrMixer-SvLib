/*
 * ethernet.h
 *
 *  Created on: Sep 3, 2021
 *      Author: rober
 */

#ifndef INC_ETHERNET_H_
#define INC_ETHERNET_H_

#include "global.h"
#include "buffer.h"

typedef struct {
	uint8_t mac[6];
	uint8_t ip[4];
	uint8_t sn[4];
	uint8_t gw[4];
	uint16_t outputPort;
	uint16_t inputPort;
}Net_t;

typedef struct {
	Net_t device;
	Net_t host;
	IO_pin select_line;
}Ethernet_t;

int32_t Ethernet_Init(Ethernet_t* eth, Net_t* dev, Net_t* host);
void setDeviceNet(Net_t* net);
void setHostNet(Net_t* net);
uint8_t isLinked();

int32_t udp_send(RingBuffer_t* buf);
int32_t udp_read(RingBuffer_t* buf);

#define SEPARATOR            "=============================================\r\n"
#define WELCOME_MSG  		 "Welcome to STM32Nucleo Ethernet configuration\r\n"
#define NETWORK_MSG  		 "Network configuration:\r\n"
#define IP_MSG 		 		 "  IP ADDRESS:  %d.%d.%d.%d\r\n"
#define NETMASK_MSG	         "  NETMASK:     %d.%d.%d.%d\r\n"
#define GW_MSG 		 		 "  GATEWAY:     %d.%d.%d.%d\r\n"
#define MAC_MSG		 		 "  MAC ADDRESS: %x:%x:%x:%x:%x:%x\r\n"
#define GREETING_MSG 		 "Well done guys! Welcome to the IoT world. Bye!\r\n"
#define CONN_ESTABLISHED_MSG "Connection established with remote IP: %d.%d.%d.%d:%d\r\n"
#define SENT_MESSAGE_MSG	 "Sent a message. Let's close the socket!\r\n"
#define WRONG_SEND_MSG	 	 "Something went wrong with send data; return value: %ld\r\n"
#define WRONG_READ_MSG	 	 "Something went wrong with read data; return value: %ld\r\n"
#define SEND_SOCKET_ERROR	 "Can not open socket for send data; return value: %ld\r\n"
#define READ_SOCKET_ERROR	 "Can not open socket for read data; return value: %ld\r\n"
#define READ_DATA_ERROR	     "Read data error; return value: %ld\r\n"
#define WRONG_STATUS_MSG	 "Something went wrong; STATUS: %ld\r\n"
#define LISTEN_ERR_MSG		 "LISTEN Error!\r\n"



#endif /* INC_ETHERNET_H_ */
