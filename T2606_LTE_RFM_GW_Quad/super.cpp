#include "main.h"
#include "super.h"
#include "io.h"
#include "atask.h"
#include "hardware/watchdog.h"

typedef struct
{
    uint32_t value;
    uint32_t limit;
} super_wd_cntr;

typedef struct
{
    bool wd_enabled;
    super_wd_cntr cntr[SUPER_CNTR_NBR_OF];
    uint32_t timeout;
} super_st;


super_st super =
{
    .wd_enabled = false,
    .cntr = 
    {
      [SUPER_CNTR_LOOP]     = {.value = 0, .limit = 100},
      [SUPER_CNTR_R69]      = {.value = 0, .limit = 100},
      [SUPER_CNTR_IO]       = {.value = 0, .limit = 100},
      [SUPER_CNTR_HARAKIRI] = {.value = 0, .limit = SUPER_HARAKIRI_TIMEOUT_100MSEC},
    }
};





// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
atask_st super_th       = {"Super Task     ", 1000,    0,     0,  255,    0,   1, super_task};


void super_initialize(void)
{
    if(io_wd_is_enabled()){
        Serial.println("WD is enabled!!");
        super.wd_enabled = true;
        watchdog_enable(SUPER_WD_TIMEOUT, 1);
    }
    // atask_add_new(&super_th);  !! run from loop1
}

void super_clear_cntr(super_cntr_et cntr_indx)
{
    super.cntr[cntr_indx].value = 0;
}


void super_task(void)
{
    switch(super_th.state)
    {
        case 0:
            super_th.state = 10;
            break;
        case 10:
            if ( super.wd_enabled)  {
                watchdog_update();
                for (uint8_t i = 0; i < SUPER_CNTR_NBR_OF; i++) super.cntr[i].value = 0;
                super_th.state = 20;
            }
            break;
        case 20:
            watchdog_update();
            for (uint8_t i = 0; i < SUPER_CNTR_NBR_OF; i++)
            {
                if(super.cntr[i].value++ > super.cntr[i].limit){
                    Serial.printf("Starting WD reset for counter %d\n",i);
                    super_th.state = 100;
                    super.timeout = millis() + SUPER_WD_TIMEOUT + 1000;
                    io_led_flash(LED_RED, BLINK_SOS, 100);
                }
            }
            break;
        case 100:
            if(millis() > super.timeout ){
                Serial.println("Restarting/test");
                super_th.state = 10;
            }
            break;
    }
}
