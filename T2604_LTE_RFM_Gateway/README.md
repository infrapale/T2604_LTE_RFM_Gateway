*******************************************************************************
UART Command Syntax

    rfm_send_msg_st *rx_msg = &send_msg; 
    Frame: <R1F1O2:L>\r\n
            \\\\\\\\______ value, message (optional)
             \\\\\\\______ action set: '=' get: '?' reply: ':'
              \\\\\\______ index
               \\\\\______ function
                \\\\______ sender addr
                 \\\______ sender tag
                  \\______ module_addr target
                   \______ module_tag target

*******************************************************************************

    module_tag  and sender tag identifies the type of module
      R Radio
      Y Relay
      K Keypad
      # All modules
    module_addr and sender addr
      0-9
      # All modules
    Radio Functions
      A Is messages available
      R Read Raw Message
      D Read Decoded Message
      T Transmit Raw Message
      J Send Desoced Message as json
      I Get last RSSI
      S Set parameter
    Index
      0-9
      0 is also used when N/A
    Action
      set: '=' 
      get: '?' 
      reply: ':'
    Value
      Total max 61 characters for RFM69
      '*'  Not applicable or relevant

*******************************************************************************

2026 SMS and RFM433 Message Syntax:
   Frame: <X;Taget;From;func1;value1;func2;value2;...funcn;valuen;>
      X = Message Type
          C = Control
          T = Time
          R = Relay Control
          S = Sensor values
          A = Alarm
      Target/From 1..4 characters
          # = Broadcast
          Examples: TK1, MH1, MH2, K1, KHH, OD1, LH1
      Func  1-6 characters
          Examples:
            Lights:  MH2-1, KHH, PIHA1  
            Sensors:  
              T Temperature   fp max 2 decimals
              H Humidity      integer 0..100
              L Lux           16 bit integer 
              l LDR           16 bit integer 
              C CO2           16 bit integer 
              ! Remark        max 16 char 
              P PIR           1/0
              N Counter  (16 bit)
            Alarms:
              P PIR  
        Value
            Max 16 characters
            Numeric integer or float or '?'
    Example Messages:
      <R;MH1;TK1;KOK1;1;KOK2;0;KOK3;1>
      <R;TK1;ET1;0;TUP1;1;TUP2;1>
      <R;TK1;ET1;0;TUP1;1;TUP2;1>
      <R;RANTA;PUMP;0>                  switch off the water/air pump
      <R;RANTA;PUMP;1>                  switch on the water/air pump
      <R;RANTA;PUMP;3600>               switch on the water/air pump for 1 hour
      <C;KOTONA;n>                      set home mode = n  (TBD)
      <S;#;PIHA1;T;-12.3;H;44;L;2344>
      <S;#;RANTA;T;22.3;W;13.4;l;876>
      <A;#;PIHA2;PIR1;1;PIR2;0>
      <A;#;TK1;OVI1;1>
      <A;#;TK1;OVI1;?>

*******************************************************************************

*****************************************************************************


*******************************************************************************
Sensor Radio Message:   {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                        {"Z":"Dock","S":"T_dht22","V":"8.7","R":"-"}
Relay Radio Message     {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
Sensor Node Rx Mesage:  <#X1N:OD1;Temp;25.0;->
Relay Node Rx Mesage:   <#X1N:RMH1;RKOK1;T;->
Time Message:           <##C1T1=;2026;03;12;10;12>
Relay Mesage            <#R12=x>   x:  0=off, 1=on, T=toggle
*******************************************************************************

              -------------
    SMS       |           | RFM Relay Message
  ----------->|           |---------------->
              |           |
              -------------


              -------------
    SMS       |           | SMS Reply Message
  ----------->|           |---------------->
              |           |
              -------------


SMS Messages
  SMS Control Messages
    KOTONA 1    Set at home state             RFM: <C;#;SMS;KOTONA;1>
    ALARM 0     Clear/set alarms              RFM: <C;#;SMS;ALARM;0>
  SMS Relay Message
    MH11 1      Turn on MH1-1                 RFM: <R;MH1;SMS;MH11;1>
    PUMP 0      Turn off the pump:            RFM: <R;DOCK;SMS;PUMP;0> 
    PUMP 3600   Turn the pump on for 1 hour:  RFM: <R;DOCK;SMS;PUMP;3600> 
  SMS Get Sensor Values
    PIHA1       Get PIHA1 sensor values       SMS: PIHA1;T;-12.3;H;44;L;2344
    REPO1       Get sensor report 1           SMS: PIHA1;T;12.3;TUPA1;T;22.5;WATER;T;9.4 


#RFM69 Messages:
 
 
  <##C1T1=;2026;05;01;18;23>
 
  {"Z":"Dock","S":"T_dht22","V":31.60,"R":""}
  Dock;T_dht22;31.60; RSSI: -92

  {"Z":"VA_OD","S":"Temp","V":19.56,"R":"-"}
  VA_OD;Temp;19.56;- RSSI: -93

include <ctype.h> Gives you:
  isdigit
  isalpha
  isalnum
  toupper, tolower

#include <stdlib.h> Gives you:
  atoi
  atof
  strtol
  strtod

Avoid these unsafe functions: They can overflow buffers silently.
  strcpy
  strcat
  sprintf

Stick to:
  strncpy
  snprintf
  memcpy
