


#ifndef __gpio_spi_h
#define __gpio_spi_h

#include "stdint.h"
#include "gpio_spi.h"
#include "N5162S.h"

				  //PA13
#define spi_get_miso()				((PA->PIN) & (1<<13))  // 若相应输入位为低则得到0，相应输入位为高则得到1
#define spi_set_clk_high()		((PA)->DOUT |= (1<<11))		  //时钟置高   PA11
#define spi_set_clk_low()			((PA)->DOUT &= ~(1<<11))	  //时钟置低
#define spi_set_mosi_hight()	((PA)->DOUT |= (1<<15))		  //发送脚置高	PA15
#define spi_set_mosi_low()		((PA)->DOUT &= ~(1<<15))		  //发送脚置低
#define spi_set_nss_high()		((PA)->DOUT |= (1<<9))		  //片选置高   PA9
#define spi_set_nss_low()			((PA)->DOUT &= ~(1<<9))		  //片选置低


#define PB0_GPIO()	((PB->PIN) & (1<<0)) ? 1 : 0
#define PB1_GPIO()	((PB->PIN) & (1<<1)) ? 1 : 0
#define PB2_GPIO()	((PB->PIN) & (1<<2)) ? 1 : 0
#define PB3_GPIO()	((PB->PIN) & (1<<3)) ? 1 : 0
#define PB4_GPIO()	((PB->PIN) & (1<<4)) ? 1 : 0

#define PA1_GPIO()	((PA->PIN) & (1<<1)) ? 1 : 0
#define PA3_GPIO()	((PA->PIN) & (1<<3)) ? 1 : 0
#define PA5_GPIO()	((PA->PIN) & (1<<5)) ? 1 : 0

#define LED_ON	PA->DOUT &= (~(1<<12))
#define LED_OFF	PA->DOUT |= (1<<12)


void drv_spi_init( void );
uint8_t drv_spi_read_write_byte( uint8_t TxByte );
void drv_spi_read_write_string( uint8_t* ReadBuffer, uint8_t* WriteBuffer, uint16_t Length );

void delay(uint32_t time);






#endif








