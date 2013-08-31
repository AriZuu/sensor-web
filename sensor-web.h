/*
 * Copyright (c) 2006-2013, Ari Suutari <ari@stonepile.fi>.
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

#ifdef unix

int ledStatus;

#define LED_GREEN	(1 << 10)
#define	LED_RED		(1 << 8)
#define LED_YELLOW	(1 << 11)
#define	LED_IOSET	ledStatus
#define	LED_IOCLR	ledStatus
#define	LED_IODIR	ledStatus

#else

#ifdef VARASTO

// Embedded artists quickstart board

#define LED_GREEN	(1 << 24)
#define LED_YELLOW	(1 << 25)
#define	LED_RED		0
#define	LED_IOSET	GPIO1_IOSET
#define	LED_IOCLR	GPIO1_IOCLR
#define	LED_IODIR	GPIO1_IODIR

#else

// Olimex LPC-E2129

#define LED_GREEN	(1 << 10)
#define	LED_RED		(1 << 8)
#define LED_YELLOW	(1 << 11)
#define	LED_IOSET	GPIO0_IOSET
#define	LED_IOCLR	GPIO0_IOCLR
#define	LED_IODIR	GPIO0_IODIR

#endif
#endif

#define MAX_TEMP 2
#define MAX_TEMP_HISTORY 24

typedef struct {

  uint8_t serialNum[8];
  float   temp;
  float   tempMin;
  float   tempMax;
  float   temps[MAX_TEMP_HISTORY];
  int     tempCount;
} Sensor;

extern Sensor sensorData[MAX_TEMP];
extern POSMUTEX_t sensorMutex;

void httpdTask(void* arg);
void shellTask(void* arg);
void sensorTask(void);
void initNetwork(void);
void initSensors(void);
void sensorAddressStr(char* buf, Sensor* sensor);
