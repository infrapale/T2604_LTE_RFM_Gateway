#ifndef __RFM69_H__
#define __RFM69_H__
#define FIELD_LEN   8
#define NBR_OF_NODES  4

void rfm69_initialize(void);

void rfm69_print_fields(void);

bool rfm69_split_msg(char *msg, int16_t rssi );

bool rfm69_parse_msg(char *msg, int16_t rssi );

void rfm69_debug_print(void);

#endif