/*
 * XMEGA_A3BU.c
 *
 * Created: 02.01.2025 12:08:44
 * Author : Ivan Prints
 */ 

#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "spic_driver.h"
#include "twie_driver.h"
#include "gpio_driver.h"
#include "debugSerial.h"
#include "sysTickTimer.h"
#include "rtc.h"
#include "socket.h"
#include "netHandler.h"
#include "canspi.h"

rtc_date sys_rtc = {
	.date = 4,
	.month = 1,
	.year = 25,
	.dayofweek = 6,
	.hour = 0,
	.minute = 8,
	.second = 00
};


gpio hbRedLed = {(uint8_t *)&PORTF , PIN6_bp};
	
gpio in1 = {(uint8_t *)&PORTB , PIN0_bp};
gpio in2 = {(uint8_t *)&PORTB , PIN1_bp};
gpio rel1 = {(uint8_t *)&PORTB , PIN2_bp};
gpio rel2 = {(uint8_t *)&PORTB , PIN3_bp};


uCAN_MSG txMessage;
uCAN_MSG rxMessage;
	

/*
MCU - OK
SPIC - (LAN, CAN, SD) - OK
LAN - OK
CAN - OK
GPIO - OK
UART - OK (printf)
TWIE
	RTC
	i2c-rtc INT pin handler
move index.html to PROGMEM

TODO -
	SD Card
	RTC mcu	
*/
uint8_t tmpData;

int main(void)
{
	
	OSC.CTRL |= OSC_RC32MEN_bm;
	while ((OSC.STATUS & OSC_RC32MRDY_bm) == 0)
	_PROTECTED_WRITE(CLK.CTRL, CLK_SCLKSEL_RC32M_gc);
	gpio_set_pin_direction(&hbRedLed , PORT_DIR_OUT); 
	gpio_set_pin_direction(&in1 , PORT_DIR_IN); 
	gpio_set_pin_direction(&in2 , PORT_DIR_IN);
	gpio_set_pin_direction(&rel1 , PORT_DIR_OUT);
	gpio_set_pin_direction(&rel2 , PORT_DIR_OUT);
	
	gpio_set_pin_level(&hbRedLed, true);
	gpio_set_pin_level(&hbRedLed, false);
	
	sei();
	debugSerialInit(DEB_SERIAL_BAUD);
	initSpic();
	netInit();
	CANSPI_Initialize();
	twieInit();
	rtc_int_enable();
	//rtc_set(&sys_rtc);
	sysTickInit();
	
	while (1) 
    {
		gpio_toggle_pin_level(&hbRedLed);
		
		gpio_set_pin_level(&rel1, gpio_get_pin_level(&in1));
		gpio_set_pin_level(&rel2, gpio_get_pin_level(&in2));
		
		//_delay_ms(20);
		
		
		 if(CANSPI_Receive(&rxMessage))
		 {
			 gpio_toggle_pin_level(&hbRedLed);
		 }
		tmpData = getRxByte();
		uint8_t tmpChr;
		if(tmpData != 0){
			tmpChr = 10;
		}

		if(getRtcInt()){
			rtc_sync(&sys_rtc);
			printf("RTC %d-%d-%d %d; %d:%d:%d %d; %d \n", sys_rtc.date, sys_rtc.month, sys_rtc.year, sys_rtc.dayofweek, sys_rtc.hour, sys_rtc.minute, sys_rtc.second, getRxByte(), getUpTime());
			
			txMessage.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B;
			txMessage.frame.id = 0x0A;
			txMessage.frame.dlc = 8;
			txMessage.frame.data0 = 0;
			txMessage.frame.data1 = 1;
			txMessage.frame.data2 = 2;
			txMessage.frame.data3 = 3;
			txMessage.frame.data4 = 4;
			txMessage.frame.data5 = 5;
			txMessage.frame.data6 = 6;
			txMessage.frame.data7 = 7;
			CANSPI_Transmit(&txMessage);
		}
		
		
		netHandler();
		
    }
	
	return 0;
}

