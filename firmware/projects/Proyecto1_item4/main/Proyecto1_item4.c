/** @mainpage Proyecto 1
 *
 * \section genDesc General Description
 *
 *Esta aplicacion permite la visualización de una serie de dígitos en una pantalla LCD
 *
 * \section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                       |
 * |:----------:|:--------------------------------------------------|
 * | 20/03/2024 | Primeras inicializaciones de funciones necesarias	|
 * | 03/04/2024 | Finalizacion de las ultimas etapas y documentacion|
 *
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
#include "switch.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
/** @def HIGH
 * @brief Estado encendido o alto
 */
#define HIGH 1

/** @def LOW
 * @brief Estado encendido o alto
 */
#define LOW 0

/**
 * @brief Estructura que representa un puerto GPIO, con un número de pin y una dirección
 */
typedef struct
{
    /**
     * @brief Contiene el número de pin del correspondiente puerto GPIO
     *
     */
    gpio_t pin; /*!< GPIO pin number */
    /**
     * @brief Contiene la dirección(salida/entrada) del correspondiente puerto GPIO.
     *
     */
    io_t dir; /*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/** @fn void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
 * @brief Convierte el dato recibido a BCD, guarda cada uno de los dígitos de salida
 * en el arreglo que recibe como puntero
 * @param data Dato entero sin signo de 32 bits, representa el número en decimal a convertir en BCD
 * @param digits Dato entero sin signo de 8 bits, representa la cantidad de digitos del dato
 * decimal  a representar en BCD
 * @param bcd_number Puntero a un arreglo de enteros sin signo de 8 bits que almacenará los
 * n dígitos del dato
 *
 */
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
    for (uint8_t i = 0; i < digits; i++)
    {
        bcd_number[i] = data % 10;
        data /= 10;
    }
}

/** @fn void cambiarEstadosGPIO(uint8_t digitoBCD, gpioConf_t *vectorGpioConf_t)
 * @brief Cambia el estado de cada GPIO, puede ser a "0" o a "1", según el estado del bit
 * correspondiente en el BCD ingresado
 * @param digitoBCD Dato entero sin signo de 8 bits, representa un dígito en BCD
 * @param vectorGpioConf_t Un puntero a un arreglo de estructuras del tipo gpioConf_t
 *
 */

void cambiarEstadosGPIO(uint8_t digitoBCD, gpioConf_t *vectorGpioConf_t)
{

    for (uint8_t i = 0; i < 4; i++)
    {
        /*Utilizo una máscara, corriendo 1 i veces y comparando en el for cada dígito y dependiendo
        el resultado de la operación and seteo el estado de los puertosGPIO correspondiente*/
        if (digitoBCD & (1 << i))
        {
            GPIOState(vectorGpioConf_t[i].pin, HIGH);
        }
        else
        {
            GPIOState(vectorGpioConf_t[i].pin, LOW);
        }
    }
}

/** @fn void mostrarEnDisplay(uint32_t dato, uint8_t digitos_salida, gpioConf_t *vectorGpioConf, gpioConf_t *vectorPuertosLCD)
 * @brief Setea los pines GPIO_9, GPIO_18, GPIO_19 utilizando en funcion en funcion de los
 * del dato y la cantidad de dígitos que este posea, y utilizando otras funcionalidades, muestra por LCD el 
 * dato ingresado.
 * @param dato Dato entero sin signo de 32 bits, representa el número en decimal a mostrar por LCD
 * @param digitos_salida Dato entero sin signo de 8 bits, representa la cantidad de digitos del dato
 * decimal  a mostrar por pantalla LCD
 * @param vectorGpioConf Puntero a un arreglo de estructuras del tipo gpioConf_t que contiene los pines GPIO_20
 * GPIO_21, GPIO_22 y GPIO_23.
 * @param vectorPuertosLCD Puntero a un arreglo de estructuras del tipo gpioConf_t que contiene los pines GPIO_9
 * GPIO_18 y GPIO_19
 *
 */
void mostrarEnDisplay(uint32_t dato, uint8_t digitos_salida, gpioConf_t *vectorGpioConf, gpioConf_t *vectorPuertosLCD)
{
    uint8_t arregloDigitos[3];
    convertToBcdArray(dato, 3, arregloDigitos);
    for (int i = 0; i < digitos_salida; i++)
    {
        cambiarEstadosGPIO(arregloDigitos[i], vectorGpioConf);
        GPIOOn(vectorPuertosLCD[i].pin);
        GPIOOff(vectorPuertosLCD[i].pin);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    /*
     Creo el vector de estructuras del tipo gpioConf_t que contienen los puertos GPIO_20, GPIO_21, GPIO_22 
     y GPIO_23, con sus correspondientes direcciones(entrada/salida)
    */
    gpioConf_t arregloGPIO[4] = {
        {GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};

    /*
     Inicializo los puertos GPIO en el vector de 4 elementos
    */
    for (int i = 0; i < 4; i++)
    {
        GPIOInit(arregloGPIO[i].pin, arregloGPIO[i].dir);
    }
    /*
     Creo el vector de estructuras del tipo gpioConf_t que contienen los puertos GPIO_9, GPIO_18, GPIO_19 
     con sus correspondientes direcciones(entrada/salida)
    */
    gpioConf_t arregloGpioLCD[3] = {{GPIO_9, GPIO_OUTPUT}, {GPIO_18, GPIO_OUTPUT}, {GPIO_19, GPIO_OUTPUT}};

    /*
    Inicializo los puertos GPIO en el vector de 3 elementos
    */
    for (int j = 0; j < 3; j++)
    {
        GPIOInit(arregloGpioLCD[j].pin, arregloGpioLCD[j].dir);
    }

    mostrarEnDisplay(138, 3, arregloGPIO, arregloGpioLCD);
}
/*==================[end of file]============================================*/