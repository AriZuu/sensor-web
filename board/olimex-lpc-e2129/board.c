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

#include "lpc_reg.h"
#include "sensor-web.h"

void ledOn(Led led)
{
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
}

void ledOff(Led led)
{
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

}

void initBoard()
{
  /*
   * Led pins to output.
   */

  GPIO0_IOSET = (1 << 8) | (1 << 10) | (1 << 11);
  GPIO0_IODIR |= (1 << 8) | (1 << 10) | (1 << 11);
}

