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
    uint8_t key[] = RFM69_KEY;
    atask_initialize();

    rfm69_modem.initialize(MY_MODULE_TAG, MY_MODULE_ADDR, key);
    rfm69_modem.radiate(__APP__);
    atask_add_new(&modem_handle);
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

       if(rfm69_modem.msg_is_avail())
    {
        // rfm69_modem.get_msg(mbuff, BUFF_LEN, false);
        rfm69_modem.get_msg(mbuff, BUFF_LEN, false);    //
        Serial.println(mbuff);
        rfm69_modem.get_msg_decode(mbuff, BUFF_LEN, true);
        rssi = rfm69_modem.get_last_rssi();
        Serial.print(mbuff); Serial.print(" RSSI: "); Serial.println(rssi);
        //handler_parse_sensor_msg(mbuff, rssi, &node_data );
        //Serial.printf("%s - %s - %s  %d\n", node_data.zone, node_data.item, node_data.value, node_data.rssi);

        // if (handler_parse_msg(mbuff,rssi))
        // {
        //     // handler_process_event();
        // }
        //delay(3000);
        //rfm69_modem.radiate_node_json((char*) "<R1X1J1:Dock;T_bmp1;9.1;->");
        //rfm69_modem.radiate("OK");
    }
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




