#include "pico/sleep.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/rtc.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"
#include "ds3232rtc.hpp"

extern uint8_t buf[];
extern char *week[];

uint8_t sleep_period_mins = 1;


/*
//Function to read time from rtc and print
void ds3231_readtime(void){
    char buff[128];
    struct ts t;
    DS3231_get(&t);
    snprintf(buff, 128, "%d.%02d.%02d %02d:%02d:%02d", t.year,
            t.mon, t.mday, t.hour, t.min, t.sec);

    printf("Time from lib:");
    printf(buff);
}
*/

void setAlarm(int sleep_mins){
    unsigned char wakeup_min;

    //Enable power to RTC
    //May need this with multiple devices on I2C Bus
    //gpio_put(15,true);

    struct ts t;
    DS3231_init(DS3231_CONTROL_INTCN);

    //Read time from rtc
    DS3231_get(&t);
    // calculate the minute when the next alarm will be triggered
    wakeup_min = (t.min / sleep_mins + 1) * sleep_mins;
    if (wakeup_min > 59) {
        wakeup_min -= 60;
    }

     uint8_t flags[4] = { 0, 1, 1, 1 };

    // set Alarm2. only the minute is set since we ignore the hour and day component
    DS3231_set_a2(wakeup_min, 0, 0, flags);

    // activate Alarm2 and enable Battery Backed SQW
    DS3231_set_creg(DS3231_CONTROL_INTCN | DS3231_CONTROL_A2IE | DS3231_CONTROL_BBSQW);
    sleep_ms(100);

    //Enable power to RTC
    //May need this with multiple devices on I2C Bus
    //gpio_put(15,false);
}

void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig){

    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_LSB);

    //reset procs back to default
    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //reset clocks
    clocks_init();
    stdio_init_all();

    return;
}

int main() {
    const uint LED_PIN = 25;
    int i;

    stdio_init_all();
    printf("Startup...\n");
    uart_default_tx_wait_blocking();

    /*
    Configure Pin 15 to supply power to
    DS3231 during I2C comms
    May need this with multiple devices on I2C Bus
    Possibly resolved by using 4.7k as internal Pico pullups
    may not be strong enough
    */
    //gpio_init(15);
    //gpio_set_dir(15, GPIO_OUT);

    i2c_inst_t *i2c   = i2c0;
    static const uint8_t DEFAULT_SDA_PIN        = 4;
    static const uint8_t DEFAULT_SCL_PIN        = 5;
    int8_t sda        = DEFAULT_SDA_PIN;
    int8_t scl        = DEFAULT_SCL_PIN;
    i2c_init(i2c, 400000);

    printf("I2C init\n");
    uart_default_tx_wait_blocking();

    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(scl);

    //save values for later
    uint scb_orig = scb_hw->scr;
    uint clock0_orig = clocks_hw->sleep_en0;
    uint clock1_orig = clocks_hw->sleep_en1;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Start the RTC
    rtc_init();

    printf("RTC started\n");
    uart_default_tx_wait_blocking();
    while (true) {
        //gpio_put(LED_PIN, 1);

        sleep_run_from_xosc();
        DS3231_clear_a2f();

        printf("Setting alarm\n");
        //sleep for 1 mins
        setAlarm(sleep_period_mins);
        printf("Alarm set\n");

        //Block until print message is sent
        uart_default_tx_wait_blocking();

        //Required to ensure SQW is held up
        //as we have removed the resistor pack
        gpio_pull_up(10);
        sleep_goto_dormant_until_pin(10, true, false);

        DS3231_clear_a2f();
        recover_from_sleep(scb_orig, clock0_orig, clock1_orig);

        printf("Recovered\n");
        uart_default_tx_wait_blocking();

        for(i=0; i< 20; i++){
            gpio_put(LED_PIN, 1);
            sleep_ms(250);
            gpio_put(LED_PIN, 0);
            sleep_ms(250);
        }
        //Do some work before sleeping again
        sleep_ms(10000);
    }
}
