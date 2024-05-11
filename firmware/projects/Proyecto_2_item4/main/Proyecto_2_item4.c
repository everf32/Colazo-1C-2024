/*! @mainpage Medidor de distancia por ultrasonido
 *
 * \section genDesc General Description
 *
 * En este proyecto se trabaja sobre la conversion analogica-digital
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/04/2024 | Inicio del proyecto		                     |
 * | 15/04/2024 | Documentacion del proyecto                     |
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
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define TIEMPO_CONVERSION_AD 2000
#define TIEMPO_CONVERSION_DA 4000
/**
 * @def BUFFER_SIZE 
 * @brief Tamaño del vector que contiene los datos de un ECG
*/
#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/
/** 
 * @def ConversorAD_task_handle 
 * @brief elemento utilizado para manejar las tareas con interrupciones
*/
TaskHandle_t ConversorAD_task_handle = NULL;

/** 
 * @def ConversorDA_task_handle 
 * @brief elemento utilizado para manejar las tareas con interrupciones
*/
TaskHandle_t ConversorDA_task_handle = NULL;
/** 
 * @def ValorAnalogico 
 * @brief variable de tipo entero sin signo que almacenará el valor analógico leído
*/
uint16_t valorAnalogico = 0;
/** 
 * @def ecg 
 * @brief vector que contiene todos los datos necesario para levantar una señal de ECG
*/
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[internal functions declaration]=========================*/
/** 
 * @fn escribirValorEnPC() 
 * @brief Permite la salida por monitor serie de los valores analógicos leídos
*/
void escribirValorEnPc()
{
   // UartSendString(UART_PC, "valor ");
    UartSendString(UART_PC, (char *)UartItoa(valorAnalogico, 10));
    UartSendString(UART_PC, "\r");
}

/** 
 * @brief Tarea involucrada en la conversion AD
*/
void AD_conversor_task()
{

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &valorAnalogico);
        escribirValorEnPc();
    }
}

/** 
 * @brief Tarea involucrada en la conversion DA 
*/
void DA_conversor_task()
{
    uint8_t i = 0;
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // enviar dato
        AnalogOutputWrite(ecg[i]);
        i++;
        if (i == BUFFER_SIZE-1)
        {
            i = 0;
        }
    }
}
/** 
 * @fn FuncTimerConversionDA()
 * @brief Involucrada en enviar una notificación para poder continuar la tarea de conversion DA
*/
void FuncTimerConversionDA()
{
    vTaskNotifyGiveFromISR(ConversorDA_task_handle, pdFALSE);
}

/** 
 * @fn FuncTimerConversionAD()
 * @brief Involucrada en enviar una notificación para poder continuar la tarea de conversion AD
*/
void FuncTimerConversionAD()
{
    vTaskNotifyGiveFromISR(ConversorAD_task_handle, pdFALSE);
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
    UartInit(&ControlarUart);
    /* Inicialización de timer de conversor AD */
    timer_config_t timer_conversionDA = {
        .timer = TIMER_A,
        .period = TIEMPO_CONVERSION_DA,
        .func_p = FuncTimerConversionDA,
        .param_p = NULL};
    TimerInit(&timer_conversionDA);

    timer_config_t timer_conversionAD = {
        .timer = TIMER_B,
        .period = TIEMPO_CONVERSION_AD,
        .func_p = FuncTimerConversionAD,
        .param_p = NULL};
    TimerInit(&timer_conversionAD);

    analog_input_config_t Analog_input = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&Analog_input);
    AnalogOutputInit();

    /*creacion de tareas*/
    xTaskCreate(&DA_conversor_task, "conversor DA", 512, NULL, 5, &ConversorDA_task_handle);
    xTaskCreate(&AD_conversor_task, "conversor AD", 4096, NULL, 5, &ConversorAD_task_handle);

    /*Inicio del conteo de timers*/
    TimerStart(timer_conversionAD.timer);
    TimerStart(timer_conversionDA.timer);
}
