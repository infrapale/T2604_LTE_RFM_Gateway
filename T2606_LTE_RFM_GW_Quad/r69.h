#ifndef __R69_H__
#define __R69_H__

#define R69_MSG_SIZE        (60)


typedef struct
{
    uint8_t     tx_task_indx;
    uint8_t     rx_task_indx;
    char        txbuff[R69_MSG_SIZE];
    char        rxbuff[R69_MSG_SIZE];
    bool        send_avail;
    uint16_t    duration;
    uint16_t    not_send_before;

} r69_st;


void r69_initialize(void);

void debug_cb_print(const char *msg);

void r69_send(char *buff);

bool r69_ready_to_send(void);

#endif