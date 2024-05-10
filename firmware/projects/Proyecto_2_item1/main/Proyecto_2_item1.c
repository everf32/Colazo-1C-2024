/*! @mainpage Medidor de distancia por ultrasonido
 *
 * \section genDesc General Description
 *La aplicación muestra la distancia medida por un sensor de ultraosonido por un display LCD, mediante
 la utilización de tareas 
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/04/2024 | Comienza el archivo                            |
 * | 10/05/2024 | Finaliza la documentación del proyecto         |
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
/*==================[macros and definitions]=================================*/
/** @def REFRESCO_TECLAS 
 * @brief Representa el tiempo en microsegundos que se utilizará para dar un delay a la tarea teclas
 * 
*/
#define REFRESCO_TECLAS 50

/** @def REFRESCO_MEDICION 
 * @brief Representa el tiempo en microsegundos que se utilizará para dar un delay a la tarea medir
 * 
*/
#define REFRESCO_MEDICION 1000
/** @def REFRESCO_DISPLAY 
 * @brief Representa el tiempo en microsegundos que se utilizará para dar un delay a la tarea medir
 * 
*/
#define REFRESCO_DISPLAY 100
/*==================[internal data definition]===============================*/
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

static void medirTask(void *pvParameter)
{ // en esta tarea se mide las distancias por el sensor y se modifica el estado de los leds dependiendo lo que se este midiendo
    while (true)
    {
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
        vTaskDelay(REFRESCO_MEDICION / portTICK_PERIOD_MS);
    }
}

static void mostarTask(void *pvParameter)
{
    while (true)
    {
        if (on)
        {
            // aca se realizan las tareas de encender leds dependiendo la distancia y encender el display
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
        vTaskDelay(REFRESCO_DISPLAY / portTICK_PERIOD_MS);
    }
}

static void teclasTask(void *pvParameter)
{
    uint8_t teclas;
    while (true)
    {
        teclas = SwitchesRead();
        switch (teclas)
        {
        case SWITCH_1:
            on = !on;
            break;
        case SWITCH_2:
            hold = !hold;
            break;
        }
        vTaskDelay(REFRESCO_TECLAS / portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();

    xTaskCreate(&medirTask, "medir", 512, NULL, 5, NULL);
    xTaskCreate(&teclasTask, "mostrar", 512, NULL, 5, NULL);
    xTaskCreate(&mostarTask, "teclas", 512, NULL, 5, NULL);
}
