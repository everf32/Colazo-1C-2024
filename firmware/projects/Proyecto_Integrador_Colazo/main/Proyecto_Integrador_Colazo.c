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

/*==================[internal data definition]===============================*/
TaskHandle_t conversorAD_task_handle = NULL;
TaskHandle_t comparar_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t procesar_task_handle = NULL;
bool On;

/*==================[internal functions declaration]=========================*/

void readData(uint8_t *data, uint8_t length)
{
}

void ProcesarTask()
{
	// tarea encargada de aplicar diferentes procesamientos a las señales de entrada: temperatura,humedad, luz
}

void compararTask()
{
	/* tarea encargada de comparar valores actuales de temperatura, humedad y luz versus diferentes valores umbrales
	ya seteados
	*/
}

void mostrarTask()
{
	// tarea encargada de encender la tira led para el valor de temperatura
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
		.func_p = readData};
		
	/*
		timer_config_t timer_senial = {
			.timer = TIMER_B,
			.period = T_SENIAL * CHUNK,
			.func_p = FuncTimerSenial,
			.param_p = NULL};

		timer_config_t timer_senial = {
			.timer = TIMER_B,
			.period = T_SENIAL * CHUNK,
			.func_p = FuncTimerSenial,
			.param_p = NULL};

		timer_config_t timer_senial = {
			.timer = TIMER_B,
			.period = T_SENIAL * CHUNK,
			.func_p = FuncTimerSenial,
			.param_p = NULL};

	*/
	// Inicializacion
	/*
		NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
		NeoPixelAllOff();
		BleInit(&ble_configuration);
	*/

	/*creacion de tareas*/
	xTaskCreate(&ProcesarTask, "procesar", 512, NULL, 5, &procesar_task_handle);
	xTaskCreate(&mostrarTask, "mostrar", 512, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&conversorADTask, "conversor", 512, NULL, 5, &conversorAD_task_handle);
	xTaskCreate(&compararTask, "comparar", 512, NULL, 5, &comparar_task_handle);
}
/*==================[end of file]============================================*/