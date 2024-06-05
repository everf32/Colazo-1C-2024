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
#include <string.h>
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
#include "analog_io_mcu.h"
#include "Si7007.h"
#include "buzzer.h"
#include "stdbool.h"

/*==================[macros and definitions]=================================*/
#define PERIODA 100
#define PERIODB 200
#define PERIODC 300
#define PERIODD 4000000
#define NUMERO_NEO_PIXEL 8
#define PIN_NEO_PIXEL GPIO_9
#define UPPER_THRESHOLD 8.0		   // Umbral superior para temperatura
#define LOWER_THRESHOLD 2.0		   // Umbral inferior para temperatura
#define LIGHT_UPPER_THRESHOLD 3000 // Umbral superior para la luz (valor ADC), del umbral inferior no hago uso

/*==================[internal data definition]===============================*/
TaskHandle_t conversorAD_task_handle = NULL;
TaskHandle_t comparar_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t procesar_task_handle = NULL;
TaskHandle_t medir_task_handle = NULL;
float temperature;
static neopixel_color_t TiraLed[8];
// uint8_t temperatures[10] = {5, 6, 4, 7, 2, 5, 3, 4, 5, 2};
uint16_t light_Level = 0;
float humidity;
bool Encendido = true;
uint8_t indice = 0;
uint8_t control = 0;

/*==================[internal functions declaration]=========================*/

void leerDato(uint8_t *dato)
{
	switch (dato[0])
	{
	case 'A':
		Encendido = true;
		break;

	case 'a':
		Encendido = false;
		break;
	}
}
/*
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
	vTaskNotifyGiveFromISR(comparar_task_handle, pdFALSE);
}
*/
void FuncTimerMedir()
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(comparar_task_handle, pdFALSE);
}

void ProcesarTask()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// tarea encargada de aplicar diferentes procesamientos a las señales de entrada: temperatura,humedad, luz
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
}

void medirTask()
{
	char msg1[25];
	char auxmsg1[10];
	char msg2[25];
	char auxmsg2[10];
	char msg3[25];
	char auxmsg3[10];
	char msg4[25];
	char auxmsg4[10];
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (Encendido)
		{
			//--------------Medicion de luz-----------------
			AnalogInputReadSingle(CH0, &light_Level);
			//----------Medicion de temperatura-------------
			temperature = Si7007MeasureTemperature();
			//-------------Medicion de humedad--------------
			humidity = Si7007MeasureHumidity();
			// enviar temperatura por bluetooth
			strcpy(msg1, "");
			sprintf(auxmsg1, "*T%.2f\n*", temperature);
			strcat(msg1, auxmsg1);
			BleSendString(msg1);
			// enviar humedad por bluetooth
			strcpy(msg2, "");
			sprintf(auxmsg2, "*H%.2f%%\n*", humidity);
			strcat(msg2, auxmsg2);
			BleSendString(msg2);
			// enviar luz por bluetooth
			strcpy(msg3, "");
			sprintf(auxmsg3, "*L%u\n*", light_Level);
			strcat(msg3, auxmsg3);
			BleSendString(msg3);

			// para mandar los datos de temperatura a una grafica en el celular
			strcpy(msg4, "");
			sprintf(auxmsg4, "*G%.2f\n*", temperature);
			strcat(msg4, auxmsg4);
			BleSendString(msg4);

			printf("Temperatura: %.2f %%°C \n humedad: %.2f %%\n luz: %u\n", temperature, humidity, light_Level);
		}
		else
			printf("Sistema apagado no mido \n");
	}
}

void compararTask()
{

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (Encendido)
		{
			// tarea encargada de comparar valores actuales de temperatura, humedad y luz versus diferentes valores umbrales
			// ya seteados
			if (temperature < 23)
			// if ((temperature < 8 && temperature > 2) && (humidity < 30) && light_Level)
			{
				// Apagar el buzzer
				BuzzerOff();
			
			}
			// else if ((temperature < 2 || temperature > 8) && (humidity > 30) && light_Level)
			if (temperature > 23)
			{
				// Encender buzzer
				BuzzerOn();

				NeoPixelRainbow(NEOPIXEL_HUE_RED,254,254,1);
				
			}

			// if (lightLevel > LIGHT_UPPER_THRESHOLD || lightLevel < LIGHT_LOWER_THRESHOLD) {
			//  Acciones específicas para la luz
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	
	ble_config_t ble_config = {
		.device_name = "Seguridad de vacunas",
		.func_p = leerDato};

	Si7007_config medidorTemp_Hum = {
		.select = GPIO_3,
		.PWM_1 = CH1,
		.PWM_2 = CH2};

	analog_input_config_t entrada_analogica = {
		.input = CH0,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};

		
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
	timer_config_t TimerMedir = {
		.timer = TIMER_A,
		.period = PERIODD,
		.func_p = FuncTimerMedir,
		.param_p = NULL};

	/*--------------------Inicializacion de dispositivos---------------------*/
	BleInit(&ble_config);
	AnalogInputInit(&entrada_analogica);
	Si7007Init(&medidorTemp_Hum);
	TimerInit(&TimerMedir);
	LedsInit();
	BuzzerInit(GPIO_16);
	NeoPixelInit(PIN_NEO_PIXEL, NUMERO_NEO_PIXEL, &TiraLed);
	NeoPixelAllOff();
	/*creacion de tareas*/
	// xTaskCreate(&ProcesarTask, "procesar", 512, NULL, 5, &procesar_task_handle);
	// xTaskCreate(&mostrarTask, "mostrar", 512, NULL, 5, &mostrar_task_handle);
	// xTaskCreate(&conversorADTask, "conversor", 512, NULL, 5, &conversorAD_task_handle);
	xTaskCreate(&compararTask, "comparar", 4096, NULL, 5, &comparar_task_handle);
	xTaskCreate(&medirTask, "medir", 4096, NULL, 5, &medir_task_handle);

	/*Inicialización de timers*/
	// TimerStart(timerComparar.timer);
	// TimerStart(TimerMostrar.timer);
	// TimerStart(timerConversorAD.timer);
	// TimerStart(timerProcesar.timer);
	TimerStart(TimerMedir.timer);
}
/*==================[end of file]============================================*/