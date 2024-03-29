/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/03/2024 | Document creation		                         |
 *
 * @author Ever Colazo (everf97@live.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>	
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
uint8_t arregloGPIO [4] = {{GPIO_20 , GPIO_OUTPUT}, {GPIO_21,GPIO_OUTPUT},
                            {GPIO_22,GPIO_OUTPUT}, {GPIO_23,GPIO_OUTPUT}};

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	for(uint8_t i = 0 ; i<digits ; i++ )
    {
    bcd_number [i] = data %10;
    data = data /10;
    }
    return true;
}

void estadosBCD(uint8_t digitoBCD , gpioConf_t *vectorConStruct)
{

}

/*==================[external functions definition]==========================*/
void app_main(void)
{


}
/*==================[end of file]============================================*/