#ifndef __MSG_H__
#define __MSG_H__

#define MSG_MAX_FIELDS          20
#define MSG_MAX_FIELD_LEN       16
#define MSG_MAX_RAW_MSG_LEN     200
#define MSG_MAX_SMS_CMD_LEN     8

typedef enum 
{
    MSG_FROM_UNDEFINED = 0,
    MSG_FROM_UART,
    MSG_FROM_RFM,
    MSG_FROM_SMS,
    MSG_FROM_NBR_OF
} msg_from_et;

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
    char        raw[MSG_MAX_RAW_MSG_LEN];
    msg_from_et from;
    char        fields[MSG_MAX_FIELDS][MSG_MAX_FIELD_LEN];
    uint8_t     field_count;
} msg_st;


typedef struct
{
    char cmd[MSG_MAX_SMS_CMD_LEN];
    sms_cmd_type_et type;
} sms_cmd_st;


void msg_initialize(void);

bool msg_is_valid_char(char c);

uint8_t msg_split(char *msg_inp,  char separator ); 

size_t msg_set_sms_string(char *sms_str);

void msg_process_sms_cmd(void);

void msg_process(msg_from_et from, char *raw_msg );

void msg_sub_print(void);

#endif