// io_BCM2835.c
//
// PIN18 / PWM0 is used to change the brightness of the status_display
// if started as root PWM control is possible , otherwise only on and off

#include <bcm2835.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include "io_BCM2835.h"
// PWM output on RPi Plug P1 pin 12 (which is GPIO pin 18)
// in alt fun 5.
#define PIN 18
// and it is controlled by PWM channel 0
#define PWM_CHANNEL 0
// This controls the max range of the PWM signal
#define RANGE 1024




char dimming_supported=0;


int init_brightness_control(void)
{

    if (getuid() == 0)
      dimming_supported=1;

    if (!bcm2835_init())
        return 1;

    if(dimming_supported==1)
    {
      // Set the output pin to Alt Fun 5, to allow PWM channel 0 to be output there
      bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_ALT5);
      // Clock divider is set to 16.
      // With a divider of 16 and a RANGE of 1024, in MARKSPACE mode,
      // the pulse repetition frequency will be
      // 1.2MHz/1024 = 1171.875Hz, suitable for driving a DC motor with PWM
      bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
      bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
      bcm2835_pwm_set_range(PWM_CHANNEL, RANGE);
    }
    else
    { //no dimming, only switch on and off
      // Set the pin to be an output
      bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_OUTP);

    }

        return 0;
}

void set_display_brightness(unsigned int value)
{
  if(dimming_supported==1)
  {
    if(value==DARKNESS)
    {
      bcm2835_pwm_set_data(PWM_CHANNEL, ((RANGE/10)*(100-DARKNESS_LEVEL))/10); // output is inverted, because of driver
    }
    if(value==BRIGHTNESS)
    {
      bcm2835_pwm_set_data(PWM_CHANNEL, ((RANGE/100)*(100-BRIGHTNESS_LEVEL))/10); // output is inverted, because of driver
    }
  }
  else
  {
    if(value==DARKNESS)
      bcm2835_gpio_write(18, HIGH); // Turn it off, output is inverted, because of driver
    if(value==BRIGHTNESS)
      bcm2835_gpio_write(18, LOW); // Turn it on, output is inverted, because of driver

  }
}

void close_brightness_control(void)
{
    bcm2835_close();
}
