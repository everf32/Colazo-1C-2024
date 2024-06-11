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
 * |      Peripheral     |   ESP32   	|
 * |:-------------------:|:-------------|
 * | 	Sensor humedad	 | 	GPIO_1		|
 * | 	Sensor humedad	 | 	GPIO_1		|
 * | 	Sensor humedad	 | 	GPIO_1		|
 * | 	Sensor humedad	 | 	GPIO_1		|
 * | 	Sensor humedad	 | 	GPIO_1		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/03/2024 | Document creation		                         |
 *
 * @author Ever Colazo (everf97@gmail.com)
 *
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <gpio_mcu.h>
#include "analog_io_mcu.h"
#include <timer_mcu.h>
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/

#define GPIO_BOMBA_PHA GPIO_9;
#define GPIO_BOMBA_PHB GPIO_17;
#define GPIO_BOMBA_AGUA GPIO_18;
#define GPIO_HUMEDAD GPIO_1;
#define GPIO_PH GPIO_0;
#define PERIODO_MEDICION 3000000 // en microsegundos por el timer
#define PERIODO_INFORMAR 5000000
/*==================[internal data definition]===============================*/
bool On = false; // variable para corroborar el estado del sistema si esta encendido o apagado. Arranca apagado
bool humedad;
float ph = 0;
float umbral_pH_Inferior = 1.28;
float umbral_pH_Superior = 1.43;

TaskHandle_t medirTaskHandle = NULL;
TaskHandle_t informarTaskHandle = NULL;
TaskHandle_t regarTaskHandle = NULL;

/*==================[internal functions declaration]=========================*/

void escribirValorEnPc()
{
    UartSendString(UART_PC, "pH ");
    UartSendString(UART_PC, (char *)UartItoa(ph, 10));
    if(!humedad)
    {
        UartSendString(UART_PC, ", humedad correcta");
    }
}

void Encendido()
{
    On = true;
}

void Apagado()
{
    On = false;
}

FuncTimerMedir_Regar()
{
    /*ya que uso el mismo timer para medir y regar pongo para que se envie la notificacion desde esta misma funcion*/
    vTaskNotifyGiveFromISR(medirTaskHandle, pdFALSE);
    vTaskNotifyGiveFromISR(regarTaskHandle, pdFALSE);
}

FuncTimerInformar()
{
    vTaskNotifyGiveFromISR(informarTaskHandle, pdFALSE);
}

void medir() // tarea para realizar las mediciones de ph y humedad
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (On) // si on se encuentra en bajo no se mide
        {
            AnalogInputReadSingle(CH1, &ph);
            humedad = GPIORead(GPIO_HUMEDAD);
            /*se le asigna un valor a ph entre 0V y 3000mV. Tenemos como dato que el sensor tiene una salida desde
            0V a 3V para un rango de pH desde 0 a 14. Entonces 0V se corresponde a pH = 0 y 3V se corresponde a un
            pH=14
            Por lo tanto lo que se plantea es dividir entre 1000 el valor del voltaje asignado a pH y luego hacer regla de
            3 para calcular que voltaje se corresponde a un pH de 6 y a un pH de 6.7
            Calculando me da que un ph de 6 se corresponde con un voltaje de 1.28V y un pH de 6.7 se corresponde
            con un voltaje de 1.43V*/
            ph = ph / 1000;
        }
    }
}

void regar()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (On)
        {
            /*Para controlar la bomba de agua*/
            if (!humedad && (ph < umbral_pH_Superior) && (ph > umbral_pH_Inferior))
            {
                /*cuando la salida del sensor de humedad es 1, se detecta que el nivel de humedad es superior al deseado
                 y que el pH esta dentro del rango aceptable apago todos los sensores*/
                GPIOOff(GPIO_BOMBA_AGUA);
                GPIOOff(GPIO_BOMBA_PHA);
                GPIOOff(GPIO_BOMBA_PHB);
            }
            else if (humedad && (ph < umbral_pH_Superior) && (ph > umbral_pH_Inferior))
            {
                GPIOOn(GPIO_BOMBA_AGUA);
                GPIOOff(GPIO_BOMBA_PHB);
                GPIOOff(GPIO_BOMBA_PHA);
            }
            else if (!humedad && (ph > umbral_pH_Superior))
            {
                GPIOOff(GPIO_BOMBA_AGUA);
                GPIOOff(GPIO_BOMBA_PHB);
                GPIOOn(GPIO_BOMBA_PHA);
            }

            else if ((!humedad) && (ph < umbral_pH_Inferior))
            {
                /*cuando la salida del sensor de humedad es 0, se detecta que el nivel de humedad es inferior al deseado*/
                GPIOOff(GPIO_BOMBA_AGUA);
                GPIOOn(GPIO_BOMBA_PHB);
                GPIOOff(GPIO_BOMBA_PHA);
            }
            else if ((humedad) && (ph < umbral_pH_Inferior))
            {
                GPIOOn(GPIO_BOMBA_AGUA);
                GPIOOn(GPIO_BOMBA_PHB);
                GPIOOff(GPIO_BOMBA_PHA);
            }
        }
        else
        {
            GPIOOn(GPIO_BOMBA_AGUA);
            GPIOOn(GPIO_BOMBA_PHB);
            GPIOOff(GPIO_BOMBA_PHA);
        }
    }
}

void informar()
{
    while (true)
    {
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

    serial_config_t ControlarUart =
        {
            .port = UART_PC,
            .baud_rate = 115200,
            .func_p = NULL,
            .param_p = NULL,
        };

    analog_input_config_t EntradaAnalogica = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};

    timer_config_t timer_medir_regar = {
        .timer = TIMER_A,
        .period = PERIODO_MEDICION,
        .func_p = FuncTimerMedir_Regar,
        .param_p = NULL};

    timer_config_t timer_informar = {
        .timer = TIMER_B,
        .period = PERIODO_INFORMAR,
        .func_p = FuncTimerInformar,
        .param_p = NULL};

    /*Inicializacion de dispositivos o perifericos*/
    SwitchesInit();
    TimerInit(&timer_medir_regar);
    TimerInit(&timer_informar);
    UartInit(&ControlarUart);
    AnalogInputInit(&EntradaAnalogica);

    SwitchActivInt(SWITCH_1, &Encendido, NULL);

    SwitchActivInt(SWITCH_2, &Apagado, NULL);

    /*Creación de tareas*/
    xTaskCreate(&regar, "medir", 4096, NULL, 5, &medirTaskHandle);
    xTaskCreate(&medir, "regar", 4096, NULL, 5, &regarTaskHandle);
    xTaskCreate(&informar, "informar", 4096, NULL, 5, &informarTaskHandle);

    /*Arranque de timers*/
    TimerStart(timer_informar.timer);
    TimerStart(timer_medir_regar.timer);
}
/*==================[end of file]============================================*/