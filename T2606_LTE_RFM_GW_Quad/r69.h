#ifndef __R69_H__
#define __R69_H__

#define R69_MSG_SIZE        (60)

void r69_initialize(void);

void debug_cb_print(const char *msg);

void r69_send(char *buff);

bool r69_ready_to_send(void);

#endif