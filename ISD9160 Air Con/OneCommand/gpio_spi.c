/** 软件SPI */

#include "stdint.h"
#include "ISD9100.h"
#include "gpio.h"
#include "gpio_spi.h"




void delay(uint32_t time)
{
	while(time)	time--;
}



/**
  * @brief :SPI初始化(软件)
  * @param :无
  * @note  :无
  * @retval:无
  */
void drv_spi_init( void )
{
//	 GPIO_SetMode(PA, 9, GPIO_MODE_OUTPUT );
//	 GPIO_SetMode(PA, 11, GPIO_MODE_OUTPUT );
//	 GPIO_SetMode(PA, 15, GPIO_MODE_OUTPUT );
//	 //GPIO_SetMode(PA, 13, GPIO_MODE_INPUT );
	
	
	PA->MODE|=(1<<9);
	PA->MODE|=(1<<11);
	PA->MODE|=(1<<15);
	PA->MODE&=(~(1<<13));
	// PA->MODE &= ~(3<<26);
	spi_set_nss_high();	
	spi_set_clk_high();
}

/**
  * @brief :SPI收发一个字节
  * @param :
  *			@TxByte: 发送的数据字节
  * @note  :非堵塞式，一旦等待超时，函数会自动退出
  * @retval:接收到的字节
*/
uint8_t drv_spi_read_write_byte( uint8_t TxByte )
{
	uint8_t i = 0, Data = 0;
	
	spi_set_clk_low( );
	delay(2500)	;
	for( i = 0; i < 8; i++ )			//一个字节8byte需要循环8次
	{
		/** 发送 */
		if( 0x80 == ( TxByte & 0x80 ))
		{
			spi_set_mosi_hight( );		//如果即将要发送的位为 1 则置高IO引脚
			delay(40)	;
		}
		else
		{
			spi_set_mosi_low( );		//如果即将要发送的位为 0 则置低IO引脚
			delay(40)	;
		}
		TxByte <<= 1;					//数据左移一位，先发送的是最高位
		
		spi_set_clk_high( );			//时钟线置高
		//__nop( );
		//__nop( ); __nop( );__nop( );__nop( );
		delay(5000)	;
		/** 接收 */
		Data <<= 1;						//接收数据左移一位，先接收到的是最高位
		if(spi_get_miso( ))
		{
			Data |= 0x01;				//如果接收时IO引脚为高则认为接收到 1
		}
		
		spi_set_clk_low( );				//时钟线置低
		//__nop( );__nop( ); __nop( );__nop( );__nop( );
		//__nop( ); __nop( );__nop( );__nop( );__nop( );
		delay(5000)	;
	}
	delay(2000)	;
	return Data;		//返回接收到的字节
}

/**
  * @brief :SPI收发字符串
  * @param :
  *			@ReadBuffer: 接收数据缓冲区地址
  *			@WriteBuffer:发送字节缓冲区地址
  *			@Length:字节长度
  * @note  :非堵塞式，一旦等待超时，函数会自动退出
  * @retval:无
  */
void drv_spi_read_write_string( uint8_t* ReadBuffer, uint8_t* WriteBuffer, uint16_t Length )
{
	spi_set_nss_low( );			//片选拉低
	spi_set_clk_low();
	while( Length-- )
	{
		*ReadBuffer = drv_spi_read_write_byte( *WriteBuffer );		//收发数据
		ReadBuffer++;
		WriteBuffer++;			//读写地址加1
	}
	
	spi_set_nss_high( );		//片选拉高
	spi_set_clk_high();
}


/** 软件SPI */

