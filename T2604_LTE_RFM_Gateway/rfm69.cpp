/**************************************************************************
File:  rfm69.cpp
Function:
  Receive sensor and time data over RFM69 Radio Tranmission
  Update time 
  Save selected sensor values
  Process sensor values
***************************************************************************
Sample messages:
  <##C1T1=;2026;05;01;18;23>
  ;;; RSSI: -59

  {"Z":"Dock","S":"T_dht22","V":31.60,"R":""}
  Dock;T_dht22;31.60; RSSI: -92

  {"Z":"VA_OD","S":"Temp","V":19.56,"R":"-"}
  VA_OD;Temp;19.56;- RSSI: -93


**************************************************************************/

#include <stdlib.h>
#include "main.h"
#include "io.h"
#include "rfm69.h"
#include "uart.h"
#include "atask.h"
#include "secrets.h"
#include <RH_RF69.h>
#include "Rfm69Modem.h"

#define     MSG_MAX_FIELD_NBR   16
#define     MSG_FIELD_LEN       16
#define     BUFF_LEN            80
#define     ENCRYPTKEY          RFM69_KEY   // defined in secret.h
#define     TIME_TX_INTERVAL    60000

#ifdef ENABLE_RFM69
RH_RF69         rf69(RFM69_CS, RFM69_INT);
Rfm69Modem      rfm69_modem(&rf69,  RFM69_RST, -1 );
//modem_data_st   modem_data = {MY_MODULE_TAG, MY_MODULE_ADDR};
#endif

typedef enum
{
    MSG_MATCH_DATE_TIME = 0,
    MSG_MATCH_NBR_OF
} msg_match_et;

typedef enum
{
    MSG_TYPE_UNDEFINED = 0,
    MSG_TYPE_JSON,
    MSG_TYPE_LIST,
    MSG_TYPE_NBR_OF
} msg_type_et;

char msg_field[MSG_MAX_FIELD_NBR][MSG_FIELD_LEN] = {};

typedef struct
{
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     minute;
} date_time_st;

typedef struct
{
    char  match[MSG_FIELD_LEN];
} msg_match_st;


typedef struct
{
    char            mbuff[BUFF_LEN];
    msg_type_et     msg_type;
    int16_t         rssi;
    uint8_t         nbr_fields;
    date_time_st    date_time;
    int8_t          tx_indx;         
} rfm69_ctrl_st;



msg_match_st msg_match[MSG_MATCH_NBR_OF] =
{
    [MSG_MATCH_DATE_TIME] = {.match="##C*T*="},
};
    

rfm69_ctrl_st hctrl = 
{
    .rssi = 0,
    .nbr_fields = 0,
    .date_time = {0,0,0,0,0},
    .tx_indx = -1
};
void handler_task(void);
void modem_69_task(void);
atask_st mth                = {"RFM69 Task     ", 100,0, 0, 255, 0, 1, modem_69_task};
atask_st hth                = {"Handler Task   ", 100,0, 0, 255, 0, 1, handler_task};

uint8_t key[] = RFM69_KEY;

void rfm69_initialize(void)
{
    rfm69_modem.initialize(MY_MODULE_TAG, MY_MODULE_ADDR, key);
    rfm69_modem.radiate(__APP__);
    atask_add_new(&mth);
    hctrl.tx_indx = uart_reserve_tx_buffer(TIME_TX_INTERVAL);
    atask_add_new(&hth);
}

uint8_t rfm69_find_match(char *msg_tag)
{
    uint8_t match_indx = 0;
    uint8_t char_indx = 0;
    bool do_continue_match = true;
    bool do_continue_char = true;
    bool match_found = false;
    uint8_t len = strlen(msg_tag);

    while(do_continue_match && !match_found)
    {
        do_continue_char = true;
        char_indx = 0;
        Serial.printf("%s =?= %s\n",msg_tag,msg_match[match_indx].match);
        while(do_continue_char)
        {
            if(strlen(msg_match[match_indx].match) == len)
            {
                if(msg_match[match_indx].match[char_indx] != '*')
                {
                    if(msg_match[match_indx].match[char_indx] != msg_tag[char_indx])
                        do_continue_char = false;
                }
                char_indx++;
                if(char_indx >= len) {
                    do_continue_char = false;    
                    match_found = true;
                }
            }
            else do_continue_char = false;
        }
        if(!match_found){
            match_indx++;
            if(match_indx >= MSG_MATCH_NBR_OF) do_continue_match= false;
        }
    }

    return match_indx;
}

void rfm69_print_fields(void)
{
    // Serial.printf("Number of fields: %d : ",hctrl.nbr_fields);
    for(uint8_t i = 0; i < hctrl.nbr_fields; i++)
    {
        Serial.printf("%s ",msg_field[i]);
    }
    Serial.println();
}


void rfm69_print_date_time(date_time_st *date_time_p)
{
    Serial.printf("%d-%d-%d %d:%d",
        date_time_p->year ,
        date_time_p->month ,
        date_time_p->day ,
        date_time_p->hour ,
        date_time_p->minute);
}

bool validate_range(uint32_t value, uint32_t min, uint32_t max)
{
    if((value >= min) && (value <= max)) 
        return true;
    else
        return false;
}

bool rfm69_process_fields(void)
{
    bool all_correct = true;
    uint32_t u32;
    uint8_t mindx =rfm69_find_match(msg_field[0]);
    Serial.printf("Match Index %d\n",mindx);
    switch(mindx)
    {
        case MSG_MATCH_DATE_TIME:
            u32 = strtoul(msg_field[1], NULL,10);
            if(validate_range(u32,2026,2999)) hctrl.date_time.year = (uint16_t)u32;
            else all_correct = false;
            u32 = strtoul(msg_field[2], NULL,10);
            if(validate_range(u32,1,12)) hctrl.date_time.month = (uint8_t)u32;
            else all_correct = false;
            u32 = strtoul(msg_field[3], NULL,10);
            if(validate_range(u32,1,31)) hctrl.date_time.day = (uint8_t)u32;
            else all_correct = false;
            u32 = strtoul(msg_field[4], NULL,10);
            if(validate_range(u32,0,24)) hctrl.date_time.hour = (uint8_t)u32;
            else all_correct = false;
            u32 = strtoul(msg_field[5], NULL,10);
            if(validate_range(u32,0,60)) hctrl.date_time.minute = (uint8_t)u32;
            else all_correct = false;
            if(all_correct) Serial.print("Correct Date&Time: ");
            else Serial.print("Inorrect Date&Time: ");
            rfm69_print_date_time(&hctrl.date_time);
            Serial.println();
            break;
    }
    return all_correct;
}
bool rfm69_split_msg(char *msg, int16_t rssi )
{
    bool do_continue = true;
    bool all_done = false;
    String Msg = msg;
    String Sub;
    int indx1 = 1;
    int indx2 = Msg.indexOf(';');
    int indx_end = Msg.indexOf('>');
    uint8_t findx = 0;   // field index

    hctrl.nbr_fields = 0;
    hctrl.rssi = rssi;
    //hctrl.rssi = rssi;
    Msg.trim();
    uint8_t len = Msg.length();
    if(Msg[0] != '<') do_continue = false;
    if(Msg[len-1] != '>') do_continue = false;
    if(indx_end < 0 )  do_continue = false;
    if(!do_continue) Serial.println("Frame was NOK");
    if(indx2 < 2) {
        Serial.println("Message is to short!");
        do_continue = false;
    }

    if(do_continue){
        while(!all_done && do_continue)
        {
            Sub = Msg.substring(indx1,indx2);
            Sub.toCharArray(msg_field[findx], MSG_FIELD_LEN );
            indx1 = indx2+1;
            indx2 = Msg.indexOf(';',indx1+1);
            if(indx2 < 0){
                if(indx1 < indx_end ) indx2 = indx_end;
                else all_done = true;
            }
            findx++;
        }
    }
    hctrl.nbr_fields = findx;
    return do_continue;
}

void modem_69_task(void)
{
    rfm69_modem.modem_task();
}

void handler_task(void)
{
    switch(hth.state)
    {
        case 0:
            hth.state = 10;
            break;
        case 10:
            if(rfm69_modem.msg_is_avail())
            {
                io_led_flash(COLOR_BLUE, BLINK_JITTER_1, 40);
                rfm69_modem.get_msg(hctrl.mbuff, BUFF_LEN, false); 

                switch(hctrl.mbuff[0])
                {
                    case '<':
                        hctrl.msg_type = MSG_TYPE_LIST;
                        rfm69_modem.get_msg(hctrl.mbuff, BUFF_LEN, true);

                        break;
                    case '{':
                        hctrl.msg_type = MSG_TYPE_JSON;
                        rfm69_modem.get_msg_decode(mbuff, BUFF_LEN, true);
                        break;
                    default:
                        hctrl.msg_type = MSG_TYPE_UNDEFINED;
                        rfm69_modem.get_msg(hctrl.mbuff, BUFF_LEN, true);  // clear available
                        break;
                }

                hctrl.rssi = rfm69_modem.get_last_rssi();
                Serial.print(hctrl.mbuff); Serial.print(" RSSI: "); Serial.println(hctrl.rssi);
                //hth.state = 20;
            }    
            break;
        case 20:
            if (rfm69_split_msg(hctrl.mbuff,hctrl.rssi))
            {
                rfm69_print_fields();
                if(rfm69_process_fields()) {
                    // SerialTFT.println(hctrl.mbuff);
                    uart_add_msg(hctrl.tx_indx, hctrl.mbuff);
                    hth.state = 30;
                } else hth.state = 100;
            } else hth.state = 100;
            break;
        case 30:
            hth.state = 10;
            break;
        case 100:    
            hth.state = 10;
            io_led_flash(COLOR_RED, BLINK_NORMAL, 20);
            break;
    }
}