/*! @mainpage Monitoreo de temperatura en caja transportadora de vacunas
 *
 * @section genDesc General Description
 *
 * Esta aplicación realiza la medición en tiempo real de la temperatura,humedad y luz en una caja de vacunas,
 * alertando cuando la temperatura esté fuera del rango establecido(2°C-8°C) mediante el sonido de un Buzzer. Además mediante la conexión ble
 * permite el monitoreo remoto desde una aplicación desde un dispositivo inteligente como un smartphone o tablet.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	LDR  	 	| 	GPIO_0		|
 * | 	Si7007	 	| 	GPIO_3		|
 * | Neopixel Stripe| 	GPIO_9		|
 * | 	Buzzer	 	| 	GPIO_18		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/05/2024 | Inicio del proyecto		                     |
 * |05/06/2024  |Inicio de documentación                         |
 * |            |Fin de documentación                            |
 *
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
#define PERIODO 1000000
#define GPIO_SENSOR_HUMEDAD_TEMPERATURA GPIO_3
#define GPIO_NEO_PIXEL GPIO_9
#define GPIO_BUZZER GPIO_18
#define NUMERO_TIRA_PIXEL 8
#define UMBRAL_SUPERIOR_TEMPERATURA 8.0 // Umbral superior para temperatura
#define UMBRAL_INFERIOR_TEMPERATURA 2.0 // Umbral inferior para temperatura
#define NEOPIXEL_COLOR_OFF 0x00000000

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
 * @brief Variable utilizada para almacenar en tiempo real el nivel de luz medida.
 *
 */
uint16_t nivel_luz = 0;
/**
 * @brief Variable utilizada para almacenar en tiempo real la humedad medida.
 *
 */
float humedad;
/**
 * @brief Variable booleana que representa el estado del sistema de monitoreo.
 *
 */
bool Encendido = false;
/**
 * @brief Variable encargada de almacenar el promedio de temperatura medido en un cierto intervalo de tiempo.
 *
 */
float promedio_temperatura = 0;
/**
 * @brief Variable encargada de almacenar el promedio de humedad medido en un cierto intervalo de tiempo.
 *
 */
float promedio_humedad = 0;
/**
 * @brief Variable encargada de almacenar el promedio de luz medido en un cierto intervalo de tiempo.
 *
 */
uint16_t promedio_luz = 0;
/**
 * @brief Variable encargada de almacenar el máximo valor de temperatura medido en el tiempo en el cual se registraron
 * los datos.
 */
float maximo_temperatura = 0;
/**
 * @brief Variable encargada de almacenar el mínimo valor de temperatura medido en el tiempo en el cual se registraron
 * los datos.
 */
float minimo_temperatura = 100;
/**
 * @brief Variable auxiliar encargada de almacenar el valor acumulado de temperatura mientras la aplicación se encuentra
 * funcionando.
 */
float acumulador_temperatura = 0;
/**
 * @brief Variable auxiliar encargada de almacenar el valor acumulado de humedad mientras la aplicación se encuentra
 * funcionando.
 */
float acumulador_humedad = 0;
/**
 * @brief Variable auxiliar encargada de almacenar el número de ciclos en los cuales se realizaron mediciones.
 * Se utilizan para calcular el promedio.
 */
uint8_t contador_medicion = 0;
/**
 * @brief Variable auxiliar encargada de almacenar el valor acumulado de luz mientras la aplicación se encuentra
 * funcionando.
 */
uint16_t acumulador_luz = 0;
/**
 * @brief Matriz estática de colores para controlar la tira de LEDs. No se modifica nunca, es una matriz con los 
 * colores que contiene la tira LED original.
 */
static neopixel_color_t arreglo_colores[8] = {NEOPIXEL_COLOR_GREEN, NEOPIXEL_COLOR_GREEN, NEOPIXEL_COLOR_LGREEN, NEOPIXEL_COLOR_YELLOW,
											  NEOPIXEL_COLOR_YELLOW, NEOPIXEL_COLOR_ORANGE, NEOPIXEL_COLOR_RED, NEOPIXEL_COLOR_RED};
/**
 * @brief Matriz auxiliar de colores en la cual los valores van a variar en función de las temperaturas medidas.
 */
neopixel_color_t arreglo_colores_dinamico[8] = {NEOPIXEL_COLOR_GREEN, NEOPIXEL_COLOR_GREEN, NEOPIXEL_COLOR_LGREEN, NEOPIXEL_COLOR_YELLOW,
												NEOPIXEL_COLOR_YELLOW, NEOPIXEL_COLOR_ORANGE, NEOPIXEL_COLOR_RED, NEOPIXEL_COLOR_RED};
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
		/*Cuando inicio el sistema seteo todos los valores de las variables a 0 excepto el minimo de temperatura*/
		contador_medicion = 0;
		promedio_temperatura = 0;
		promedio_humedad = 0;
		maximo_temperatura = 0;
		minimo_temperatura = 100;
		acumulador_temperatura = 0;
		acumulador_humedad = 0;
		acumulador_luz = 0;
		BuzzerOff();

		/*Vuelvo a enviar por bluetooth los datos cuando enciendo para que no me queden en pantalla los
		valores viejos de promedio, maximos y mimimos*/
		char msg1[25];
		char auxmsg1[10];
		char msg2[25];
		char auxmsg2[10];
		char msg3[25];
		char auxmsg3[10];
		char msg4[25];
		char auxmsg4[10];
		char msg5[25];
		char auxmsg5[10];

		strcpy(msg4, "");
		sprintf(auxmsg1, "*J%d\n*", 0);
		strcat(msg1, auxmsg1);
		BleSendString(msg1);

		strcpy(msg2, "");
		sprintf(auxmsg2, "*j %d\n*", 0);
		strcat(msg2, auxmsg2);
		BleSendString(msg2);

		strcpy(msg3, "");
		sprintf(auxmsg3, "*b%d\n*", 0);
		strcat(msg3, auxmsg3);
		BleSendString(msg3);

		strcpy(msg4, "");
		sprintf(auxmsg4, "*B%d\n*", 0);
		strcat(msg4, auxmsg4);
		BleSendString(msg4);
		break;

	case 'a':
		Encendido = false;
		break;
	}
}

/**
 * @fn void FuncTimerMedir_Comparar_Procesar()
 * @brief Función encargada de enviar una notificación a las tareas de medir, comparar y procesar para que
 * retomen su funcionamiento.
 */

void FuncTimerGlobal()
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(comparar_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(procesar_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
}

/**
 * @brief Tarea encargada de mostrar los valores medidos de temperatura, humedad y luz, además de promedios, máximos
 * y mínimos cuando sean requeridos y de encender la tira led que indica estado de temperatura.
 */

void mostrarTask()
{
	char msg1[25];
	char auxmsg1[10];
	char msg2[25];
	char auxmsg2[10];
	char msg3[25];
	char auxmsg3[20];
	uint8_t brillo_tira_led = 100;

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (Encendido)
		{
			NeoPixelBrightness(brillo_tira_led);
			NeoPixelSetArray(arreglo_colores_dinamico);

			/*Acá cambio los leds prendidos de la tira en función de la temperatura*/
			if (((temperatura > 5 && temperatura < 5.5)) || ((temperatura < 5) && (temperatura > 4.5)))
			{
				for (uint8_t i = 0; i < 7; i++)
				{
					arreglo_colores_dinamico[i] = arreglo_colores[i];
				}
			}
			if (((temperatura > 5.5 && temperatura < 6)) || ((temperatura < 4.5) && (temperatura > 4)))
			{
				arreglo_colores_dinamico[0] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[1] = NEOPIXEL_COLOR_OFF;

				for (uint8_t i = 2; i < 7; i++)
				{
					arreglo_colores_dinamico[i] = arreglo_colores[i];
				}
			}
			if ((temperatura > 6 && temperatura < 6.5) || (temperatura < 4 && temperatura > 3.5))
			{
				arreglo_colores_dinamico[0] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[1] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[2] = NEOPIXEL_COLOR_OFF;

				for (uint8_t i = 3; i < 7; i++)
				{
					arreglo_colores_dinamico[i] = arreglo_colores[i];
				}
			}
			if ((temperatura > 6.5 && temperatura < 7) || (temperatura < 3.5 && temperatura > 3))
			{
				arreglo_colores_dinamico[0] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[1] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[2] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[3] = NEOPIXEL_COLOR_OFF;
				for (uint8_t i = 4; i < 7; i++)
				{
					arreglo_colores_dinamico[i] = arreglo_colores[i];
				}
			}
			if ((temperatura > 7 && temperatura < 7.5) || (temperatura < 3 && temperatura > 2.5))
			{
				arreglo_colores_dinamico[0] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[1] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[2] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[3] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[4] = NEOPIXEL_COLOR_OFF;
				for (uint8_t i = 5; i < 7; i++)
				{
					arreglo_colores_dinamico[i] = arreglo_colores[i];
				}
			}
			if ((temperatura > 7.5 && temperatura < 8) || (temperatura < 2.5 && temperatura > 2))
			{
				arreglo_colores_dinamico[0] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[1] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[2] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[3] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[4] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[5] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[6] = NEOPIXEL_COLOR_OFF;
				arreglo_colores_dinamico[6] = arreglo_colores[6];
				arreglo_colores_dinamico[7] = arreglo_colores[7];
			}

			if (temperatura > 8 || temperatura < 2)
			{
				for (uint8_t i = 0; i < 7; i++)
				{
					arreglo_colores_dinamico[0] = NEOPIXEL_COLOR_OFF;
					arreglo_colores_dinamico[1] = NEOPIXEL_COLOR_OFF;
					arreglo_colores_dinamico[2] = NEOPIXEL_COLOR_OFF;
					arreglo_colores_dinamico[3] = NEOPIXEL_COLOR_OFF;
					arreglo_colores_dinamico[4] = NEOPIXEL_COLOR_OFF;
					arreglo_colores_dinamico[5] = NEOPIXEL_COLOR_OFF;
					arreglo_colores_dinamico[6] = NEOPIXEL_COLOR_OFF;
				}
			}

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
			sprintf(auxmsg3, "*L%d %%\n*", nivel_luz);
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
			nivel_luz = (nivel_luz * 100) / 3300; // para pasar el nivel de luz a un porcentaje
			acumulador_luz += nivel_luz;

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
	char msg6[25];
	char auxmsg6[10];
	char msg7[25];
	char auxmsg7[10];

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		/*-----------------Porción encargada de buscar máximos y mínimos--------------------*/
		if (maximo_temperatura < temperatura)
		{
			maximo_temperatura = temperatura;
		}
		if (minimo_temperatura > temperatura)
		{
			minimo_temperatura = temperatura;
		}
		if (!Encendido)
		{
			/*----------------Porción encargada de calcular promedios-------------------*/
			promedio_temperatura = (acumulador_temperatura / contador_medicion);
			promedio_humedad = (acumulador_humedad / contador_medicion);
			promedio_luz = (acumulador_luz / contador_medicion);

			/*---------------Porción encargada de enviar por bluetooth valores procesados-----------------*/
			strcpy(msg4, "");
			sprintf(auxmsg4, "*J%.2f\n*", promedio_temperatura);
			strcat(msg4, auxmsg4);
			BleSendString(msg4);

			strcpy(msg5, "");
			sprintf(auxmsg5, "*j %.2f\n*", promedio_humedad);
			strcat(msg5, auxmsg5);
			BleSendString(msg5);

			strcpy(msg7, "");
			sprintf(auxmsg7, "*h%d\n*", promedio_luz);
			strcat(msg7, auxmsg7);
			BleSendString(msg7);

			strcpy(msg6, "");
			sprintf(auxmsg6, "*b%.2f\n*", maximo_temperatura);
			strcat(msg6, auxmsg6);
			BleSendString(msg6);

			strcpy(msg6, "");
			sprintf(auxmsg6, "*B%.2f\n*", minimo_temperatura);
			strcat(msg6, auxmsg6);
			BleSendString(msg6);
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
			if ((temperatura < UMBRAL_SUPERIOR_TEMPERATURA) && (temperatura > UMBRAL_INFERIOR_TEMPERATURA))
			{
				// Apagar el buzzer
				BuzzerOff();
			}

			else if ((temperatura > UMBRAL_SUPERIOR_TEMPERATURA) || (temperatura < UMBRAL_INFERIOR_TEMPERATURA))
			{
				/*Encender el buzzer para alarmar*/
				BuzzerPlayTone(4000, 750);
			}
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
		.select = GPIO_SENSOR_HUMEDAD_TEMPERATURA,
		.PWM_1 = CH1,
		.PWM_2 = CH2};

	/*------------------Definición de entrada analógica--------------------*/
	analog_input_config_t entrada_analogica = {
		.input = CH0,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};

	static neopixel_color_t TiraLed[8];

	/*--------------------Definicion de timers---------------------*/

	timer_config_t TimerGlobal = {
		.timer = TIMER_C,
		.period = PERIODO,
		.func_p = FuncTimerGlobal,
		.param_p = NULL};

	/*--------------------Inicializacion de dispositivos---------------------*/
	BleInit(&ble_config);
	AnalogInputInit(&entrada_analogica);
	Si7007Init(&medidorTemp_Hum);
	TimerInit(&TimerGlobal);
	BuzzerInit(GPIO_BUZZER);
	NeoPixelInit(GPIO_NEO_PIXEL, NUMERO_TIRA_PIXEL, &TiraLed);

	/*-------------------------creacion de tareas----------------------------*/
	xTaskCreate(&ProcesarTask, "procesar", 4096, NULL, 5, &procesar_task_handle);
	xTaskCreate(&mostrarTask, "mostrar", 4096, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&compararTask, "comparar", 4096, NULL, 5, &comparar_task_handle);
	xTaskCreate(&medirTask, "medir", 4096, NULL, 5, &medir_task_handle);

	/*-----------------------Inicialización de timers------------------------*/
	TimerStart(TimerGlobal.timer);
}
/*==================[end of file]============================================*/