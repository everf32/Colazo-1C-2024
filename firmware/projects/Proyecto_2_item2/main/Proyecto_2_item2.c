/*! @mainpage Medidor de distancia por ultrasonido(Item 2)
 *
 * \section genDesc General Description
 * Esta aplicacion permite la detección de distancias mediante un sensor de ultrasonido, encendiendo leds para 
 * ciertos rangos de distancias y mostrando dicha distancia en una pantalla LCD mediante la utilizacion de tareas 
 * controladas por timers e interrupciones.
 * 
 *
 * @section changelog Changelog
 *
 * | Peripheral | ESP32                                          |
 * |:----------:|:-----------------------------------------------|
 * | lcditse0803|            		                             |
 * | HC-SR04    | GPIO_3                                         |
 * |
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
#include "hc_sr04.h"
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
/** @def REFRESCO_MEDICION 
 * @brief Representa el tiempo en microsegundos que estará comandando el disparo del timer 
 * que activa la tarea de medir
 * 
*/
#define REFRESCO_MEDICION 1000000

/** @def REFRESCO_DISPLAY
 * @brief Representa el tiempo en microsegundos que estará comandando el disparo del timer 
 * que activa la tarea de mostrar por display y control de Leds
 * 
*/
#define REFRESCO_DISPLAY 100000
/*==================[internal data definition]===============================*/
/**
 * @def Medir_task_handle 
 * @brief Objeto para el manejo de tarea medir
*/
TaskHandle_t Medir_task_handle = NULL;

/**
 * @def Mostrar_task_handle
 * @brief Objeto para el manejo de tarea mostrar
 * 
*/
TaskHandle_t Mostrar_task_handle = NULL;

/**
 * @def distancia 
 * @brief Variable global entera sin signo que almacena la distancia medida por el sensor de ultrasonido
*/
uint16_t distancia = 0;

/**
 * @def hold
 * @brief Variable global de tipo booleana que almacena el estado de "mantener" el último valor sensado
*/
bool hold;

/**
 * @def on
 * @brief Variable global de tipo booleana que almacena el estado de encendido del sistema de medición
*/
bool on;

/*==================[internal functions declaration]=========================*/
/**
 * @fn FuncTimerMedir()
 * @brief Envía una notificación a la tarea medir
*/
void FuncTimerMedir(void *param)
{
    vTaskNotifyGiveFromISR(Medir_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada a medir*/
}

/**
 * @fn FuncTimerMostrar()
 * @brief Envía una notificación a la tarea mostrar
*/
void FuncTimerMostrar(void *param)
{
    vTaskNotifyGiveFromISR(Mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al mostrar */
}

/**
 * @fn medirTask()
 * @brief Tarea dedicada a realizar las mediciones de distancia con el sensor de ultrasonido
*/
static void medirTask()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /*la tarea espera en este punto hasta recibir la notificacion*/
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
    }
}

/**
 * @fn mostrarTask()
 * @brief Tarea destinada a la realización de encender LEDs en función de la distancia sensada además de mostrar 
 * por LCD la distancia medida en el sensor.
*/
static void mostrarTask()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /*la tarea espera en este punto hasta recibir la notificacion*/
        if (on)
        {
            if (distancia < 10)
            {
                LedsOffAll();
            }
            else if ((distancia > 10) & (distancia < 20))
            {
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }

            else if ((distancia > 20) & (distancia < 30))
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            }

            else if (distancia > 30)
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }

            if (!hold)
            {
                LcdItsE0803Write(distancia);
            }
        }

        else
        {
            LcdItsE0803Off();
            LedsOffAll();
        }
    }
}

/**
 * @fn TeclaOn()
 * @brief Cambia el estado de la bandera booleana on
 * 
*/
void TeclaOn()
{
    on = !on;
}

/**
 * @fn TeclaHold()
 * @brief Cambia el estado de la bandera booleana Hold
 * 
*/
void TeclaHold()
{
    if (on)
    {
        hold = !hold;
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();

    /* Inicialización de timer medicion */
    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = REFRESCO_MEDICION,
        .func_p = FuncTimerMedir,
        .param_p = NULL};
    TimerInit(&timer_medicion);

    /* Inicialización de timer mostrar */
    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = REFRESCO_DISPLAY,
        .func_p = FuncTimerMostrar,
        .param_p = NULL};
    TimerInit(&timer_mostrar);

    /*En este caso, a partir de crear la interrupcion iniciada por el SWITCH_1 elijo que la funcion estadoTeclas
    se va a ejecutar*/
    SwitchActivInt(SWITCH_1, &TeclaOn, NULL);

    /*En este caso, a partir de crear la interrupcion iniciada por el SWITCH_2 elijo que la funcion estadoTeclas
    se va a ejecutar*/
    SwitchActivInt(SWITCH_2, &TeclaHold, NULL);

    /*creacion de tareas*/
    xTaskCreate(&medirTask, "medir", 512, NULL, 5, &Medir_task_handle);
    xTaskCreate(&mostrarTask, "mostrar", 512, NULL, 5, &Mostrar_task_handle);

    /*Inicio del conteo de timers*/
    TimerStart(timer_medicion.timer);
    TimerStart(timer_mostrar.timer);
}
