/* Mirf Example

	 This example code is in the Public Domain (or CC0 licensed, at your option.)

	 Unless required by applicable law or agreed to in writing, this
	 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	 CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "mirf.h"
#include <stdbool.h>
#include <math.h>

bool send_signal = false;

typedef union {
	uint8_t value[4];
	unsigned long now_time;
} MYDATA_t;

MYDATA_t mydata;

typedef struct
{
    // Private attributes
    char A;	
    char B;	
    char C;
    char D;
    char E; // Light Control

} BinaryDataClass;


#if CONFIG_ADVANCED
void AdvancedSettings(NRF24_t * dev)
{
#if CONFIG_RF_RATIO_2M
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 2MBps");
	Nrf24_SetSpeedDataRates(dev, 1);
#endif // CONFIG_RF_RATIO_2M

#if CONFIG_RF_RATIO_1M
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 1MBps");
	Nrf24_SetSpeedDataRates(dev, 0);
#endif // CONFIG_RF_RATIO_2M

#if CONFIG_RF_RATIO_250K
	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 250KBps");
	Nrf24_SetSpeedDataRates(dev, 2);
#endif // CONFIG_RF_RATIO_2M

	ESP_LOGW(pcTaskGetName(0), "CONFIG_RETRANSMIT_DELAY=%d", CONFIG_RETRANSMIT_DELAY);
	Nrf24_setRetransmitDelay(dev, CONFIG_RETRANSMIT_DELAY);
}
#endif // CONFIG_ADVANCED

#if CONFIG_RECEIVER

BinaryDataClass receivedData;
#define LIGHT_PIN 48

void light(void *pvParameter)
{
    while (1)
    {
        if (receivedData.E == '1')
		{
			gpio_set_level(LIGHT_PIN, 1);
		}
		else
		{
			gpio_set_level(LIGHT_PIN, 0);
		}
        vTaskDelay(pdMS_TO_TICKS(10)); // Check the update every 10ms
    }
}

void decoder_uint2bin(uint8_t num, BinaryDataClass *result)
{
    if(num >= pow(2, 4))
	{
		result->A = '1';
		num -= pow(2, 4);
	}
	else
	{
		result->A = '0';
	}
	if(num >= pow(2, 3))
	{
		result->B = '1';
		num -= pow(2, 3);
	}
	else
	{
		result->B = '0';
	}
	if(num >= pow(2, 2))
	{
		result->C = '1';
		num -= pow(2, 2);
	}
	else
	{
		result->C = '0';
	}
	if(num >= pow(2, 1))
	{
		result->D = '1';
		num -= pow(2, 1);
	}
	else
	{
		result->D = '0';
	}
	if(num >= pow(2, 0))
	{
		result->E = '1';
		num -= pow(2, 0);
	}
	else
	{
		result->E = '0';
	}
}

void receiver(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	NRF24_t dev;
	Nrf24_init(&dev);
	uint8_t payload = sizeof(mydata.value);
	uint8_t channel = 90;
	Nrf24_config(&dev, channel, payload);

	//Set own address using 5 characters
	esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"FGHIJ");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}

#if CONFIG_ADVANCED
	AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

	//Print settings
	Nrf24_printDetails(&dev);
	ESP_LOGI(pcTaskGetName(0), "Listening...");
	while(1)
	{
		//When the program is received, the received data is output from the serial port
		if (Nrf24_dataReady(&dev)) {
			Nrf24_getData(&dev, mydata.value);
			ESP_LOGI(pcTaskGetName(0), "Got data at time:%lu", mydata.now_time);
			// for(int i = 0; i < 6; i++)
			// {
			// 	printf("The uint8_t value is: %u\n", mydata.value[i]);
			// }
			decoder_uint2bin(mydata.value[3], &receivedData);
		}
		vTaskDelay(1);
	}
}
#endif // CONFIG_RECEIVER


#if CONFIG_TRANSMITTER

BinaryDataClass controlData;

#define TEST_PIN 13
#define LIGHT_CONTROL_PIN 14

void check_if_transmmit(void *pvParameter)
{
    while (1)
    {
        if (gpio_get_level(TEST_PIN) == 1)
        {
            // ESP_LOGI(pcTaskGetName(0), "Hello\n");
            // vTaskDelay(pdMS_TO_TICKS(500)); // Add a delay to prevent continuous printing
			send_signal = true;
        }
		else
		{
			send_signal = false;
		}
        vTaskDelay(pdMS_TO_TICKS(10)); // Check the input pin every 10ms
    }
}

void light_control(void *pvParameter)
{
    while (1)
    {
        if (gpio_get_level(LIGHT_CONTROL_PIN) == 1)
        {
			if (controlData.E == '0')
			{
				printf("Now it is HIGH!\n");
			}
            controlData.E = '1';
        }
		else
		{
			if (controlData.E == '1')
			{
				printf("Now it is LOW!\n");
			}
			controlData.E = '0';
		}
        vTaskDelay(pdMS_TO_TICKS(10)); // Check the input pin every 10ms
    }
}

uint8_t encoder_bin2uint(const BinaryDataClass *bc)
{
    uint8_t result = 0;
	if(controlData.E == '1')
	{
		result = result + pow(2, 0);
	}
	if(controlData.D == '1')
	{
		result = result + pow(2, 1);
	}
	if(controlData.C == '1')
	{
		result = result + pow(2, 2);
	}
	if(controlData.B == '1')
	{
		result = result + pow(2, 3);
	}
	if(controlData.A == '1')
	{
		result = result + pow(2, 4);
	}

    return result;
}

void transmitter(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	NRF24_t dev;
	Nrf24_init(&dev);
	uint8_t payload = sizeof(mydata.value);
	uint8_t channel = 90;
	Nrf24_config(&dev, channel, payload);

	//Set the receiver address using 5 characters
	esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}

#if CONFIG_ADVANCED
	AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

	//Print settings
	Nrf24_printDetails(&dev);

	while(1) {
		if(send_signal == true)
		{
			mydata.now_time = xTaskGetTickCount();
			mydata.value[3] = encoder_bin2uint(&controlData);
			// printf("Control is: %u \n", mydata.value[3]);
			Nrf24_send(&dev, mydata.value);
			// vTaskDelay(1);
			ESP_LOGI(pcTaskGetName(0), "Wait for sending.....");
			if (Nrf24_isSend(&dev, 1000)) {
				ESP_LOGI(pcTaskGetName(0),"Send success at time:%lu", mydata.now_time);
			} else {
				ESP_LOGW(pcTaskGetName(0),"Send fail:");
			}
			vTaskDelay(1000/portTICK_PERIOD_MS);
		}
		else
		{
			vTaskDelay(10/portTICK_PERIOD_MS);
		}
			
	}
}
#endif // CONFIG_TRANSMITTER

void app_main(void)
{

#if CONFIG_RECEIVER
	gpio_config_t light_pin_config = {
        .pin_bit_mask = 1ULL << LIGHT_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&light_pin_config);
	xTaskCreate(light, "Light Control Task", 2048, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(receiver, "RECEIVER", 1024*3, NULL, 2, NULL);
#endif

#if CONFIG_TRANSMITTER
	controlData.A = '0';
	controlData.B = '0';
	controlData.C = '0';
	controlData.D = '0';
	controlData.E = '0';

	gpio_config_t input_pin_config = {
        .pin_bit_mask = 1ULL << TEST_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&input_pin_config);

	xTaskCreate(check_if_transmmit, "Check Input Pin Task", 2048, NULL, tskIDLE_PRIORITY, NULL);

	gpio_config_t light_control_pin_config = {
        .pin_bit_mask = 1ULL << LIGHT_CONTROL_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&light_control_pin_config);

	xTaskCreate(light_control, "Light Control Task", 2048, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(transmitter, "TRANSMITTER", 1024*3, NULL, 2, NULL);
#endif

}