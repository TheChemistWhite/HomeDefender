#include <stdio.h>
#include "led.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "thread.h"
#include "net/loramac.h"
#include "semtech_loramac.h"

#define PM_LOCK_LEVEL       (1)

#define MAX_JOIN_RETRIES 3

extern semtech_loramac_t loramac;
#if !IS_USED(MODULE_PERIPH_RTC)
static ztimer_t timer;
#endif

#ifdef USE_OTAA
static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];
#endif

#define SENSOR_PIN      13 
#define BUZZER_PIN      23

char stack1[THREAD_STACKSIZE_MAIN];
kernel_pid_t thread_pid_buzzer;

bool sound = false;

bool joinLoRaNetwork(void) {

    int joinRetries = 0;

    while (joinRetries < MAX_JOIN_RETRIES) {

        printf("Starting join procedure (attempt %d)\n", joinRetries + 1);
        if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) == SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
            printf("Join succeeded\n");
            return true; 
        } else {
            printf("Join procedure failed\n");
            joinRetries++;
        }
    }

    printf("Exceeded maximum join retries\n");
    return false; 
}


int main(void) {

    
    printf("HOMEDEFENDER: a LoRaWAN low-power application\n");

    #if IS_USED(MODULE_PM_LAYERED)
        for (unsigned i = 1; i < PM_NUM_MODES - 1; ++i) {
            pm_unblock(i);
        }
    #endif

    #ifdef USE_OTAA /* OTAA activation mode */
        
        fmt_hex_bytes(deveui, CONFIG_LORAMAC_DEV_EUI_DEFAULT);
        fmt_hex_bytes(appeui, CONFIG_LORAMAC_APP_EUI_DEFAULT);
        fmt_hex_bytes(appkey, CONFIG_LORAMAC_APP_KEY_DEFAULT);
        semtech_loramac_set_deveui(&loramac, deveui);
        semtech_loramac_set_appeui(&loramac, appeui);
        semtech_loramac_set_appkey(&loramac, appkey);

        
        semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

        
        if (!semtech_loramac_is_mac_joined(&loramac)) {
            
            if (!joinLoRaNetwork()) {
                printf("Failed to join the network\n");
                return 1;
            }

    #ifdef MODULE_PERIPH_EEPROM
            
            semtech_loramac_save_config(&loramac);
    #endif
        }
    #endif
    
    gpio_init(SENSOR_PIN, GPIO_IN);
    gpio_init(BUZZER_PIN, GPIO_OUT);
    xtimer_ticks32_t last_motion_time = {0};
    char json[200];

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
                msg_to_be_sent = (uint8_t *)json;

                printf("Sending: %s\n", msg_to_be_sent);
                uint8_t ret = semtech_loramac_send(&loramac, (uint8_t*)msg_to_be_sent, strlen(json));
                if (ret != SEMTECH_LORAMAC_TX_DONE){
                    printf("Cannot send message '%s', ret code: %d\n", msg_to_be_sent, ret);
                    free(msg_to_be_sent);
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
                
                msg_to_be_sent = (uint8_t *)json;

                printf("Sending: %s\n", msg_to_be_sent);
                uint8_t ret = semtech_loramac_send(&loramac, (uint8_t*)msg_to_be_sent, strlen(json));
                if (ret != SEMTECH_LORAMAC_TX_DONE){
                    printf("Cannot send message '%s', ret code: %d\n", msg_to_be_sent, ret);
                    free(msg_to_be_sent);
                }

                last_motion_time = xtimer_now();
            }
        }

        
    }

    return 0;
}
