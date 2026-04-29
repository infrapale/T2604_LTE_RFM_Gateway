#ifndef __IO_H__
#define __IO_H__

#define T2601_PICO_RFM69

#ifdef  MCU_PICO_PLUS_2
    #define PIN_TX0     (32u)
    #define PIN_RX0     (33u)
    #define PIN_PWRKEY  (36u)
    #define PIN_RESET   (35u)
#else
    #define PIN_TX0     (0u)
    #define PIN_RX0     (1u)
#endif

#define PIN_I2C1_SDA    (2u)
#define PIN_I2C1_SCL    (3u)

#define PIN_I2C0_SDA    (4u)
#define PIN_I2C0_SCL    (5u)

#define PIN_GP6         (6u)
#define PIN_GP7         (7u)
#define PIN_TX1         (8u)
#define PIN_RX1         (9u)


// LED Definitions
#define LED_NBR_OF      3
#define PIN_LED_RED     (10u)
#define PIN_LED_YELLOW  (11u)
#define PIN_LED_BLUE    (12u)

#define PIN_GP13        (13u)
#define PIN_GP14        (14u)
#define PIN_GP15        (15u)
// RFM69
#define PIN_RFM_MISO    (16u)
#define PIN_RFM_CS      (17u)
#define PIN_RFM_SCK     (18u)
#define PIN_RFM_MOSI    (19u)
#define PIN_RFM_RESET   (20u)
#define PIN_RFM_IRQ     (21u)

#define PIN_EN_DEBUG    (22u)

#define BLINK_DISABLE  (9998)
#define BLINK_FOREVER  (9999)


typedef enum
{
    COLOR_RED = 0,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_NBR_OF
} color_et;

typedef enum
{
  BLINK_OFF = 0,
  BLINK_ON,
  BLINK_1_FLASH,
  BLINK_2_FLASH,
  BLINK_4_FLASH,
  BLINK_SLOW,
  BLINK_NORMAL,
  BLINK_FAST,
  BLINK_SOS,
  BLINK_JITTER_1,
  BLINK_JITTER_2,
  BLINK_JITTER_3,
  BLINK_NBR_OF
} blink_et;

void io_initialize(void);

void io_task_initialize(void);

void io_led_flash(color_et color, blink_et bindx, uint16_t tick_nbr);

void io_task(void);


#endif
