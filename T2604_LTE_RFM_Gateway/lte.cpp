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



// ------------------------------------------------------------
// Utility: Read a full line from LTE modem (with timeout)
// ------------------------------------------------------------
String xxlte_read_line(uint32_t timeout = 1000) {
  String s = "";
  uint32_t start = millis();

  while (millis() - start < timeout) {
    if (LteSerial.available()) {
      char c = LteSerial.read();
      s += c;
      if (c == '\n') break;
    } else {
      delay(2);
    }
  }
  return s;
}

uint16_t lte_read_line(char *lp, uint16_t max_len, uint32_t timeout = 5000)
{
    uint32_t timeout_at = millis() + timeout;
    uint16_t cursor = 0;
    bool      do_continue = true;

    while ((millis() < timeout_at) && do_continue) {
        if (LteSerial.available()) {
            lp[cursor] = LteSerial.read();
            timeout_at = millis() + 50;
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

uint16_t lte_read_response(char *buf, uint16_t max_len, uint32_t timeout = 2000)
{
    uint32_t deadline = millis() + timeout;
    uint16_t cursor = 0;

    while (millis() < deadline && cursor < max_len - 1) {
        if (LteSerial.available()) {
            buf[cursor++] = LteSerial.read();
            deadline = millis() + 50;   // extend timeout while data is flowing
        } else {
            delay(2);
        }
    }

    buf[cursor] = 0;
    return cursor;
}

bool lte_read_sms_header()
{
    char resp[400] = {0};
    uint16_t len = lte_read_response(resp, sizeof(resp));

    Serial.println("FULL RESPONSE:");
    Serial.println(resp);

    // find the header line
    char *hdr = strstr(resp, "+CMGR:");
    if (!hdr) return false;

    // copy only the header line
    char header[200] = {0};
    int i = 0;
    while (hdr[i] && hdr[i] != '\n' && i < 199) {
        header[i] = hdr[i];
        i++;
    }
    header[i] = 0;

    Serial.print("HEADER EXTRACTED: ");
    Serial.println(header);

    return lte_parse_msg_header(&lte_msg, header);
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

// ------------------------------------------------------------
// SMS Reader
// ------------------------------------------------------------
void lte_read_msg(int index) {
//   Serial.print("[ACTION] Reading SMS ");
//   Serial.println(index);

//   lte_flush_serial();
//   LteSerial.print("AT+CMGR=");
//   LteSerial.println(index);

//   String sender = "";
//   String message = "";

//   uint32_t start = millis();
//   while (millis() - start < 2000) {
//     if (!LteSerial.available()) {
//       delay(5);
//       continue;
//     }

//     String line = lte_read_line();
//     String t = line;
//     t.trim();

//     if (t.length() == 0) continue;

//     Serial.print("[SMS] ");
//     Serial.println(t);

//     // Header line
//     if (t.startsWith("+CMGR")) {
//       int q1 = t.indexOf('"');
//       int q2 = t.indexOf('"', q1 + 1);
//       int q3 = t.indexOf('"', q2 + 1);
//       int q4 = t.indexOf('"', q3 + 1);

//       if (q3 > 0 && q4 > 0) {
//         sender = t.substring(q3 + 1, q4);
//         Serial.print("[INFO] Sender: ");
//         Serial.println(sender);
//       }
//     }
//     // Body line
//     else if (!t.startsWith("+") && t != "OK") {
//       message += t + "\n";
//     }
//   }

//   Serial.print("[INFO] Message: ");
//   Serial.println(message);

//   // ----------------------------------------------------------
//   // Auto‑reply
//   // ----------------------------------------------------------
//   if (sender.length() > 0) {
//     Serial.println("[ACTION] Sending auto‑reply…");

//     lte_flush_serial();
//     LteSerial.print("AT+CMGS=\"");
//     LteSerial.print(sender);
//     LteSerial.println("\"");
//     delay(200);

//     LteSerial.print("Auto‑reply: Message received!");
//     LteSerial.write(26); // Ctrl+Z
//     delay(1500);
//   }

//   // ----------------------------------------------------------
//   // Delete SMS
//   // ----------------------------------------------------------
//   Serial.println("[ACTION] Deleting SMS");
//   lte_flush_serial();
//   LteSerial.print("AT+CMGD=");
//   LteSerial.println(index);
//   delay(300);
}

void lte_clear_msg(lte_msg_st *lte_msg)
{
    lte_msg->complete = false;
    memset(lte_msg->sender,0x00,sizeof(lte_msg->sender));
    memset(lte_msg->timestamp,0x00,sizeof(lte_msg->timestamp));
    memset(lte_msg->message,0x00,sizeof(lte_msg->message));
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
    Serial.println(lte_msg->message);

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
bool lte_parse_msg_header(lte_msg_st *lte_msg, const char *msg)
{
    // +CMT: "+358401234567","","24/03/27,12:45:10+08"
    bool parse_res = true;
    if (strncmp(msg, "+CMT", 4) == 0) {

        const char *p = msg;
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
        Serial.print("[parse header] 100: ");
        p = strchr(p + 1, '"'); if (!p) return false;
        Serial.print("[parse header] 101: ");
        p = strchr(p + 1, '"'); if (!p) return false;
        Serial.print("[parse header] 102: ");
        q = strchr(p + 1, '"'); if (!q) return false;
        Serial.print("[parse header] 103: ");
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

        } else parse_res = false;


        Serial.print("[INFO] Timestamp: ");
        Serial.println(lte_msg->timestamp);

        // strncpy(lte_msg->sender, sender, sizeof(lte_msg->sender));
        // strncpy(lte_msg->timestamp, timestamp, sizeof(lte_msg->timestamp));
    } else parse_res = false;
    return parse_res;
}

bool lte_parse_msg_body(lte_msg_st *lte_msg, const char *msg)
{
    bool body_res = false;
    if (msg[0] != '+') {
        // Body line
        strncpy(lte_msg->message, msg, sizeof(lte_msg->message));

        body_res = true;
    }
    return body_res;
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
lte_msg_et lte_read_sms_header() {
    lte_msg_et header_res = LTE_MSG_EMPTY;
    if (LteSerial.available())
    {
        char chline[200] ={0};
        uint16_t len = lte_read_line(chline,200,4000);
        Serial.print("chline: "); Serial.println(chline);
        //line.trim();
        if (len != 0) 
        {
            //   line.toCharArray(chline,200);
            Serial.print("[LTE read header] ");
            Serial.println(chline);
            if (lte_parse_msg_header(&lte_msg,chline)){
                Serial.println("Header OK");
                header_res = LTE_MSG_HEADER_OK;
            }
            else{
                    header_res = LTE_MSG_HEADER_INCORRECT;
                    Serial.println("Header incorrect");

            } 
        }
    }
    return header_res;
}

lte_msg_et lte_read_sms_body() {
  lte_msg_et body_res = LTE_MSG_EMPTY;;
  if (LteSerial.available())
  {
      char chline[200];
      uint16_t len = lte_read_line(chline,200,4000);
      Serial.println(chline);

      if (len != 0) 
      {
          Serial.print("[LTE read body] ");
          Serial.println(chline);
          if (lte_parse_msg_body(&lte_msg,chline)){
              Serial.println("Body OK");
              body_res = LTE_MSG_BODY_OK;
          }
      }
  }
  return body_res;
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
    lte_send_at("AT");
    lte_send_at("ATE0");
    lte_send_at("AT+CPIN=\"1234\"");
    lte_send_at("AT+CMGF=1");          // SMS text mode
    lte_send_at("AT+CNMI=2,2,0,0,0");  // Push SMS immediately 
}

// ------------------------------------------------------------
// Main loop
// ------------------------------------------------------------
void lte_loop() {
    // lte_check_sms();
    // if(lte_msg.complete){
    //     lte_msg.contact_indx = (contact_indx_st)lte_get_contact_index(lte_msg.sender);
    //     lte_print_msg(&lte_msg);
    //     lte_reply_msg(&lte_msg);
    //     lte_clear_msg(&lte_msg);
    // } 
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
            lte_msg.complete = false;
            header_status = lte_read_sms_header();
            switch(header_status)
            {
               case LTE_MSG_HEADER_OK:
                    body_timeout = millis() + 2000;
                    lte_th.state = 110;
                   break;
                case LTE_MSG_EMPTY:
                    break;
                default:    
                    Serial.println("Incorrect Header");
                    break;
            }
            break;
        case 110:
            body_status = lte_read_sms_body();
            switch(body_status)
            {
                case LTE_MSG_BODY_OK: 
                    lte_msg.complete = true;
                    break;
                case LTE_MSG_EMPTY:
                    Serial.println("Empty Body");
                    if (millis() > body_timeout){
                        Serial.println("Body timeout");
                    }
                    break;
                default:
                    Serial.println("Incorrect Body");
                    lte_th.state = 100;
                    break;
            }
            if(lte_msg.complete) lte_th.state = 200;
            break;
        case 200:
            lte_msg.contact_indx = (contact_indx_st)lte_get_contact_index(lte_msg.sender);
            lte_print_msg(&lte_msg);
            //lte_reply_msg(&lte_msg);
            lte_clear_msg(&lte_msg);
            lte_th.state = 100;
            break;

    }
}



