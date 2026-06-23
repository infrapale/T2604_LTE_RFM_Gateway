#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Arduino.h>
#include <time.h>
#include "main.h"
#include "msg.h"
#include "modem69.h"
#include "atask.h"

#define MSG_MAX_FIELDS          20
#define MSG_MAX_FIELD_LEN       16
#define MSG_MAX_RAW_MSG_LEN     200
#define MSG_MAX_SMS_CMD_LEN     8


typedef struct 
{
    char        raw[MSG_MAX_RAW_MSG_LEN];
    msg_from_et from;
    char        fields[MSG_MAX_FIELDS][MSG_MAX_FIELD_LEN];
    uint8_t     field_count;
} msg_st;


typedef enum
{
    SMS_CMD_HOME = 0,
    SMS_CMD_RELAY_PUMP,
    SMS_CMD_RELAY_PEER,
    SMS_CMD_SENSOR_PIHA1,
    SMS_CMD_SENSOR_REPO1,
    SMS_CMD_SENSOR_REPO2,
    SMS_CMD_NBR_OF
} sms_cmd_type_et;

typedef struct
{
    char cmd[MSG_MAX_SMS_CMD_LEN];
    sms_cmd_type_et type;
} sms_cmd_st;

extern main_ctrl_st main_ctrl;

sms_cmd_st sms_cmd[SMS_CMD_NBR_OF] =
{
    {"HOME",    SMS_CMD_HOME},
    {"PUMP",    SMS_CMD_RELAY_PUMP},
    {"PEER",    SMS_CMD_RELAY_PEER},
    {"PIHA1",   SMS_CMD_SENSOR_PIHA1},
    {"REPO1",   SMS_CMD_SENSOR_REPO1},
    {"REPO2",   SMS_CMD_SENSOR_REPO2},
};

void msg_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st msg_th              =    {"Message Task   ", 5000,    0,     0,  255,    0,  1,  msg_task };


msg_st msg = {0};

void msg_mod_test(void);

void msg_initialize(void)
{
  //msg_mod_test();
  // atask_add_new(&msg_th);
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
    strncpy(msg.raw, &msg_inp[i],MSG_MAX_RAW_MSG_LEN);
    return len;
}

uint8_t msg_split(char *msg_inp, char separator = ';') {

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
    while (i < len - 1 && f < MSG_MAX_FIELDS) {
        char c = msg.raw[i];

        if (c == separator) {
            // End of field
            msg.fields[f][p] = '\0';
            f++;
            p = 0;
        }
        else {
            if (p < MSG_MAX_FIELD_LEN - 1) {
                msg.fields[f][p++] = c;
            }
        }

        i++;
    }

    // Final field
    if (f < MSG_MAX_FIELDS) {
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
    Serial.println("msg_relay_action");

}

void str_to_upper(char str[]) {
    uint16_t i = 0;
    while (str[i] != '\0') { 
        str[i] = toupper(str[i]); // Convert character to uppercase
        i++;
    }
}
size_t msg_set_sms_string(char *sms_str)
{
    str_to_upper(sms_str);
    Serial.printf("msg_set_sms_string: %s\n", sms_str);
    msg.raw[0] = '<';
    strncpy(&msg.raw[1],sms_str, MSG_MAX_RAW_MSG_LEN-1);
    size_t len = strnlen(msg.raw,MSG_MAX_RAW_MSG_LEN);
    msg.raw[len++] = '>';
    msg.raw[len++] = 0x00;
    msg_split(msg.raw, ' ');
    Serial.printf(" ... %s\n", msg.raw);
    // msg_sub_print();
    return len;
} 

void msg_process_sms_cmd(void)
{
    int cmd_indx = -1; 
    int16_t param;
    char    buff[RFM69_MAX_LEN];

    Serial.println("msg_process_sms_cmd");
    for(uint8_t i = 0; ((i < SMS_CMD_NBR_OF) && (cmd_indx == -1)); i++)
    {
        if(strncmp(msg.fields[0], sms_cmd[i].cmd, MSG_MAX_SMS_CMD_LEN) == 0) cmd_indx = i;
        Serial.printf("sms %s - %s: %d\n", msg.fields[0], sms_cmd[i].cmd, cmd_indx);
    }
    Serial.printf("cmd_indx= %d\n", cmd_indx);

    if (cmd_indx != -1)
    {
        param = atoi(msg.fields[1]);
        switch(cmd_indx)
        {
            case SMS_CMD_HOME:
                break;
            case SMS_CMD_RELAY_PUMP:
                sprintf(buff,"<R;RANTA;%s;PUMP;%d>", main_ctrl.my_addr, param);
                modem69_radiate(buff);
                Serial.println(buff);
                break;
            case SMS_CMD_RELAY_PEER:
                sprintf(buff,"<R;RANTA;%s;PEER;%d>", main_ctrl.my_addr, param);
                modem69_radiate(buff);
                Serial.println(buff);
                break;
            case SMS_CMD_SENSOR_PIHA1:
                sprintf(buff,"<S;#;PIHA1;T;-12.3;H;44;L;2344>");
                Serial.println(buff);
                break;
            case SMS_CMD_SENSOR_REPO1:
                sprintf(buff,"<S;#;OD1;T;22.3;W;13.4;l;876>");
                Serial.println(buff);
                break;
            case SMS_CMD_SENSOR_REPO2:
                sprintf(buff,"<S;#;REPO2;T;22.3;W;13.4;l;876>");
                Serial.println(buff);
                break;
            default:
                break;
        }

    }

}

void  msg_time_action(void)
{
    Serial.printf("Time: %s\n", msg.raw);
    main_ctrl.timeinfo.tm_year  = atoi(msg.fields[3]);
    main_ctrl.timeinfo.tm_mon   = (uint8_t)atoi(msg.fields[4]);    
    main_ctrl.timeinfo.tm_mday  = (uint8_t)atoi(msg.fields[5]);
    main_ctrl.timeinfo.tm_hour  = (uint8_t)atoi(msg.fields[6]);
    main_ctrl.timeinfo.tm_min   = (uint8_t)atoi(msg.fields[7]);
}

void msg_process(msg_from_et from, char *raw_msg )
{
    //Serial.printf("Message1 %d: %s\n", from, raw_msg);

    msg.from = from;
    switch(from) 
    {
        case MSG_FROM_UART:
            strncpy(msg.raw, raw_msg, MSG_MAX_RAW_MSG_LEN);
            break;
        case MSG_FROM_RFM:
            strncpy(msg.raw, raw_msg, MSG_MAX_RAW_MSG_LEN);
            break;
        case MSG_FROM_SMS:
             msg_set_sms_string(raw_msg);
            break;
    } 
    //Serial.printf("Message2 %d: %s\n", from, msg.raw);

    msg.field_count = msg_split(msg.raw);
    // msg_sub_print();
    switch(from)
    {
        case MSG_FROM_UART:
            switch(msg.fields[0][0])
            {
                case 'R':
                    msg_relay_action();
                    break;
                default:
                    break;    
            }
            break;
        case MSG_FROM_RFM:
            switch(msg.fields[0][0])
            {
                case 'R':
                    msg_relay_action();
                    break;
                case 'T':
                    msg_time_action();
                default:
                    break;    
            }
            break;
        case MSG_FROM_SMS:
            msg_process_sms_cmd();
            break;
    }

}

//     MH11 1      Turn on MH1-1                 RFM: <R;MH1;MH11;1>
//     PUMP 0      Turn off the pump:            RFM: <R;Dock;PUMP;0> 
//     PUMP 3600   Turn the pump on for 1 hour:  RFM: <R;Dock;PUMP;3600> 
//   SMS Get Sensor Values
//     PIHA1       Get PIHA1 sensor values       SMS: <S;#;PIHA1;T;-12.3;H;44;L;2344>
//     REPO1       Get sensor report 1           SMS: <S;#;PIHA1;T;12.3;TUPA1;T;22.5;WATER;T;9.4 >


typedef struct 
{
    msg_from_et from;
    char   msg[60];
} test_msg_st;

#define TEST_MSG_NBR_OF 8
test_msg_st test[TEST_MSG_NBR_OF] =
{
      {MSG_FROM_SMS, "PUMP;1"},
      {MSG_FROM_SMS, "PUMP;100"},
      {MSG_FROM_SMS, "PUMP;0"},
      {MSG_FROM_RFM, "\r\n<S;#;PIHA1;T;-12.3;H;44;L;2344>"},
      {MSG_FROM_RFM, "<S;#;RANTA;T;22.3;W;13.4;l;876>\n"},
      {MSG_FROM_RFM, "{A;#;PIHA2;PIR1;1;PIR2;0}"},
      {MSG_FROM_RFM, "<A#TK1OVI1;>"},
      {MSG_FROM_RFM, "<A;#;TK1;OVI1;?>"}
};

void msg_mod_test(void)
{
    Serial.println("msg.cpp module tests:");
    for(uint8_t i = 0; i < TEST_MSG_NBR_OF; i++)
    {
        msg_process(test[i].from, test[i].msg);
    }
}

void msg_task(void)
{
    static uint8_t indx = 0;

    msg_process(test[indx].from, test[indx].msg);
    indx++;
    if(indx >= TEST_MSG_NBR_OF) indx = 0;


}
