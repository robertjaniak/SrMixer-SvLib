/*
 * ethernet.c
 *
 *  Created on: Sep 3, 2021
 *      Author: rober
 */



#include "ethernet.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>
#include "socket.h"

#define DEBUG_MESSAGE_SIZE 64
char debugMessage[DEBUG_MESSAGE_SIZE];

#ifdef HAL_UART_MODULE_ENABLED
	extern UART_HandleTypeDef huart1;
#endif /* HAL_SPI_MODULE_ENABLED */


#ifdef HAL_SPI_MODULE_ENABLED
	extern SPI_HandleTypeDef hspi2;
#endif /* HAL_SPI_MODULE_ENABLED */


#define HUART &huart1
#define HSPI &hspi2

Ethernet_t* eth_ptr;

void cs_sel() {
	HAL_GPIO_WritePin(eth_ptr->select_line.port, eth_ptr->select_line.pin, GPIO_PIN_RESET); //CS LOW
}

void cs_desel() {
	HAL_GPIO_WritePin(eth_ptr->select_line.port, eth_ptr->select_line.pin, GPIO_PIN_SET); //CS HIGH
}

uint8_t spi_rb(void) {
	uint8_t rbuf = 0;

	#ifdef HAL_SPI_MODULE_ENABLED
		HAL_SPI_Receive(HSPI, &rbuf, 1, 0xFFFFFFFF);
	#endif /* HAL_SPI_MODULE_ENABLED */

	return rbuf;
}

void spi_wb(uint8_t b) {

	#ifdef HAL_SPI_MODULE_ENABLED
		HAL_SPI_Transmit(HSPI, &b, 1, 0xFFFFFFFF);
	#endif /* HAL_SPI_MODULE_ENABLED */
}

#define PRINT_HEADER() do  {\
  PRINT_STR(SEPARATOR);\
  PRINT_STR(WELCOME_MSG);\
  PRINT_STR(SEPARATOR);\
} while(0)

#define PRINT_NETINFO(netInfo) do {\
  PRINT_STR(NETWORK_MSG);\
  sprintf(debugMessage, MAC_MSG, netInfo.mac[0], netInfo.mac[1], netInfo.mac[2], netInfo.mac[3], netInfo.mac[4], netInfo.mac[5]);\
  PRINT_STR(debugMessage);\
  sprintf(debugMessage, IP_MSG, netInfo.ip[0], netInfo.ip[1], netInfo.ip[2], netInfo.ip[3]);										\
  PRINT_STR(debugMessage);\
  sprintf(debugMessage, NETMASK_MSG, netInfo.sn[0], netInfo.sn[1], netInfo.sn[2], netInfo.sn[3]);								\
  PRINT_STR(debugMessage);\
  sprintf(debugMessage, GW_MSG, netInfo.gw[0], netInfo.gw[1], netInfo.gw[2], netInfo.gw[3]);										\
  PRINT_STR(debugMessage);\
} while(0)

int32_t Ethernet_Init(Ethernet_t* eth, Net_t* device, Net_t* host){

	int8_t result = -1;

	eth_ptr = eth;

	wiz_NetInfo netInfo;

	setDeviceNet(device);
	setHostNet(host);

	memcpy(netInfo.mac, eth_ptr->device.mac, 6);
	memcpy(netInfo.ip, eth_ptr->device.ip, 4);
	memcpy(netInfo.sn, eth_ptr->device.sn, 4);
	memcpy(netInfo.gw, eth_ptr->device.gw, 4);

	PRINT_HEADER();

	reg_wizchip_cs_cbfunc(cs_sel, cs_desel);
	reg_wizchip_spi_cbfunc(spi_rb, spi_wb);

	result = wizchip_init(0, 0);

	if (result != 0){
		return result;
	}

	wizchip_setnetinfo(&netInfo); // set device ip .ect

	HAL_Delay(100);

	wizchip_getnetinfo(&netInfo); // get device ip etc.

	memcpy(eth_ptr->device.mac, netInfo.mac, 6);
	memcpy(eth_ptr->device.ip, netInfo.ip,  4);
	memcpy(eth_ptr->device.sn, netInfo.sn,  4);
	memcpy(eth_ptr->device.gw, netInfo.gw,  4);

	PRINT_NETINFO(eth_ptr->device);

	return result;
}

void setDeviceNet(Net_t* net){
	memcpy(eth_ptr->device.mac, net->mac, 6);
	memcpy(eth_ptr->device.ip, net->ip, 4);
	memcpy(eth_ptr->device.sn, net->sn, 4);
	memcpy(eth_ptr->device.gw, net->gw, 4);
	eth_ptr->device.outputPort = net->outputPort;
	eth_ptr->device.inputPort = net->inputPort;
}

void setHostNet(Net_t* net){
	memcpy(eth_ptr->host.mac, net->mac, 6);
	memcpy(eth_ptr->host.ip, net->ip, 4);
	memcpy(eth_ptr->host.sn, net->sn, 4);
	memcpy(eth_ptr->host.gw, net->gw, 4);
	eth_ptr->host.outputPort = net->outputPort;
	eth_ptr->host.inputPort = net->inputPort;
}

uint8_t isLinked(){

	return (getPHYCFGR() & PHYCFGR_LNK_ON);
}

// return transmited data size or error code
int32_t udp_send(Buffer_t* buf){

	int32_t ret = 0;
	uint16_t size;

	uint8_t sn = 0;

	switch (getSn_SR(sn)){

		case SOCK_UDP :

			 ret = sendto(sn, buf->data, buf->elements, eth_ptr->host.ip, eth_ptr->host.inputPort);
			  if (ret < 0)
			  {
				  PRINT_MSG_STR("UDP Send | Error data send: %li \r\n", ret);
				  if (ret == -13) //PRINT_STR("TIMEOUT... \r\n");
				  return ret;
			  }

			size = (uint16_t) ret;
			Buffer_clear(buf);
			PRINT_MSG_STR("UDP Send | Transmited data size: %i \r\n", size);
			break;

		case SOCK_CLOSED:
			if((ret = socket(sn, Sn_MR_UDP, eth_ptr->device.outputPort, 0x00)) != sn)
			{
				PRINT_MSG_INT("UDP Send | Socket opening error | %li \r\n", ret);
				return ret;
			}
			PRINT_MSG_INT("UDP Send | Open socket: %li \r\n", ret);
			break;

		 default :
			break;
	}
	 return ret;
}

// return received data size o error code
int32_t udp_read(Buffer_t* buf){

	int32_t  ret = 0;
	uint16_t size;

	uint8_t sn = 1;

	switch (getSn_SR(sn))
	{
		case SOCK_UDP :
			 if((size = getSn_RX_RSR(sn)) > 0)
			 {
				 if(size > buf->size) size = buf->size;
				 ret = recvfrom(sn, buf->data, size, eth_ptr->host.ip, &eth_ptr->host.outputPort);
				 if (ret <= 0)
				 {
					 PRINT_MSG_STR("UDP Read | Error data read: %li \r\n", ret);
					 return ret;
				 }
				 size = (uint16_t) ret;
				 buf->elements = ret;
				 PRINT_MSG_STR("UDP Read | Received data size: %i \r\n", size);
			 }
			 break;

		case SOCK_CLOSED:
			if((ret = socket(sn, Sn_MR_UDP, eth_ptr->device.inputPort, 0x00)) != sn)
			{
				 PRINT_MSG_INT("UDP Read | Error open socket: %li \r\n", ret);
				 return ret;
			}
			PRINT_MSG_INT("UDP Read | Open socket: %li \r\n", ret);
			break;

		 default :
			break;
	}
	return ret;
}
