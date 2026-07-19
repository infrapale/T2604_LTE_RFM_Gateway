
#include    "main.h"
#include    "secrets.h"
#include    <RH_RF69.h>
#include    "modem69.h"
#include    "atask.h"
#include    "msg.h"
#include    "io.h"
#include    "r69.h"
#include    "sensor.h"
#include    "super.h"




#define ENCRYPTKEY          RFM69_KEY   // defined in secret.h
#define IO_TICK_INTERVAL    (100)
#define MIN_SEND_INTERVAL   (2000)


uint8_t key[] = RFM69_KEY;

RH_RF69         rf69(PIN_RFM_CS, PIN_RFM_IRQ);
Modem69         rfm69_modem(&rf69,  PIN_RFM_RESET);

extern main_ctrl_st main_ctrl;
// extern msg_st msg;

r69_st r69 = {0};

void modem_task(void)
{
    rfm69_modem.modem_task();
}


void r69_tx_task(void);
void r69_rx_task(void);

atask_st tx_th                 = {"RFM69 Send     ", 100,0, 0, 255, 0, 1, r69_tx_task};
atask_st rx_th                 = {"RFM69 Receive  ", 100,0, 0, 255, 0, 1, r69_rx_task};
atask_st modem_th              = {"Radio Modem    ", 100,0, 0, 255, 0, 1, modem_task};

void r69_initialize(void)
{

    io_rfm69_spi0_initialize();
    r69.not_send_before = millis() + MIN_SEND_INTERVAL;
    rfm69_modem.set_debug_print(debug_cb_print);
    rfm69_modem.initialize(key);
    rfm69_modem.radiate(__APP__);

    atask_add_new(&modem_th);
    r69.tx_task_indx =  atask_add_new(&tx_th);
    r69.rx_task_indx =  atask_add_new(&rx_th);
    
    // rfm69_modem.radiate(__APP__);

}

void debug_cb_print(const char *msg)
{
    Serial.print("[deb] ");
    Serial.print(msg);
}

void r69_send(char *buff)
{
    memcpy(r69.txbuff, buff, R69_MSG_SIZE);
    r69.send_avail = true;
}

bool r69_ready_to_send(void)
{
    return !r69.send_avail;
}


// sensor_node_et sensor_node_send(void)
// SENSOR_NODE_UNDEFINED

void r69_tx_task(void)
{
 
    switch(tx_th.state)
    {
        case 0:
            tx_th.state = 10;
            break;
        case 10:
            if(r69.send_avail) tx_th.state = 20;
            break;
        case 20:
            rfm69_modem.radiate(r69.txbuff);
            r69.not_send_before = millis() + MIN_SEND_INTERVAL;
            tx_th.state = 30;
            break;
        case 30:
            if(millis() > r69.not_send_before){
                tx_th.state = 10;
                r69.send_avail = false;
            }
            break;
        case 40:
            tx_th.state = 10;
            break;

    }
    super_clear_cntr(SUPER_CNTR_R69);

}

void r69_rx_task(void)
{

    switch(rx_th.state)
    {
        case 0:
            rx_th.state = 10;
            break;
        case 10:
            if(rfm69_modem.msg_is_avail())
            {
                rfm69_modem.get_msg(r69.rxbuff, R69_MSG_SIZE, true);
                Serial.println(r69.rxbuff);
                //msg_split(r69.rxbuff,';');
                // Serial.printf("Split nbr %d\n",msg.field_count);
                // msg_sub_print();
                sensor_process_msg();
                rx_th.state = 20;
            }
            break;
        case 20:
            rx_th.state = 10;
            break;
        case 30:
            rx_th.state = 10;
            break;
        case 40:
            rx_th.state = 10;
            break;

    }
}