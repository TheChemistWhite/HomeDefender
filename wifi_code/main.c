#include <stdio.h>
#include "led.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "thread.h"
#include "paho_mqtt.h"
#include "MQTTClient.h"

// MQTT client settings
#define BUF_SIZE 1024

#define MQTT_BROKER_ADDR "192.168.1.7" // After connecting to the hotspot with the pc, insert here the ifconfig wlo1
#define MQTT_TOPIC "sensor/pir"
#define MQTT_VERSION_v311 4 /* MQTT v3.1.1 version is 4 */
#define COMMAND_TIMEOUT_MS 4000

#define DEFAULT_MQTT_CLIENT_ID ""

#define DEFAULT_MQTT_USER ""

#define DEFAULT_MQTT_PWD ""

#define DEFAULT_MQTT_PORT 1883 // Default MQTT port

#define DEFAULT_KEEPALIVE_SEC 10 // Keepalive timeout in seconds

#define IS_RETAINED_MSG 0

#define PRINT_KEY_LINE_LENGTH 5

static MQTTClient client;
static Network network;

#define SENSOR_PIN      13 
#define BUZZER_PIN      23

char stack1[THREAD_STACKSIZE_MAIN];
kernel_pid_t thread_pid_buzzer;

bool sound = false;

static int mqtt_disconnect(void){

    int res = MQTTDisconnect(&client);

    if (res < 0){
        printf("mqtt_example: Unable to disconnect\n");
    }

    else{
        printf("mqtt_example: Disconnect successful\n");
    }

    NetworkDisconnect(&network);
    return res;
}

static int mqtt_connect(void){
    int ret = 0;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = MQTT_VERSION_v311;
    data.clientID.cstring = DEFAULT_MQTT_CLIENT_ID;
    data.username.cstring = DEFAULT_MQTT_USER;
    data.password.cstring = DEFAULT_MQTT_PWD;
    data.keepAliveInterval = 60;
    data.cleansession = 1;

    printf("MQTT: Connecting to MQTT Broker from %s %d\n",
           MQTT_BROKER_ADDR, DEFAULT_MQTT_PORT);
    printf("MQTT: Trying to connect to %s, port: %d\n",
           MQTT_BROKER_ADDR, DEFAULT_MQTT_PORT);
    ret = NetworkConnect(&network, MQTT_BROKER_ADDR, DEFAULT_MQTT_PORT);

    if (ret < 0){
        printf("MQTT: Unable to connect\n");
        return ret;
    }

    printf("user:%s clientId:%s password:%s\n", data.username.cstring,
           data.clientID.cstring, data.password.cstring);
    ret = MQTTConnect(&client, &data);

    if (ret < 0){
        printf("MQTT: Unable to connect client %d\n", ret);
        int res = MQTTDisconnect(&client);

        if (res < 0){
            printf("MQTT: Unable to disconnect\n");
        }

        else{
            printf("MQTT: Disconnect successful\n");
        }

        NetworkDisconnect(&network);
        return res;
    }

    else{
        printf("MQTT: Connection successfully\n");
    }

    printf("MQTT client connected to broker\n");
    return 0;
}

int main(void) {
    
    gpio_init(SENSOR_PIN, GPIO_IN);
    gpio_init(BUZZER_PIN, GPIO_OUT);
    xtimer_ticks32_t last_motion_time = {0};
    char json[200];

    MQTTClientInit(&client, &network, COMMAND_TIMEOUT_MS, buf, BUF_SIZE,
                   readbuf,
                   BUF_SIZE);

    MQTTStartTask(&client);

    // Connect to MQTT broker
    mqtt_connect();

    printf("PIR Sensor Test\n");

    thread_pid_buzzer = thread_create(stack1, sizeof(stack1),
                                      THREAD_PRIORITY_MAIN + 1,
                                      THREAD_CREATE_SLEEPING,
                                      buzzer_thread,
                                      NULL, "buzzer_thread");

    
   
    while (1) {
        int val = gpio_read(SENSOR_PIN);
        
        if (val == 1) { 
            LED_ON(0); 
            xtimer_usleep(100000); 

            if (xtimer_usec_from_ticks(xtimer_now()) - xtimer_usec_from_ticks(last_motion_time) > 1000000) {
                printf("Motion detected!\n");
                sound = true;
                thread_wakeup(thread_pid_buzzer);

                sprintf(json, "{\"id\": \"%d\", \"pir\": \"%d\", \"msg\": \"MOTION DETECTED\"}",
                        1, val;

                 char* message_motion = json;

                MQTTMessage message;
                message.qos = QOS2;
                message.retained = IS_RETAINED_MSG;
                message.payload = message_motion;
                message.payloadlen = strlen(message.payload);
        
                char* topic = "homeDefender";
        
                int rc;
                if ((rc = MQTTPublish(&client, topic, &message)) < 0) {
                    printf("mqtt_example: Unable to publish (%d)\n", rc);
                }
                else {
                    printf("mqtt_example: Message (%s) has been published to topic %s "
                        "with QOS %d\n",
                        (char *)message.payload, topic, (int)message.qos);
                }
                
                last_motion_time = xtimer_now();
            }
        } else {
            LED_OFF(0); 
            sound = false;
            xtimer_usleep(200000); 

            if (xtimer_usec_from_ticks(xtimer_now()) - xtimer_usec_from_ticks(last_motion_time) > 1000000) {
                printf("Motion stopped!\n");

                sprintf(json, "{\"id\": \"%d\", \"pir\": \"%d\", \"msg\": \"Motion Stopped\"}",
                        1, val;

                MQTTMessage message;
                message.qos = QOS2;
                message.retained = IS_RETAINED_MSG;
                message.payload = message_motion;
                message.payloadlen = strlen(message.payload);
        
                char* topic = "homeDefender";
        
                int rc;
                if ((rc = MQTTPublish(&client, topic, &message)) < 0) {
                    printf("mqtt_example: Unable to publish (%d)\n", rc);
                }
                else {
                    printf("mqtt_example: Message (%s) has been published to topic %s "
                        "with QOS %d\n",
                        (char *)message.payload, topic, (int)message.qos);
                }

                last_motion_time = xtimer_now();
            }
        }

    }

    return 0;
}
