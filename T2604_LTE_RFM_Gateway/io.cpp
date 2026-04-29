#include <Arduino.h>
#include "atask.h"
#include "main.h"
#include "io.h"

typedef struct
{
  uint8_t   pin;
  uint32_t  pattern;
  uint16_t  tick_nbr;
  bool      enable;
  bool      forever;
} led_st;

typedef struct
{
  uint8_t pattern_bit;
  uint8_t switches;
  uint8_t tindx;
} io_ctrl_st;

io_ctrl_st io_ctrl;

led_st led[COLOR_NBR_OF] =
{
    {PIN_LED_RED, 0, 0, false},
    {PIN_LED_YELLOW, 0, 0, false},
    {PIN_LED_BLUE, 0, 0, false},
};



const uint32_t led_pattern[BLINK_NBR_OF] = 
{
    [BLINK_OFF]       = 0b00000000000000000000000000000000,
    [BLINK_ON]        = 0b11111111111111111111111111111111,
    [BLINK_1_FLASH]   = 0b00000000000000000000000000000001,
    [BLINK_2_FLASH]   = 0b10000000000000001000000000000000,
    [BLINK_4_FLASH]   = 0b10000001000000001000000010000000,
    [BLINK_SLOW]      = 0b11111111111111110000000000000000,
    [BLINK_NORMAL]    = 0b11110000111100001111000011110000,
    [BLINK_FAST]      = 0b11001100110011001100110011001100,
    [BLINK_SOS]       = 0b10010010011100111001110010010010,
    [BLINK_JITTER_1]  = 0b10101010101010101010101010101010,
    [BLINK_JITTER_2]  = 0b10010010010010010010010010010010,
    [BLINK_JITTER_3]  = 0b10001000100010001000100010001000,
};

void io_task(void);
//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st io_task_handle       =   {"I/O Task       ", 100,     0,     0,  255,    0,  1,  io_task };

void io_initialize(void)
{
  analogReadResolution(12);
  //RFM95 Reset
  pinMode(PIN_RFM_RESET, OUTPUT);
  digitalWrite(PIN_RFM_RESET, HIGH);

  pinMode(PIN_PWRKEY, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);

  io_ctrl.pattern_bit = 0;
  for (uint8_t i = COLOR_RED; i <= COLOR_BLUE; i++)
  {
    pinMode(led[i].pin, OUTPUT);
    digitalWrite(led[i].pin, LOW);
  } 
}
void io_task_initialize(void)
{
    //io_ctrl.tindx =  atask_add_new(&io_task_handle);
}


void io_led_flash(color_et color, blink_et bindx, uint16_t tick_nbr)
{
  led[color].pattern = led_pattern[bindx];
  led[color].tick_nbr = tick_nbr;
  if(tick_nbr == BLINK_DISABLE) led[color].enable = false;
  else led[color].enable = true;
  if(tick_nbr == BLINK_FOREVER) led[color].forever = true;
  else led[color].forever = false;
}

void io_task(void)
{

    uint32_t patt = 1UL << io_ctrl.pattern_bit;
    for (uint8_t i = COLOR_RED; i <= COLOR_BLUE; i++)
    {
        if (led[i].enable){
            if ((led[i].tick_nbr > 0) || led[i].forever) {
                if ((led[i].pattern & patt) != 0)
                    digitalWrite(led[i].pin, HIGH);
                else  
                    digitalWrite(led[i].pin, LOW);
            }
            if(!led[i].forever){
                if(led[i].tick_nbr == 0) digitalWrite(led[i].pin, LOW);
                else led[i].tick_nbr--;  
            }
        }
    } 
    if (++io_ctrl.pattern_bit >= 32) io_ctrl.pattern_bit = 0;
}