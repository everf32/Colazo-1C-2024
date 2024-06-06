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
#define PERIODA 5000000
#define PERIODB 4000000
#define NUMERO_TIRA_PIXEL 8
#define PIN_NEO_PIXEL GPIO_9
#define UPPER_THRESHOLD 8.0 // Umbral superior para temperatura
#define LOWER_THRESHOLD 2.0 // Umbral inferior para temperatura

/*==================[internal data definition]===============================*/
/**
 * @brief variable de tipo TaskHandle_t que se utiliza para manejar la tarea comparar.
 * 
*/
TaskHandle_t comparar_task_handle = NULL;
/**
 * @brief variable de tipo TaskHandle_t que se utiliza para manejar la tarea mostrar.
 * 
*/
TaskHandle_t mostrar_task_handle = NULL;
/**
 * @brief variable de tipo TaskHandle_t que se utiliza para manejar la tarea procesar.
 * 
*/
TaskHandle_t procesar_task_handle = NULL;
/**
 * @brief variable de tipo TaskHandle_t que se utiliza para manejar la tarea medir.
 * 
*/
TaskHandle_t medir_task_handle = NULL;
/**
 * @brief Variable utilizada para almacenar en tiempo real la temperatura medida.
 * 
*/
float temperatura;
/**
 * @brief Variable de tipo neopixel_color_t que representa un arreglo de 8 leds en una tira.
 * 
*/
static neopixel_color_t TiraLed[8];
/**
 * @brief Variable utilizada para almacenar en tiempo real el nivel de luz medida.
 * 
*/
uint16_t nivel_luz = 0;
/**
 * @brief Variable utilizada para almacenar en tiempo real la humedad recibido.
 * 
*/
float humedad;
/**
 * @brief Variable booleana que representa el estado del sistema de monitoreo.
 * 
*/
bool Encendido = false;
float promedio_temperatura = 0;
float promedio_humedad = 0;
float maximo_temperatura = 0;
float minimo_temperatura = 0;
float acumulador_temperatura = 0;
float acumulador_humedad = 0;
uint8_t contador_medicion = 0;

/*==================[internal functions declaration]=========================*/

/**
 * @fn void leerDato(uint8_t *dato)
 * @brief Función encargada de recibir los datos provenientes de la conexión bluetooth de baja energía(ble).
 * @param dato 
 * @brief Dato recibido mediante la conexión bluetooth de baja energía(ble).
 * 
*/

void leerDato(uint8_t *dato)
{
	switch (dato[0])
	{
	case 'A':
		Encendido = true;
		contador_medicion = 0;
		promedio_temperatura = 0;
		promedio_humedad = 0;
		maximo_temperatura = 0;
		minimo_temperatura = 0;
		acumulador_temperatura = 0;
		acumulador_humedad = 0;
		break;

	case 'a':
		Encendido = false;

		break;
	}
}

/**
 * @fn void FuncTimerMostrar()
 * @brief función encargada de enviar una notificación a las tareas de mostrar para que 
 * retomen su funcionamiento
*/
void FuncTimerMostrar()
{
	vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
}

/**
 * @fn void FuncTimerMedir_Comparar_Procesar()
 * @brief función encargada de enviar una notificación a las tareas de medir, comparar y procesar para que 
 * retomen su funcionamiento
*/

void FuncTimerMedir_Comparar_Procesar()
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(comparar_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(procesar_task_handle, pdFALSE);
}

/**
 * @brief Tarea encargada de mostrar los valores medidos de temperatura, humedad y luz, además de promedios, máximos
 * y mínimos cuando sean requeridos y de encender la tira led que indica estado de temperatura
*/

void mostrarTask()
{
	char msg1[25];
	char auxmsg1[10];
	char msg2[25];
	char auxmsg2[10];
	char msg3[25];
	char auxmsg3[10];
	neopixel_color_t arreglo_colores[8] = {NEOPIXEL_COLOR_RED, NEOPIXEL_COLOR_ORANGE, NEOPIXEL_COLOR_YELLOW,
										   NEOPIXEL_HUE_MAGENTA, NEOPIXEL_COLOR_TURQUOISE, NEOPIXEL_COLOR_CYAN, NEOPIXEL_COLOR_LGREEN, NEOPIXEL_COLOR_GREEN};
	uint8_t brillo_tira_led = 100;

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (Encendido)
		{
			// tarea encargada de encender la tira led para el valor de temperatura
			// en otra tarea deberia tener funciones que apagan los leds de la tira conforme baja cierto nivel de temperatura y sube, tengo que
			//  calcular las divisiones de cada aumento
			NeoPixelBrightness(brillo_tira_led);
			NeoPixelSetArray(arreglo_colores);

			/*----------------Envio los datos medidos por bluetooth-----------------*/
			strcpy(msg1, "");
			sprintf(auxmsg1, "*T%.2f\n*", temperatura);
			strcat(msg1, auxmsg1);
			BleSendString(msg1);

			// enviar humedad por bluetooth
			strcpy(msg2, "");
			sprintf(auxmsg2, "*H%.2f%%\n*", humedad);
			strcat(msg2, auxmsg2);
			BleSendString(msg2);

			// enviar luz por bluetooth
			strcpy(msg3, "");
			sprintf(auxmsg3, "*L%u\n*", nivel_luz);
			strcat(msg3, auxmsg3);
			BleSendString(msg3);
		}
		else
		{
			NeoPixelAllOff();
		}
	}
}

/**
 * @brief Tarea encargada de medir las distintas variables, como temperatura, humedad y luz.
*/

void medirTask()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (Encendido)
		{
			//--------------Medicion de luz-----------------
			AnalogInputReadSingle(CH0, &nivel_luz);
			//----------Medicion de temperatura-------------
			temperatura = Si7007MeasureTemperature();
			acumulador_temperatura += temperatura;
			//-------------Medicion de humedad--------------
			humedad = Si7007MeasureHumidity();

			acumulador_humedad += humedad;
			contador_medicion += 1;
		}
	}
}

/**
 * @brief Tarea encargada de procesar los datos medidos, calculando sus medias, máximos y mínimos
*/
void ProcesarTask()
{
	char msg4[25];
	char auxmsg4[10];
	char msg5[25];
	char auxmsg5[10];

	//  tarea encargada de aplicar diferentes procesamientos a las señales de entrada: temperatura,humedad, luz
	//  aca para el promedio de los datos puedo usar el estado de apagado, para luego de tocar en el celular para apagar
	//  y que no mida mas usar esos datos almacenados en un vector(definir tamaño) para calcular el promedio
	//  directamente acumular todos los valores en una variable

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (!Encendido)
		{
			/*----------------Porción encargada de calcular promedios-------------------*/
			promedio_temperatura = (acumulador_temperatura / contador_medicion);
			promedio_humedad = (acumulador_humedad / contador_medicion);

			/*-----------------Porción encargada de buscar máximos y mínimos--------------------*/
			if (maximo_temperatura < temperatura)
			{
				maximo_temperatura = temperatura;
			}
			if (minimo_temperatura > temperatura)
			{
				minimo_temperatura = temperatura;
			}

			/*---------------Porción encargada de enviar por bluetooth valores procesados-----------------*/
			strcpy(msg4, "");
			sprintf(auxmsg4, "*J%.2f\n*", promedio_temperatura);
			strcat(msg4, auxmsg4);
			BleSendString(msg4);

			strcpy(msg5, "");
			sprintf(auxmsg5, "*j %.2f\n*", promedio_humedad);
			strcat(msg5, auxmsg5);
			BleSendString(msg5);

			printf("Temperatura promedio %.2f \n", promedio_temperatura);
			printf("Humedad promedio %.2f \n", promedio_humedad);
			printf("Temperatura maxima %.2f \n", maximo_temperatura);
			printf("Temperatura minima %.2f \n", minimo_temperatura);
		}
	}
}
/**
 * @brief Tarea encargada de comparar los datos sensados con los umbrales, y disparar una alerta cuando la medición
 * de temperatura pase ciertos umbrales
*/

void compararTask()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (Encendido)
		{
			// tarea encargada de comparar valores actuales de temperatura, humedad y luz versus diferentes valores umbrales
			// ya seteados
			if (temperatura < 23)
			{
				// Apagar el buzzer
				BuzzerOff();
			}
			if (temperatura > 23)
			{
				// Encender buzzer
				BuzzerOn();
			}
			// falta definir como interpretar la luz que mide el ldr, capaz que  haya que aplicarle algun procesamiento a la señal que entra
		}
	}
}

/*====================[external functions definition]==========================*/
void app_main(void)
{
	/*------------------Definición de objeto bluetooth--------------------*/
	ble_config_t ble_config = {
		.device_name = "Seguridad de vacunas",
		.func_p = leerDato};

	/*------------------Definición de sensor temperatura humedad--------------------*/

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

	/*--------------------Definicion de timers---------------------*/

	timer_config_t TimerMostrar = {
		.timer = TIMER_B,
		.period = PERIODA,
		.func_p = FuncTimerMostrar,
		.param_p = NULL};

	timer_config_t TimerMedir_Comparar = {
		.timer = TIMER_C,
		.period = PERIODB,
		.func_p = FuncTimerMedir_Comparar_Procesar,
		.param_p = NULL};

	/*--------------------Inicializacion de dispositivos---------------------*/
	BleInit(&ble_config);
	AnalogInputInit(&entrada_analogica);
	Si7007Init(&medidorTemp_Hum);
	TimerInit(&TimerMedir_Comparar);
	TimerInit(&TimerMostrar);
	BuzzerInit(GPIO_18);
	NeoPixelInit(PIN_NEO_PIXEL, NUMERO_TIRA_PIXEL, &TiraLed);

	/*-------------------------creacion de tareas----------------------------*/
	xTaskCreate(&ProcesarTask, "procesar", 4096, NULL, 5, &procesar_task_handle);
	xTaskCreate(&mostrarTask, "mostrar", 2048, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&compararTask, "comparar", 4096, NULL, 5, &comparar_task_handle);
	xTaskCreate(&medirTask, "medir", 4096, NULL, 5, &medir_task_handle);

	/*-----------------------Inicialización de timers------------------------*/
	TimerStart(TimerMostrar.timer);
	TimerStart(TimerMedir_Comparar.timer);
}
/*==================[end of file]============================================*/