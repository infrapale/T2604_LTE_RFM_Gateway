#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Arduino.h>
#include "msg.h"

#define MAX_FIELDS          20
#define MAX_FIELD_LEN       16
#define MAX_RAW_MSG_LEN     200


typedef struct 
{
    char        raw[MAX_RAW_MSG_LEN];
    msg_from_et from;
    char        fields[MAX_FIELDS][MAX_FIELD_LEN];
    uint8_t     field_count;
} msg_st;

msg_st msg = {0};

void msg_mod_test(void);

void msg_initialize(void)
{
  msg_mod_test();
}
bool msg_is_valid_char(char c) {
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == ';' || c == '#' || c == '.' || c == '-') return true;
    return false;
}

int msg_strip_to_raw(char *msg_inp)
{
    int len = strlen(msg_inp);
    // Serial.printf("len1: %d ",len);
    if(len == 0 ) return 0;
    int indx = len-1;
    while(msg_inp[indx] == '\n' || msg_inp[indx] == '\r'){
        msg_inp[indx--] = 0x00;
        if(indx < 3) break;
    }
    len = strlen(msg_inp);
    int i = 0;
    while(msg_inp[i] == '\n' || msg_inp[i] == '\r'){
        i++;
        if(i > len -3) break;
    }
    len -= i;
    strncpy(msg.raw, &msg_inp[i],MAX_RAW_MSG_LEN);
    return len;
}

uint8_t msg_split(char *msg_inp) {

    // Serial.print("sg_split() ");
    // Must start with '<' and end with '>'

    int len = msg_strip_to_raw(msg_inp);
    // Serial.printf("len1: %d ",len);
    if(len < 3) return 0;
    
    int f = 0;   // field index
    int i = 0;   // 
    int p = 0;   // position inside field

    // Serial.printf("len3: %d ",len);
    // Serial.printf("Start - End: %c %c ",raw_msg[i], raw_msg[len - 1] );
    if (len < 2 || msg.raw[i] != '<' || msg.raw[len - 1] != '>') return 0;
    i++;
    while (i < len - 1 && f < MAX_FIELDS) {
        char c = msg.raw[i];

        if (c == ';') {
            // End of field
            msg.fields[f][p] = '\0';
            f++;
            p = 0;
        }
        else {
            if (p < MAX_FIELD_LEN - 1) {
                msg.fields[f][p++] = c;
            }
        }

        i++;
    }

    // Final field
    if (f < MAX_FIELDS) {
        msg.fields[f][p] = '\0';
        f++;
    }

    // Serial.printf("..end return %d\n",f);
    return f;
}

void msg_sub_print(void)
{
    Serial.printf("Message fields; %d\n", msg.field_count);
    for (uint8_t i = 0; i < msg.field_count; i++) {
        Serial.printf("[%d] = %s\n", i, msg.fields[i]);
    }
}

void msg_relay_action()
{

}

void msg_process(msg_from_et from, char *raw_msg )
{
    Serial.printf("Message %d: %s\n", from, raw_msg);
    msg.from = from;
    msg.field_count = msg_split(raw_msg);
    msg_sub_print();
    switch(from)
    {
        case MSG_FROM_UART:
            switch(msg.fields[0][0])
            {
                case 'R':
                    break;
                default:
                    break;    
            }
            break;
        case MSG_FROM_RFM:
            break;
        case MSG_FROM_SMS:
            break;
    }

}

typedef struct 
{
    msg_from_et from;
    char   msg[60];
} test_msg_st;

test_msg_st test[8] =
{
      {MSG_FROM_SMS, "<R;MH1;TK1;KOK1;1;KOK2;0;KOK3;1>"},
      {MSG_FROM_SMS, "<R;TK1;ET1;0;TUP1;1;TUP2;1"},
      {MSG_FROM_SMS, "R;TK1;ET1;0;TUP1;1;TUP2;1>"},
      {MSG_FROM_RFM, "\r\n<S;#;PIHA1;T;-12.3;H;44;L;2344>"},
      {MSG_FROM_RFM, "<S;#;RANTA;T;22.3;W;13.4;l;876>\n"},
      {MSG_FROM_RFM, "{A;#;PIHA2;PIR1;1;PIR2;0}"},
      {MSG_FROM_RFM, "<A#TK1OVI1;>"},
      {MSG_FROM_RFM, "<A;#;TK1;OVI1;?>"}
};

void msg_mod_test(void)
{
    Serial.println("msg.cpp module tests:");
    for(uint8_t i = 0; i < 8; i++)
    {
        msg_process(test[i].from, test[i].msg);
    }
}
