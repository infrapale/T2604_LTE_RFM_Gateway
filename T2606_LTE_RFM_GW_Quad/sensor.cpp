/*******************************************************************************
  sensor.cpp
********************************************************************************
    Sensors:  
      T Temperature   fp max 2 decimals
      H Humidity      integer 0..100
      L Lux           16 bit integer 
      l LDR           16 bit integer 
      C CO2           16 bit integer 
      ! Remark        max 16 char 
      P PIR           1/0
      N Counter       (16 bit)

<S;#;RANTA;T1;24.1>
<S;#;RANTA;T1;24.1;W1;12.8>
<S;#;PIHA;T1;22.1;L1;2009.1>

********************************************************************************

*******************************************************************************/

#include <Arduino.h>
#include "main.h"
#include "msg.h"
#include "modem69.h"
#include "atask.h"




