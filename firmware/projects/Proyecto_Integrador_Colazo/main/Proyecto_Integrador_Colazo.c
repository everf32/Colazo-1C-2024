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
#define PERIODA 100
#define PERIODB 200
#define PERIODC 300
#define PERIODD 400
#define UPPER_THRESHOLD 8.0 // Umbral superior para temperatura
#define LOWER_THRESHOLD 2.0 // Umbral inferior para temperatura



/*==================[internal data definition]===============================*/
TaskHandle_t conversorAD_task_handle = NULL;
TaskHandle_t comparar_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t procesar_task_handle = NULL;
uint8_t temperature = 0;
uint8_t temperatures[10] = {5 , 6 , 4 , 7 , 2 , 5 , 3 , 4 , 5 , 2};
bool On;

/*==================[internal functions declaration]=========================*/

void leerDato(uint8_t *dato, uint8_t tamanio)
{
	// Función a ejecutarse ante un interrupción de recepción a través de la conexión BLE.
}

void FuncTimerProcesar()
{
	vTaskNotifyGiveFromISR(procesar_task_handle, pdFALSE);
}

void FuncTimerMostrar()
{
	vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
}

void FuncTimerConversorAD()
{
	vTaskNotifyGiveFromISR(comparar_task_handle, pdFALSE);
}
void FuncTimerComparar()
{
	vTaskNotifyGiveFromISR(conversorAD_task_handle, pdFALSE);
}

void ProcesarTask()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// tarea encargada de aplicar diferentes procesamientos a las señales de entrada: temperatura,humedad, luz
	}
}

void compararTask()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		/* tarea encargada de comparar valores actuales de temperatura, humedad y luz versus diferentes valores umbrales
		ya seteados
		*/

		/*
			if (temperature > UPPER_THRESHOLD || temperature < LOWER_THRESHOLD)
			{
				// Encender el buzzer
				gpio_set_level(PIN_BUZZER, 1);
			}
			else
			{
				// Apagar el buzzer
				gpio_set_level(PIN_BUZZER, 0);
			}
		*/
	}
}

void mostrarTask()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// tarea encargada de encender la tira led para el valor de temperatura
	}
}

void conversorADTask()
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	// tarea dedicada a convertir los datos de temperatura y luz de analogico a digital

	/*
		temperature = analogRead(PIN_MCP9700) * (3.3 / 4095.0) * 100;
		humidity = dht_read_humidity();
		lightLevel = analogRead(PIN_LDR);
	*/
}

void medirTask()
{
	
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	ble_config_t ble_config = {
		.device_name = "Seguridad de vacunas",
		.func_p = leerDato};
	/*
		timer_config_t timerProcesar = {
			.timer = TIMER_A,
			.period = PERIODA,
			.func_p = FuncTimerProcesar,
			.param_p = NULL};

		timer_config_t timerComparar = {
			.timer = TIMER_B,
			.period = PERIODB,
			.func_p = FuncTimerComparar,
			.param_p = NULL};

		timer_config_t timerConversorAD = {
			.timer = TIMER_C,
			.period = PERIODC,
			.func_p = FuncTimerConversorAD,
			.param_p = NULL};

		timer_config_t TimerMostrar = {
			.timer = TIMER_D,
			.period = PERIODD,
			.func_p = FuncTimerMostrar,
			.param_p = NULL};
	*/
	/*--------------------Inicializacion de dispositivos---------------------*/

	/*

		NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
		NeoPixelAllOff();

	*/
	LedsInit();
	BleInit(&ble_config);

	/*Para probar la conexion bluetooth
	while (1)
	{
		vTaskDelay(PERIODD / portTICK_PERIOD_MS);
		switch (BleStatus())
		{
		case BLE_OFF:
			LedOn(LED_2);
			break;
		case BLE_DISCONNECTED:
			LedsOffAll();
			break;
		case BLE_CONNECTED:
			LedToggle(LED_1);
			break;
		}
	}
     */
	/*creacion de tareas*/
	xTaskCreate(&ProcesarTask, "procesar", 512, NULL, 5, &procesar_task_handle);
	xTaskCreate(&mostrarTask, "mostrar", 512, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&conversorADTask, "conversor", 512, NULL, 5, &conversorAD_task_handle);
	xTaskCreate(&compararTask, "comparar", 512, NULL, 5, &comparar_task_handle);

	/*Inicialización de timers*/
	// TimerStart(timerComparar.timer);
	// TimerStart(TimerMostrar.timer);
	// TimerStart(timerConversorAD.timer);
	// TimerStart(timerProcesar.timer);
}
/*==================[end of file]============================================*/