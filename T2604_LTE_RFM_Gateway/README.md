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
New Relay commands over RFM69 or SMS:
   Frame: <R;UNIT;RelayX;1;RealyY;0>
           \\\\   \       \ \      \\______ frame end
            \\\\   \       \ \      \______ function: 1=on, 0=off
             \\\\   \       \ \____________ Relay name (2)
              \\\\   \       \_____________ function: 1=on, 0=off
               \\\\   \____________________ Relay name (1)
                \\\\_______________________ target unit
                 \\\_______________________ separator
                  \\_______________________ message type [1]
                   \_______________________ frame start

Frame overhead:     3
Mesage Type         2
Target unit         5
Frame total         10
Relay name          5
Relay value         2
Relay total         7

Max message len     60
- frame             50
relay limit         50/7  = 7   


<R;MH1;KOK1;1;KOK2;0;KOK3;1>
<R;TK1;TUP1;1;TUP2;1>

*****************************************************************************
New Sensor Messegas over 433MHz or Serial

  Frame: <S;#;PIHA1;T;-12.3;H;44;L;2344>
      S Sensor message  
      # Broadcast
      from   max 5 char
      Value type:
        T Temperature   fp max 2 decimals
        H Humidity      integer 0..100
        L Lux           16 bit integer 
        l LDR           16 bit integer 
        C CO2           16 bit integer 
        ! Remark        max 16 char 
        P PIR           1/0
        N Counter  (16 bit)
      CR+LF is added for Serial messages










*******************************************************************************
Sensor Radio Message:   {"Z":"OD_1","S":"Temp","V":23.1,"R":"-"}
                        {"Z":"Dock","S":"T_dht22","V":"8.7","R":"-"}
Relay Radio Message     {"Z":"MH1","S":"RKOK1","V":"T","R":"-"}
Sensor Node Rx Mesage:  <#X1N:OD1;Temp;25.0;->
Relay Node Rx Mesage:   <#X1N:RMH1;RKOK1;T;->
Time Message:           <##C1T1=;2026;03;12;10;12>
Relay Mesage            <#R12=x>   x:  0=off, 1=on, T=toggle
*******************************************************************************







#RFM69 Messages:
 
 
  <##C1T1=;2026;05;01;18;23>
 
  {"Z":"Dock","S":"T_dht22","V":31.60,"R":""}
  Dock;T_dht22;31.60; RSSI: -92

  {"Z":"VA_OD","S":"Temp","V":19.56,"R":"-"}
  VA_OD;Temp;19.56;- RSSI: -93
