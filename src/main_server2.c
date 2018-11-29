/*******************************************************************************
   * Copyright (c) 2014 IBM Corp.
   *
   * All rights reserved. This program and the accompanying materials
   * are made available under the terms of the Eclipse Public License v1.0
   * and Eclipse Distribution License v1.0 which accompany this distribution.
   *
   * The Eclipse Public License is available at
   *    http://www.eclipse.org/legal/epl-v10.html
   * and the Eclipse Distribution License is available at
   *   http://www.eclipse.org/org/documents/edl-v10.php.
   *
   * Contributors:
   *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

/*
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "freertos/semphr.h"

#include "driver/gpio.h"

#include <esp_http_server.h>



//#include "MQTTClient.h"

/ *
   The examples use simple WiFi configuration that you can set via
   'make menuconfig'.
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
* /


//#define EXAMPLE_WIFI_SSID  "INFINITUMyhh6"
//#define EXAMPLE_WIFI_PASS  "b1052d4e8b"


#define EXAMPLE_WIFI_SSID "oso"

#define EXAMPLE_WIFI_PASS ""

/ * FreeRTOS event group to signal when we are connected &
  ready to make a request
* /
static EventGroupHandle_t wifi_event_group;

/ * The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP?
* /



const int CONNECTED_BIT = BIT0;

//#define MQTT_BROKER  "192.168.2.1"//"iot.eclipse.org"
/ * MQTT Broker Address * /
//#define MQTT_PORT    1883               / * MQTT Port * /

//#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
//#define MQTT_CLIENT_THREAD_STACK_WORDS  8192
//#define MQTT_CLIENT_THREAD_PRIO         8

static const char *TAG = "example";




/ *
static void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", (char*)data->message->payload);
}
* /

//MQTTClient client;

/ *
static void taskPub(void* pvParameters)
{
  //Network network;
  //unsigned char sendbuf[80], readbuf[80] = {0};
  int rc = 0, count = 0;
  //MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

  while(1){

      MQTTMessage message;
      char payload[30];

      message.qos = QOS1;

      message.retained = 0;
      message.payload = payload;
      sprintf(payload, "message number %d", count);
      message.payloadlen = strlen(payload);

      printf("MQTT publish --> \n");

      if ((rc = MQTTPublish(&client, "ESP8266-pub", &message)) != 0)
      {
          printf("Return code from MQTT publish is %d\n", rc);
      }
      else
      {
          printf("MQTT publish topic \"ESP8266-pub\", message number is %d\n", count);
      }

      vTaskDelay(5000 / portTICK_RATE_MS);

  }
}
* /

/ *
static void mqtt_client_thread(void* pvParameters)
{

    Network network;
    unsigned char sendbuf[80], readbuf[80] = {0};
    int rc = 0, count = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    connectData.username.cstring = "giat";
    connectData.password.cstring = "1234";

    printf("mqtt client thread starts\n");

    / * Wait for the callback to set the CONNECTED_BIT in the
       event group.
    * /
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);


    printf("portMAX_DELAY %d\n", portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");

    NetworkInit(&network);

    MQTTClientInit(&client, &network, 30000, sendbuf,
      sizeof(sendbuf), readbuf, sizeof(readbuf));

    char* address = MQTT_BROKER;

    if ((rc = NetworkConnect(&network, address, MQTT_PORT)) != 0) {
        printf("Return code from network connect is %d\n", rc);
    }

    #if defined(MQTT_TASK)

        if ((rc = MQTTStartTask(&client)) != pdPASS) {
            printf("Return code from start tasks is %d\n", rc);
        } else {
            printf("Use MQTTStartTask\n");
        }
    #endif

    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = "ESP8266_sample";

    if ((rc = MQTTConnect(&client, &connectData)) != 0)
    {
        printf("Return code from MQTT connect is %d\n", rc);
    }
    else
    {
        printf("MQTT Connected\n");
    }

    printf("3 seg \n");
    vTaskDelay(3000 / portTICK_RATE_MS);

    if ((rc = MQTTSubscribe(&client, "ESP8266-sub", 2, messageArrived)) != 0) {
        printf("Return code from MQTT subscribe is %d\n", rc);
    } else {
        printf("MQTT subscribe to topic \"ESP8266-sub\"\n");
    }



    xTaskCreate(&taskPub,
                MQTT_CLIENT_THREAD_NAME,
                MQTT_CLIENT_THREAD_STACK_WORDS,
                NULL,
                MQTT_CLIENT_THREAD_PRIO,
                NULL);

    while (++count)
    {
        MQTTMessage message;
        char payload[30];

        message.qos = QOS1;

        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", count);
        message.payloadlen = strlen(payload);

        //printf("MQTT publish --> \n");

        / *
        if ((rc = MQTTPublish(&client, "ESP8266-pub", &message)) != 0)
        {
            printf("Return code from MQTT publish is %d\n", rc);
        }
        else
        {
            printf("MQTT publish topic \"ESP8266-pub\", message number is %d\n", count);
        }
        * /

        vTaskDelay(1000 / portTICK_RATE_MS);  //send every 1 seconds
    }

    printf("mqtt_client_thread going to be deleted\n");
    vTaskDelete(NULL);
    return;
}
* /

// --------- GPIO -----------------------
/ *
#define ESP_INTR_FLAG_DEFAULT 0
#define PIN_PULSADOR 13

//creo el manejador para el semáforo como variable global
SemaphoreHandle_t xSemaphore = NULL;

// Rutina de interrupción, llamada cuando se presiona el pulsador
void IRAM_ATTR pulsador_isr_handler(void* arg) {

    // da el semáforo para que quede libre para la tarea pulsador
   xSemaphoreGiveFromISR(xSemaphore, NULL);
}

void init_GPIO()
{
   //configuro el PIN_PULSADOR como un pin GPIO
   gpio_pad_select_gpio(PIN_PULSADOR);
   // seleciono el PIN_PULSADOR como pin de entrada
   gpio_set_direction(PIN_PULSADOR, GPIO_MODE_INPUT);
   // instala el servicio ISR con la configuración por defecto.
   gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
   // añado el manejador para el servicio ISR
   gpio_isr_handler_add(PIN_PULSADOR, pulsador_isr_handler, NULL);
   // habilito interrupción por flanco descendente (1->0)
   gpio_set_intr_type(PIN_PULSADOR, GPIO_INTR_NEGEDGE);

}

void task_pulsador(void* arg) {

   while(1) {

      // Espero por la notificación de la ISR
      if(xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
         printf("Pulsador presionado!\n");

      }
   }
}


* /
// --------------------------------------

// ----------- Server http ---------------------------
/ * An HTTP GET handler * /
esp_err_t hello_get_handlerServer(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;


    / * Get header value string length and allocate memory for length + 1,
     * extra byte for null termination * /
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        / * Copy null terminated value string into buffer * /
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    / * Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination * /
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            / * Get value of expected key from query string * /
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    / * Set some custom headers * /
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    / * Send response with custom headers and body set as the
     * string passed in user context* /
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    / * After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. * /
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

httpd_uri_t helloConf = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handlerServer,
    / * Let's pass response string in user
     * context to demonstrate it's usage * /
    .user_ctx  = "Hello World!"
};


httpd_handle_t start_webSrv(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &helloConf);
        //httpd_register_uri_handler(server, &echo);
        //httpd_register_uri_handler(server, &ctrl);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webSrv(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    ESP_LOGI(TAG, "Event Handler");
    httpd_handle_t *server = (httpd_handle_t *) ctx;

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM EVENT STA GOT IP");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        / * Start the web server * /
        if (*server == NULL) {
            *server = start_webSrv();
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "SYSTEM EVENT STA DISCONNECTED");
        / * Stop the web server * /
        if (*server) {
            stop_webSrv(*server);
            *server = NULL;
        }

        / * This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. * /
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);

        break;
    default:
        break;
    }
    return ESP_OK;
}

/ *
static esp_err_t event_handlerSrv(void *ctx, system_event_t *event)
{
    httpd_handle_t *server = (httpd_handle_t *) ctx;
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        ESP_LOGI(TAG, "Got IP: '%s'",
                ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

        / * Start the web server * /
        if (*server == NULL) {
            *server = start_webSrv();
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
        ESP_ERROR_CHECK(esp_wifi_connect());

        / * Stop the web server * /
        if (*server) {
            stop_webSrv(*server);
            *server = NULL;
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

* /


static void initialise_wifi(void *arg)
{
    printf("Initialise Wifi\n");
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, arg) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void aain(void)
{

    printf("Iniciando . . . \n");
    static httpd_handle_t server = NULL;
    printf("static httpd_handle_t server = NULL; . . . \n");
    ESP_ERROR_CHECK( nvs_flash_init() );
    printf("ESP_ERROR_CHECK( nvs_flash_init() ); . . . \n");
    initialise_wifi(&server);
    printf("initialise_wifi(&server); . . . \n");



    / *
    xTaskCreate(&mqtt_client_thread,
                MQTT_CLIENT_THREAD_NAME,
                MQTT_CLIENT_THREAD_STACK_WORDS,
                NULL,
                MQTT_CLIENT_THREAD_PRIO,
                NULL);
                * /
}


*/
