/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://promiedos.com.ar/...">Operation Example</a>
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
 * | 13/03/2024| Document creation		                         |
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
/*==================[macros and definitions]=================================*/

struct leds
{
uint8_t mode; //ON, OFF, TOGGLE 
uint8_t n_led; //indica el número de led a controlar
uint8_t n_ciclos; //cantidad de ciclos de encendido/apagado
uint16_t periodo // tiempo de cada ciclo
} my_leds;

#define MODE_ON 1
#define MODE_OFF 2
#define TOOGLE 3
#define led_1 1
#define led_2 2
#define led_3 3
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
/*funcion que me pide el item 3 del proyecto 1*/
void control_leds(struct leds *ptr_led){
	
	switch (ptr_led->mode)
	{
	case MODE_ON:  // cuando la variable mode es 1
	if(ptr_led->n_led == led_1)
	{
       LedOn(LED_1);
	}
	else if(ptr_led->n_led == led_2)
	{
		LedOn(LED_2);
	}
	else if(ptr_led->n_led == led_3 )
	{
		LedOn(LED_3);
	}
	break;
	case MODE_OFF: // cuando la variable mode es 0
	if(ptr_led->n_led == led_1)
    {
        LedOff(LED_1);
    }		
	else if(ptr_led->n_led == led_2)
	{
		LedOff(LED_2);
	}
	else if(ptr_led->n_led == led_3)
	{
		LedOff(LED_3);
	}
	break;
	
	case TOOGLE:
	uint8_t ciclos = 0;
	while(ciclos < ptr_led->n_ciclos) // este bucle se vuelve a ejecutar siempre y cuando i sea menor a n_ciclos 
	                             // almacenado en la estructura 
	{
		if(ptr_led->n_led == led_1)
    {
        LedToggle(LED_1);
    }		

	else if(ptr_led->n_led == led_2)
	{
		 LedToggle(LED_1);
	}

	else if(ptr_led->n_led == led_3)
	{
		 LedToggle(LED_1);
	}

     for(uint16_t retardo = 0 ; retardo < ptr_led->periodo / CONFIG_BLINK_PERIOD ; retardo++ ) // este for se ejecuta siempre y cuando
	 {                                                                                         // el retardo sea menos que el periodo de la estructura sobre el periodo de blinkeo de led
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	 }
	 
	ciclos++;

	}
	break;

	}

	}

/*==================[external functions definition]==========================*/
void app_main(void){
LedsInit();
my_leds.mode = TOOGLE;
my_leds.n_ciclos = 10;
my_leds.periodo = 500;
my_leds.n_led = led_3;
control_leds(&my_leds);
}
/*==================[end of file]============================================*/