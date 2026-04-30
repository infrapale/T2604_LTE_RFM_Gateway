/*************************************************************************

**************************************************************************

https://github.com/infrapale/T2604_LTE_RFM_Gateway.git
*************************************************************************/

#include    "main.h"
#include    "secrets.h"
#include    <RH_RF69.h>
#include    "Rfm69Modem.h"
#include    "atask.h"
#include    "io.h"
#include    "lte.h"


#define IO_TICK_INTERVAL    (100)

#define ENCRYPTKEY    RFM69_KEY   // defined in secret.h
RH_RF69         rf69(PIN_RFM_CS, PIN_RFM_IRQ);
Rfm69Modem      rfm69_modem(&rf69,  PIN_RFM_RESET, -1 );
modem_data_st   modem_data = {MY_MODULE_TAG, MY_MODULE_ADDR};

main_ctrl_st ctrl = {0};

void print_debug_task(void);
void modem_task(void);
atask_st modem_handle   =     {"Radio Modem    ", 100,0, 0, 255, 0, 1, modem_task};
atask_st debug_th       =     {"Debug Task     ", 2000,    0,     0,  255,    0,  1,  print_debug_task };



void print_debug_task(void)
{
  atask_print_status(true);
}

void setup() {
    Serial1.setTX(PIN_TX0);   
    Serial1.setRX(PIN_RX0);
    Serial.begin(115200);

    delay(1500);
    Serial1.begin(115200);
    uint8_t key[] = RFM69_KEY;
    atask_initialize();

    // rfm69_modem.initialize(MY_MODULE_TAG, MY_MODULE_ADDR, key);
    // rfm69_modem.radiate(__APP__);
    // atask_add_new(&modem_handle);
    atask_add_new(&debug_th);
    lte_initialize();
}


void setup1(){
    io_initialize();
    ctrl.next_io_tick = millis() + IO_TICK_INTERVAL;
}

void loop() 
{
    atask_run();
}

void loop1()
{
    if(millis() > ctrl.next_io_tick){
        ctrl.next_io_tick = millis() + IO_TICK_INTERVAL;
        io_task();
    }
}

void modem_task(void)
{
    rfm69_modem.modem_task();
}




