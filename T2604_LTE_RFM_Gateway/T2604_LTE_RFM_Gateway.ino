/*************************************************************************

**************************************************************************

https://github.com/infrapale/T2604_LTE_RFM_Gateway.git
*************************************************************************/

#include    "main.h"
#include    "secrets.h"
//#include    <RH_RF69.h>
#include    "Rfm69Modem.h"
#include    "atask.h"
#include    "io.h"
#include    "lte.h"
#include    "msg.h"


#define IO_TICK_INTERVAL    (100)

#define ENCRYPTKEY    modem69_KEY   // defined in secret.h
// RH_RF69         rf69(PIN_RFM_CS, PIN_RFM_IRQ);
// Rfm69Modem      modem69_modem(&rf69,  PIN_RFM_RESET, -1 );
modem_data_st   modem_data = {MY_MODULE_TAG, MY_MODULE_ADDR};

main_ctrl_st ctrl = {0};

void print_debug_task(void);
atask_st debug_th       =     {"Debug Task     ", 2000,    0,     0,  255,    0,  1,  print_debug_task };

#define BUFF_LEN   80
char mbuff[BUFF_LEN];
int16_t rssi;
//node_data_st node_data;

extern lte_msg_st lte_msg;

void print_debug_task(void)
{
  atask_print_status(true);
}

void setup() {
    Serial1.setTX(PIN_TX0);   
    Serial1.setRX(PIN_RX0);
    Serial.begin(115200);
 
    SPI.setSCK( PIN_RFM_SCK );   
    SPI.setTX( PIN_RFM_MOSI ); 
    SPI.setRX( PIN_RFM_MISO );  
    SPI.begin();


    delay(1500);
    //Serial1.begin(115200);
    atask_initialize();
    msg_initialize();
    while(true) delay(100);    

    modem69_initialize();
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
    if (!lte_msg.available){
        uint16_t len = lte_read_line(lte_msg.message, MSG_LEN, 1000);
        if (len > 0){
            Serial.printf("Message len: %d:<%s>\n", len, lte_msg.message);
        }
    }

 }

void loop1()
{
    if(millis() > ctrl.next_io_tick){
        ctrl.next_io_tick = millis() + IO_TICK_INTERVAL;
        io_task();
    }
}



