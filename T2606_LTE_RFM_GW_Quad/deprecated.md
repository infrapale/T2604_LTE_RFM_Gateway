ifndef __MODEM69_H__
#define __MODEM69_H__
#define FIELD_LEN   8
#define NBR_OF_NODES  4

#define RFM69_MAX_LEN   60

void modem69_initialize(void);

void modem69_print_fields(void);

bool modem69_split_msg(char *msg, int16_t rssi );

bool modem69_parse_msg(char *msg, int16_t rssi );

void modem69_debug_print(void);

void modem69_radiate(char *msg);

#endif


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
#include "modem69.h"
#include "uart.h"
#include "atask.h"
#include "secrets.h"
#include <RH_RF69.h>
#include "Rfm69Modem.h"
#include "msg.h"

#define     MSG_MAX_FIELD_NBR   16
#define     MSG_FIELD_LEN       16
#define     BUFF_LEN            80
#define     ENCRYPTKEY          RFM69_KEY   // defined in secret.h
#define     TIME_TX_INTERVAL    60000

RH_RF69         rf69(PIN_RFM_CS, PIN_RFM_IRQ);
Rfm69Modem      rfm69_modem(&rf69,  PIN_RFM_RESET, -1 );
//modem_data_st   modem_data = {MY_MODULE_TAG, MY_MODULE_ADDR};

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
    char            rx_mbuff[BUFF_LEN];
    msg_type_et     msg_type;
    int16_t         rssi;
    uint8_t         nbr_fields;
    date_time_st    date_time;
    //int8_t          tx_indx;         
} modem69_ctrl_st;



msg_match_st msg_match[MSG_MATCH_NBR_OF] =
{
    [MSG_MATCH_DATE_TIME] = {.match="##C*T*="},
};
    

modem69_ctrl_st hctrl = 
{
    .rssi = 0,
    .nbr_fields = 0,
    .date_time = {0,0,0,0,0},
};

void handler_task(void);
void modem_69_task(void);
atask_st mth                = {"RFM69 Task     ", 100,0, 0, 255, 0, 1, modem_69_task};
atask_st hth                = {"Handler Task   ", 100,0, 0, 255, 0, 1, handler_task};

uint8_t key[] = RFM69_KEY;

void modem69_initialize(void)
{
    rfm69_modem.initialize(MY_MODULE_TAG, MY_MODULE_ADDR, key);
    rfm69_modem.radiate(__APP__);
    atask_add_new(&mth);
    //hctrl.tx_indx = uart_reserve_tx_buffer(TIME_TX_INTERVAL);
    atask_add_new(&hth);
}

uint8_t modem69_find_match(char *msg_tag)
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

void modem69_print_fields(void)
{
    // Serial.printf("Number of fields: %d : ",hctrl.nbr_fields);
    for(uint8_t i = 0; i < hctrl.nbr_fields; i++)
    {
        Serial.printf("%s ",msg_field[i]);
    }
    Serial.println();
}


void modem69_print_date_time(date_time_st *date_time_p)
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

bool modem69_process_fields(void)
{
    bool all_correct = true;
    uint32_t u32;
    uint8_t mindx =modem69_find_match(msg_field[0]);
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
            modem69_print_date_time(&hctrl.date_time);
            Serial.println();
            break;
    }
    return all_correct;
}
bool modem69_split_msg(char *msg, int16_t rssi )
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
                rfm69_modem.get_msg(hctrl.rx_mbuff, BUFF_LEN, false); 

                switch(hctrl.rx_mbuff[0])
                {
                    case '<':
                        hctrl.msg_type = MSG_TYPE_LIST;
                        rfm69_modem.get_msg(hctrl.rx_mbuff, BUFF_LEN, true);
                        msg_process(MSG_FROM_RFM, hctrl.rx_mbuff);

                        break;
                    case '{':
                        hctrl.msg_type = MSG_TYPE_JSON;
                        rfm69_modem.get_msg_decode(hctrl.rx_mbuff, BUFF_LEN, true);
                        break;
                    default:
                        hctrl.msg_type = MSG_TYPE_UNDEFINED;
                        rfm69_modem.get_msg(hctrl.rx_mbuff, BUFF_LEN, true);  // clear available
                        break;
                }

                hctrl.rssi = rfm69_modem.get_last_rssi();
                Serial.print(hctrl.rx_mbuff); Serial.print(" RSSI: "); Serial.println(hctrl.rssi);
                //hth.state = 20;
            }    
            break;
        case 20:
            if (modem69_split_msg(hctrl.rx_mbuff,hctrl.rssi))
            {
                modem69_print_fields();
                if(modem69_process_fields()) {
                    // SerialTFT.println(hctrl.mbuff);
                    //uart_add_msg(hctrl.tx_indx, hctrl.mbuff);
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

void modem69_radiate(char *msg)
{
    rfm69_modem.radiate(msg);
}

-----------------------------------------------
   if(modem69_modem.msg_is_avail())
    {
        // modem69_modem.get_msg(mbuff, BUFF_LEN, false);
        modem69_modem.get_msg(mbuff, BUFF_LEN, false);    //
        Serial.println(mbuff);
        modem69_modem.get_msg_decode(mbuff, BUFF_LEN, true);
        rssi = modem69_modem.get_last_rssi();
        Serial.print(mbuff); Serial.print(" RSSI: "); Serial.println(rssi);
        //handler_parse_sensor_msg(mbuff, rssi, &node_data );
        //Serial.printf("%s - %s - %s  %d\n", node_data.zone, node_data.item, node_data.value, node_data.rssi);

        // if (handler_parse_msg(mbuff,rssi))
        // {
        //     // handler_process_event();
        // }
        //delay(3000);
        //modem69_modem.radiate_node_json((char*) "<R1X1J1:Dock;T_bmp1;9.1;->");
        //modem69_modem.radiate("OK");
    }

-------------------------------------


#include    <time.h>
#include    "main.h"
#include    "secrets.h"
#include    "io.h"
#include    "atask.h"
#include    "lte.h"

typedef struct
{
    uint16_t prev_state;
    uint16_t retry_cntr;
    uint32_t next_try;
    uint32_t wait_until;
    uint8_t   task_indx;
    time_t   now;
    struct tm timeinfo;
    char    buffer[64];
} lte_st;

lte_st lte ={0};

lte_msg_st lte_msg;

contact_st contact_list[NAME_NBR_OF] =
{
    [NAME_UNKNOWN]= {"+358000000000", "Unknown"},
    [NAME_TOM]    = {"+358405056630", "Tom"},
    [NAME_HESSU]  = {"+358400454270", "Hessu"},
    [NAME_SAULI]  = {"+358400737682", "Sauli"},
};


// Function prototypes
void lte_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st lte_th              =    {"LTE Task       ", 10,     0,     0,  255,    0,  1,  lte_task };

void lte_initialize(void)
{
    LteSerial.begin(115200);
    lte.task_indx =  atask_add_new(&lte_th);
}



uint16_t lte_read_msg()
{
    uint32_t timeout_at = millis() + 2000;
    bool      do_continue = true;


    lte_msg.complete = false;
    lte_msg.cursor = 0;
    // lte_msg.msg 

    if (LteSerial.available())
    {
        while ((millis() < timeout_at) && do_continue) {
            if (LteSerial.available()) {
                lte_msg.msg[lte_msg.cursor] = LteSerial.read();
                if (lte_msg.cursor < MSG_LEN-1) lte_msg.cursor++;
            }   
            else delay(2);
        }
        lte_msg.msg[lte_msg.cursor] = 0;
        Serial.printf("lte_read_msg: %d - %s\n",lte_msg.cursor,lte_msg.msg);
    }
    return lte_msg.cursor;
}

uint16_t lte_read_line(char *lp, uint16_t max_len, uint32_t timeout = 5000)
{
    uint32_t timeout_at = millis() + timeout;
    uint16_t cursor = 0;
    bool      do_continue = true;

    while ((millis() < timeout_at) && do_continue) {
        if (LteSerial.available()) {
            lp[cursor] = LteSerial.read();
            timeout_at += 100;   //millis() + 50;
            if (lp[cursor] == '\n') do_continue = false;
            if (cursor >= max_len-2) do_continue = false;
            cursor++;
        } 
        else delay(2);
    }
    lp[cursor] = 0x00;
    Serial.printf("cursor=%d - %s\n", cursor, lp);
    return cursor;
}

// ------------------------------------------------------------
// Utility: Flush leftover UART data
// ------------------------------------------------------------
void lte_flush_serial(uint32_t timeout = 300) {
  uint32_t start = millis();
  while (millis() - start < timeout) {
    while (LteSerial.available()) {
      LteSerial.read();
      start = millis();
    }
    delay(2);
  }
}

// ------------------------------------------------------------
// Send AT command and print response
// ------------------------------------------------------------
void lte_send_at(const char *cmd, uint32_t wait = 500) {
  lte_flush_serial();
  Serial.print(">> ");
  Serial.println(cmd);
  LteSerial.println(cmd);

  uint32_t start = millis();
  while (millis() - start < wait) {
    if (LteSerial.available()) {
      Serial.write(LteSerial.read());
    }
  }
  Serial.println();
}



void lte_clear_msg(lte_msg_st *lte_msg)
{
    lte_msg->complete = false;
    memset(lte_msg->sender,0x00,sizeof(lte_msg->sender));
    memset(lte_msg->timestamp,0x00,sizeof(lte_msg->timestamp));
    memset(lte_msg->msg,0x00,sizeof(lte_msg->msg));
    memset(lte_msg->body,0x00,sizeof(lte_msg->body));
    lte_msg->date_time.year = 0;
    lte_msg->date_time.month = 0;
    lte_msg->date_time.day = 0;
    lte_msg->date_time.hour = 0;
    lte_msg->date_time.minute = 0;
    lte_msg->date_time.second = 0;

}

void lte_print_msg(lte_msg_st *lte_msg)
{
    if(lte_msg->complete) Serial.print("Complete: ");
    else Serial.print("Draft: ");
    Serial.printf("From: %s %s\n", 
        lte_msg->sender, 
        contact_list[lte_msg->contact_indx].name);
    Serial.printf(" %d-%d-%d %d:%d:%d\n",
        lte_msg->date_time.year,
        lte_msg->date_time.month,
        lte_msg->date_time.day,
        lte_msg->date_time.hour,
        lte_msg->date_time.minute,
        lte_msg->date_time.second);
    Serial.print("Message: ");
    Serial.println(lte_msg->body);

}

uint8_t lte_get_contact_index(char *nbr)
{
    uint8_t contact = NAME_UNKNOWN;
    uint8_t cindx = NAME_UNKNOWN + 1;

    while((contact == NAME_UNKNOWN) && (cindx < NAME_NBR_OF))
    {
        Serial.printf("Compare -%s-%s-\n",nbr,contact_list[cindx].sender);
        if (strcmp(nbr, contact_list[cindx].sender) == 0) contact = cindx;
        else cindx++;
    }
    return contact;
}

char* safe_strchr_mut(char *buf, size_t max_len, char target)
{
    if (!buf) return NULL;

    for (size_t i = 0; i < max_len; i++) {
        if (buf[i] == '\0') return NULL;
        if (buf[i] == target) return &buf[i];
    }
    return NULL;
}


bool lte_parse_msg_header(lte_msg_st *lte_msg, const char *msg)
{
    // +CMT: "+358401234567","","24/03/27,12:45:10+08"
    bool parse_res = true;
    const char *p = msg;
    //Serial.printf("#%s#\n",msg);
    while((*p == '\n') || (*p == '\r') || (*p == ' ')) p++;
    Serial.printf("#%s#\n",p);

    if (strncmp(p, "+CMT", 4) == 0) {

        const char *q;
        char *end_ptr;
        char ch_nbr[8];
        uint16_t  u16;

        // char sender[32] = {0};
        // char timestamp[32] = {0};

        // Find first quoted field (sender)
        p = strchr(p, '"');
        if (!p) return false;
        q = strchr(p + 1, '"');
        if (!q) return false;
        size_t len = q - (p + 1);
        if (len < sizeof(lte_msg->sender))
            memcpy(lte_msg->sender, p + 1, len);

        Serial.print("[INFO] Sender: ");
        Serial.println(lte_msg->sender);

        // Timestamp: 26/04/22,11:08:12+12
        // Find timestamp (4th quoted field)
        p = strchr(q + 1, '"'); if (!p) return false;
        p = strchr(p + 1, '"'); if (!p) return false;
        p = strchr(p + 1, '"'); if (!p) return false;
        q = strchr(p + 1, '"'); if (!q) return false;
        len = q - (p + 1);
        if (len < sizeof(lte_msg->timestamp))
        {
            Serial.print("[parse header] 110: ");
            memcpy(lte_msg->timestamp, p+1, len);
            lte_msg->timestamp[len] = '\0';
            p = lte_msg->timestamp;
            q = strchr(p + 1, '/');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg->date_time.year = u16;

            p = q+1;
            q = strchr(p + 1, '/');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg->date_time.month = (uint8_t)u16;

            p = q+1;
            q = strchr(p + 1, ',');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg->date_time.day = (uint8_t)u16;

            p = q+1;
            q = strchr(p + 1, ':');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg->date_time.hour = (uint8_t)u16;
            p = q+1;
            q = strchr(p + 1, ':');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg->date_time.minute = (uint8_t)u16;
            p = q+1;
            q = strchr(p + 1, '+');
            len = q - p;
            memcpy(ch_nbr, p, len);
            ch_nbr[len] = '\0';
            u16 = (uint16_t)strtol(ch_nbr,&end_ptr,10);
            if (*end_ptr == '\0') lte_msg->date_time.second = (uint8_t)u16;

            // parse body ------------------------------------
            lte_msg->body[0] = '\0'
            char *bodyp = q+1;
            q = safe_strchr_mut(bodyp, 10, '\n');
            //len = strlen(lte_msg->msg)- q;
            //Serial.printf("Body: %s \n", q);
            //strncpy(lte_msg->body, q, SMS_LEN);

        } else parse_res = false;


        Serial.print("[INFO] Timestamp: ");
        Serial.println(lte_msg->timestamp);
        Serial.println(lte_msg->body);

        // strncpy(lte_msg->sender, sender, sizeof(lte_msg->sender));
        // strncpy(lte_msg->timestamp, timestamp, sizeof(lte_msg->timestamp));
    } else {
        parse_res = false;
        Serial.println("+CMT was not found");
    }
    return parse_res;
}

bool lte_parse_msg_body(lte_msg_st *lte_msg, const char *msg)
{
    // Ignore lines that look like headers or empty noise
    if (msg[0] == '+' || msg[0] == '\r' || msg[0] == '\n' || msg[0] == 0) {
        return false;
    }

    // Make a clean copy without trailing CR/LF
    char tmp[SMS_LEN] = {0};
    strncpy(tmp, msg, sizeof(tmp) - 1);

    char *p = tmp;
    while (*p == '\r' || *p == '\n') p++;

    char *end = tmp + strlen(tmp);
    while (end > p && (end[-1] == '\r' || end[-1] == '\n')) {
        *--end = '\0';
    }

    strncpy(lte_msg->msg, p, sizeof(lte_msg->msg) - 1);
    return (lte_msg->msg[0] != 0);
}


void lte_reply_msg(lte_msg_st *lte_msg)
{ 
    char reply[SMS_LEN] = {0};
    char at_cmd[40];

    uint16_t ch_avail = SMS_LEN;
    char *cp;
    uint16_t  len;

    strncpy(reply,"Terve ", ch_avail);
    ch_avail = SMS_LEN - strlen(reply);

    cp = contact_list[lte_msg->contact_indx].name;
    len = strlen(cp);
    if (ch_avail > len) {
        strncat(reply, cp, ch_avail);
        ch_avail = SMS_LEN - strlen(reply);
    }

    cp = "  Tupa Temp = 22.3C";
    len = strlen(cp);
    if (ch_avail > len) {
        strncat(reply, cp, ch_avail);
        ch_avail = SMS_LEN - strlen(reply);
    }

    sprintf(at_cmd, "AT+CMGS=\"%s\"", lte_msg->sender);
    lte_send_at(at_cmd);
    LteSerial.print(reply);
    LteSerial.write(26);
    Serial.printf("Reply: %s\n", reply);

}

// ------------------------------------------------------------
// Check for incoming SMS (CMTI)
// ------------------------------------------------------------
// Consume all pending lines and try to build one complete SMS
lte_msg_et lte_read_sms(void)
{
    char line[200];
    bool header_ok = false;
    bool body_ok   = false;

    while (LteSerial.available()) {
        uint16_t len = lte_read_line(line, sizeof(line), 200);
        if (len == 0) break;

        Serial.print("[LTE line] ");
        Serial.println(line);

        // Header line: +CMT: ...
        if (!header_ok && strncmp(line, "+CMT", 4) == 0) {
            if (lte_parse_msg_header(&lte_msg, line)) {
                Serial.println("Header OK");
                header_ok = true;
            } else {
                Serial.println("Header parse failed");
            }
            continue;
        }

        // Body line after header
        if (header_ok && !body_ok) {
            if (lte_parse_msg_body(&lte_msg, line)) {
                Serial.println("Body OK");
                body_ok = true;
                break;      // we have a complete SMS
            }
        }
    }

    if (header_ok && body_ok) {
        lte_msg.complete = true;
        return LTE_MSG_BODY_OK;
    }
    if (header_ok) {
        return LTE_MSG_HEADER_OK;
    }
    return LTE_MSG_EMPTY;
}



// lte_msg_et lte_read_sms_header() {
//     lte_msg_et header_res = LTE_MSG_EMPTY;
//     if (LteSerial.available())
//     {
//         char chline[200] ={0};
//         uint16_t len = lte_read_line(chline,200,4000);
//         Serial.print("chline: "); Serial.println(chline);
//         //line.trim();
//         if (len != 0) 
//         {
//             //   line.toCharArray(chline,200);
//             Serial.print("[LTE read header] ");
//             Serial.println(chline);
//             if (lte_parse_msg_header(&lte_msg,chline)){
//                 Serial.println("Header OK");
//                 header_res = LTE_MSG_HEADER_OK;
//             }
//             else{
//                     header_res = LTE_MSG_HEADER_INCORRECT;
//                     Serial.println("Header incorrect");

//             } 
//         }
//     }
//     return header_res;
// }

// lte_msg_et lte_read_sms_body() {
//   lte_msg_et body_res = LTE_MSG_EMPTY;;
//   if (LteSerial.available())
//   {
//       char chline[200];
//       uint16_t len = lte_read_line(chline,200,4000);
//       Serial.println(chline);

//       if (len != 0) 
//       {
//           Serial.print("[LTE read body] ");
//           Serial.println(chline);
//           if (lte_parse_msg_body(&lte_msg,chline)){
//               Serial.println("Body OK");
//               body_res = LTE_MSG_BODY_OK;
//           }
//       }
//   }
//   return body_res;
// }

// New: line-based SMS consumer, like in the working sketch
void lte_check_incoming_sms(void)
{
    if (!LteSerial.available()) return;

    char chline[200] = {0};
    uint16_t len = lte_read_line(chline, sizeof(chline), 1000);
    if (len == 0) return;

    // Trim CR/LF
    char *p = chline;
    while (*p == '\r' || *p == '\n') p++;
    char *end = chline + strlen(chline);
    while (end > p && (end[-1] == '\r' || end[-1] == '\n')) *--end = '\0';

    if (*p == 0) return;

    Serial.print("[LTE read] ");
    Serial.println(p);

    lte_parse_msg_header(&lte_msg, p);   // handles +CMT
    lte_parse_msg_body(&lte_msg, p);     // handles body
}



// ------------------------------------------------------------
// Modem Boot Sequence
// ------------------------------------------------------------
void lte_modem_boot() {
  Serial.println("[ACTION] Resetting modem…");
  digitalWrite(PIN_RESET, LOW);
  delay(500);
  digitalWrite(PIN_RESET, HIGH);
  delay(1500);

  Serial.println("[ACTION] Powering modem ON…");
  digitalWrite(PIN_PWRKEY, LOW);
  delay(800);
  digitalWrite(PIN_PWRKEY, HIGH);
  delay(3000);
}


// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void lte_setup() {
    lte_clear_msg(&lte_msg);
    //lte_send_at("AT+IPR=9600");
    lte_send_at("AT");
    lte_send_at("ATE0");
    lte_send_at("AT+CPIN=\"1234\"");
    lte_send_at("AT+CMGF=1");          // SMS text mode
    lte_send_at("AT+CNMI=2,2,0,0,0");  // Push SMS immediately 
}


void lte_task(void)
{
    static lte_msg_et header_status = LTE_MSG_UNDEF;
    static lte_msg_et body_status = LTE_MSG_UNDEF;
    static uint32_t body_timeout;
    
    if(lte_th.state != lte.prev_state){
        Serial.printf("LTE State %d -> %d\n", lte.prev_state, lte_th.state);
        lte.prev_state = lte_th.state;
    }
    switch(lte_th.state)
    {
        case 0:
            lte_th.state = 10;
            lte.wait_until = millis() + 3200;
            Serial.println("=== Pico 2 W + A7683E SMS Framework ===");

            // io_led_flash(COLOR_BLUE, BLINK_JITTER_1, 40);            
            // io_led_flash(COLOR_YELLOW, BLINK_JITTER_2, 40);            
            // io_led_flash(COLOR_RED, BLINK_JITTER_3, 40);
            break;
        case 10:
            Serial.println("[ACTION] Resetting modem…");
            digitalWrite(PIN_RESET, LOW);
            lte.wait_until = millis() + 500;
            lte_th.state = 15;
            break;
        case 15:
            if (millis() > lte.wait_until)
            {
                digitalWrite(PIN_RESET, HIGH);
                lte.wait_until = millis() + 1500;
                lte_th.state = 20;
            }
            break;
        case 20:
            if (millis() > lte.wait_until)
            {
                Serial.println("[ACTION] Powering modem ON…");
                digitalWrite(PIN_PWRKEY, LOW);
                lte.wait_until = millis() + 800;
                lte_th.state = 25;
            }

        case 25:
            if (millis() > lte.wait_until)
            {
                digitalWrite(PIN_PWRKEY, HIGH);
                lte.wait_until = millis() + 3000;
                lte_th.state = 30;
            }
            break;
        case 30:    
            if (millis() > lte.wait_until) lte_th.state = 50;
            break;
        case 50:
            lte_setup();
            Serial.println("[INFO] Setup complete. Waiting for SMS…");
            lte_th.state = 100;
            break;
        case 100:
            if (lte_read_msg() > 20){
                lte_th.state = 110;
            }

            // lte_msg.complete = false;
            // lte_check_incoming_sms();
            // if (lte_msg.complete) {
            //     lte_th.state = 200;
            // }
            break;
            // // Idle, waiting for incoming +CMT
            // lte_msg.complete = false;
            // {
            //     lte_msg_et res = lte_read_sms();
            //     switch (res) {
            //         case LTE_MSG_BODY_OK:
            //             lte_th.state = 200;
            //             break;
            //         case LTE_MSG_HEADER_OK:
            //             // header only so far; stay in 100 and let next tick grab body
            //             break;
            //         case LTE_MSG_EMPTY:
            //         default:
            //             // nothing meaningful yet
            //             break;
            //     }
            // }
            // break;
        case 110:
            //bool lte_parse_msg_header(lte_msg_st *lte_msg, const char *msg)
            if(lte_parse_msg_header(&lte_msg, lte_msg.msg)){ 
                Serial.println("Header is OK");
                lte_print_msg(&lte_msg);
            }
            lte_clear_msg(&lte_msg);
            lte_th.state = 100;
            break;
        case 200:
            lte_msg.contact_indx = (contact_indx_st)lte_get_contact_index(lte_msg.sender);
            lte_print_msg(&lte_msg);
            // lte_reply_msg(&lte_msg);
            lte_clear_msg(&lte_msg);
            lte_th.state = 100;
            break;


        // case 100:
        //     lte_msg.complete = false;
        //     header_status = lte_read_sms_header();
        //     switch(header_status)
        //     {
        //        case LTE_MSG_HEADER_OK:
        //             body_timeout = millis() + 2000;
        //             lte_th.state = 110;
        //            break;
        //         case LTE_MSG_EMPTY:
        //             break;
        //         default:    
        //             Serial.println("Incorrect Header");
        //             break;
        //     }
        //     break;
        // case 110:
        //     body_status = lte_read_sms_body();
        //     switch(body_status)
        //     {
        //         case LTE_MSG_BODY_OK: 
        //             lte_msg.complete = true;
        //             break;
        //         case LTE_MSG_EMPTY:
        //             Serial.println("Empty Body");
        //             if (millis() > body_timeout){
        //                 Serial.println("Body timeout");
        //             }
        //             break;
        //         default:
        //             Serial.println("Incorrect Body");
        //             lte_th.state = 100;
        //             break;
        //     }
        //     if(lte_msg.complete) lte_th.state = 200;
        //     break;
        // case 200:
        //     lte_msg.contact_indx = (contact_indx_st)lte_get_contact_index(lte_msg.sender);
        //     lte_print_msg(&lte_msg);
        //     //lte_reply_msg(&lte_msg);
        //     lte_clear_msg(&lte_msg);
        //     lte_th.state = 100;
        //     break;

    }
}



