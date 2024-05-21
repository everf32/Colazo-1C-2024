/*! @mainpage Monitoreo de temperatura en caja transportadora de vacunas
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
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
 * | 15/05/2024 | Inicio del proyecto		                     |
 *
 * @author Ever Colazo (everf97@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include <led.h>
#include <gpio_mcu.h>
#include <ble_mcu.h>
#include <neopixel_stripe.h>
#include <ws2812b.h>
#include <timer_mcu.h>
#include <analog_io_mcu.h>

/*==================[macros and definitions]=================================*/
#define PERIOD 100

/*==================[internal data definition]===============================*/
TaskHandle_t conversorAD_task_handle = NULL;
TaskHandle_t comparar_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t procesar_task_handle = NULL;

bool On;

/*==================[internal functions declaration]=========================*/

void leerDato(uint8_t *dato, uint8_t tamanio)
{
	// funcion encargada de registrar los datos que se reciben por bluetooth
}

void FuncTimerProcesar()
{
	vTaskNotifyGiveFromISR(procesar_task_handle,pdFALSE);
}

void FuncTimerMostrar()
{
vTaskNotifyGiveFromISR(mostrar_task_handle,pdFALSE);
}

void FuncTimerConversorAD()
{
	vTaskNotifyGiveFromISR(comparar_task_handle,pdFALSE);
}
void FuncTimerComparar()
{
	vTaskNotifyGiveFromISR(conversorAD_task_handle,pdFALSE);
}

void ProcesarTask()
{
	while(true)
	{
	// tarea encargada de aplicar diferentes procesamientos a las señales de entrada: temperatura,humedad, luz
	}
}

void compararTask()
{
	while(true)
	{
	/* tarea encargada de comparar valores actuales de temperatura, humedad y luz versus diferentes valores umbrales
	ya seteados
	*/
	}
}

void mostrarTask()
{
	while(true)
	{
// tarea encargada de encender la tira led para el valor de temperatura
	}
	
}

void conversorADTask()
{
	// tarea dedicada a convertir los datos de temperatura y luz de analogico a digital
}
/*==================[external functions definition]==========================*/
void app_main(void)
{

	ble_config_t ble_configuration = {
		.device_name = "ESP_EDU_1",
		.func_p = leerDato};

	
		timer_config_t ProcesarTask = {
			.timer = TIMER_A,
			.period = PERIOD,
			.func_p = FuncTimerProcesar,
			.param_p = NULL};

		timer_config_t compararTask = {
			.timer = TIMER_B,
			.period = PERIOD,
			.func_p = FuncTimerComparar,
			.param_p = NULL};

		timer_config_t conversorADTask = {
			.timer = TIMER_C,
			.period = PERIOD,
			.func_p = FuncTimerConversorAD,
			.param_p = NULL};

			timer_config_t mostrarTaskTask = {
			.timer = TIMER_D,
			.period = PERIOD,
			.func_p = FuncTimerMostrar,
			.param_p = NULL};
	
	

	/*--------------------Inicializacion--------------------*/
	/*

		NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
		NeoPixelAllOff();

	*/
	BleInit(&ble_configuration);

	/*creacion de tareas*/
	xTaskCreate(&ProcesarTask, "procesar", 512, NULL, 5, &procesar_task_handle);
	xTaskCreate(&mostrarTask, "mostrar", 512, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&conversorADTask, "conversor", 512, NULL, 5, &conversorAD_task_handle);
	xTaskCreate(&compararTask, "comparar", 512, NULL, 5, &comparar_task_handle);
}
/*==================[end of file]============================================*/