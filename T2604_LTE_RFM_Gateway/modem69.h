#ifndef __MODEM69_H__
#define __MODEM69_H__
#define FIELD_LEN   8
#define NBR_OF_NODES  4

void modem69_initialize(void);

void modem69_print_fields(void);

bool modem69_split_msg(char *msg, int16_t rssi );

bool modem69_parse_msg(char *msg, int16_t rssi );

void modem69_debug_print(void);

#endif