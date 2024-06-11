/*! @mainpage Irrigación automática de plantas
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 *
 *
 * @section hardConn Hardware Connection
 *
 * |      Peripheral     |   ESP32   	|
 * |:-------------------:|:-------------|
 * | 	Sensor pH	     | 	GPIO_0		|
 * | 	Sensor humedad	 | 	GPIO_1		|
 * | 	Bomba de pHA	 | 	GPIO_9		|
 * | 	Bomba de pHB	 | 	GPIO_17		|
 * | 	Bomba agua  	 | 	GPIO_18		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/03/2024 | Creación y finalización de documentación	     |
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
/**
 * @brief Variable booleana que almacena el valor de encendido del sistema de irrigación.
 */
bool On = false;
/**
 * @brief Variable booleana que almacena el estado de la humedad, si es true hay poca humedad.
 */
bool humedad;
/**
 * @brief Variable flotante que almacena el valor leido de pH por el sensor.
 */
float ph = 0;
/**
 * @brief Variable flotante que contiene el valor umbral inferior para pH monitoreado.
 */
float umbral_pH_Inferior = 1.28; // se corresponde con un valor de pH de 6
/**
 * @brief Variable flotante que contiene el valor umbral superior para pH monitoreado.
 */
float umbral_pH_Superior = 1.43; // se corresponde con un valor de pH de 6.7
/**
 * @brief Elemento de tipo TaskHandle para manejar la tarea de medir
 */
TaskHandle_t medirTaskHandle = NULL;
/**
 * @brief Elemento de tipo TaskHandle para manejar la tarea de informar
 */
TaskHandle_t informarTaskHandle = NULL;
/**
 * @brief Elemento de tipo TaskHandle para manejar la tarea de regar
 */
TaskHandle_t regarTaskHandle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void escribirValorpHEnPc()
 * @brief Funcion encargada de enviar por comnunicación serie los valores de pH medidos
 */
void escribirValorpHEnPc()
{
    UartSendString(UART_PC, "pH: ");
    UartSendString(UART_PC, (char *)UartItoa(ph, 10));
    if (!humedad)
    {
        UartSendString(UART_PC, ", humedad correcta");
        UartSendString(UART_PC, "\r");
    }
    else
    {
        UartSendString(UART_PC, ", humedad correcta");
        UartSendString(UART_PC, "\r");
    }
}

/**
 * @fn void escribirEstadosBombasEnPc()
 * @brief Funcion encargada de enviar por comnunicación serie los estados de las bombas cuando se enseñen
 */
void escribirEstadoBombasEnPc()
{
    if (humedad)
    {
        UartSendString(UART_PC, "Bomba de agua encendida");
        UartSendString(UART_PC, "\r");
    }
    if (ph < umbral_pH_Inferior)
    {
        UartSendString(UART_PC, "Bomba pHB encendida");
        UartSendString(UART_PC, "\r");
    }
    if (ph > umbral_pH_Superior)
    {
        UartSendString(UART_PC, "Bomba phA encendida");
        UartSendString(UART_PC, "\r");
    }
}
/**
 * @fn void Encendido()
 * @brief Funcion encargada de setear a la bandera booleana On a verdadero.
 */
void Encendido()
{
    On = true;
}

/**
 * @fn void Encendido()
 * @brief Funcion encargada de setear a la bandera booleana On a falso.
 */
void Apagado()
{
    On = false;
}

/**
 * @fn void FuncTimerMedir_Regar()
 * @brief Funcion encargada de enviar la notificacion para que se reanuden las tareas medir y regar.
 */
FuncTimerMedir_Regar()
{
    /*ya que uso el mismo timer para medir y regar pongo para que se envie la notificacion desde esta misma funcion*/
    vTaskNotifyGiveFromISR(medirTaskHandle, pdFALSE);
    vTaskNotifyGiveFromISR(regarTaskHandle, pdFALSE);
}

/**
 * @fn void FuncTimerMedir_Regar()
 * @brief Funcion encargada de enviar la notificacion para que se reanude la tarea informar
 */
FuncTimerInformar()
{
    vTaskNotifyGiveFromISR(informarTaskHandle, pdFALSE);
}

/**
 * @brief Tarea encargada derealizar las mediciones de pH y humedad.
 */
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

/**
 * @brief Tarea encargada de comparar valores medidos de pH y humedad y realizar alguna acción.
 */
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

/**
 * @brief Tarea encargada de informr por UART los distintos estados de las bombas, valores de pH y de humedad.
 */
void informar()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (On)
        {
            escribirValorpHEnPc();
            escribirEstadoBombasEnPc();
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

    /*Definiciones de periféricos*/
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

    /*Funciones para activar las interrupciones cuando se presionen la teclas 1 y 2 de la placa*/
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