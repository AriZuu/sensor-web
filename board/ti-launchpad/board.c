/*
 * Copyright (c) 2014, Ari Suutari <ari@stonepile.fi>.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>

#include <picoos.h>
#include <picoos-u.h>
#include <picoos-net.h>

#include "sensor-web.h"

void ledOn(Led led)
{
#if 0
  switch (led) {
  case RED:
    GPIO0_IOCLR = (1 << 8);
    break;

  case YELLOW:
    GPIO0_IOCLR = (1 << 11);
    break;

  case GREEN:
    GPIO0_IOCLR = (1 << 10);
    break;
  }
#endif
}

void ledOff(Led led)
{
#if 0
  switch (led) {
  case RED:
    GPIO0_IOSET = (1 << 8);
    break;

  case YELLOW:
    GPIO0_IOSET = (1 << 11);
    break;

  case GREEN:
    GPIO0_IOSET = (1 << 10);
    break;
  }
#endif
}

void initBoard()
{
  /*
   * Enable all the GPIO peripherals.
   */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);

  /*
   * Configure UART0 console pins.
   */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  /*
   * We don't need USB. Add a pull down to PD6 to turn off the TPS2052 switch.
   */
  GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_6);
  GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA,
                       GPIO_PIN_TYPE_STD_WPD);

  /*
   * PF0/PF4 are used for Ethernet LEDs.
   */
  GPIOPinConfigure(GPIO_PF0_EN0LED0);
  GPIOPinConfigure(GPIO_PF4_EN0LED1);

  GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);

  /*
   * PJ0 and J1 are used for user buttons
   */
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);

  /*
   * PN0 and PN1 are used for USER LEDs.
   */
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1,
                   GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

  /*
   * Default the LEDs to OFF.
   */
  GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);
  /*
   * PN0 and PN1 are used for USER LEDs.
   */
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  /*
   * OneWire, PK7
   */
  GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_7,
                   GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);

  /*
   * Default output to low.
   */
  GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_7, 0);
}

