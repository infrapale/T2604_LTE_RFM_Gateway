#ifndef __MSG_H__
#define __MSG_H__

typedef enum 
{
    MSG_FROM_UNDEFINED = 0,
    MSG_FROM_UART,
    MSG_FROM_RFM,
    MSG_FROM_SMS,
    MSG_FROM_NBR_OF
} msg_from_et;

void msg_initialize(void);

bool msg_is_valid_char(char c);

uint8_t msg_split(const char *msg);

void msg_process(msg_from_et from, char *raw_msg );

void msg_sub_print(void);

#endif