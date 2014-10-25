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

#include <string.h>

#ifndef NO_ONEWIRE
#include <picoos-ow.h>
#include <temp10.h>
#endif

#include <picoos.h>
#include <picoos-u.h>
#include <picoos-net.h>
#include "sensor-web.h"

#define TRACENAME "sens:  "

Sensor sensorData[MAX_TEMP];

POSMUTEX_t sensorMutex;
static POSTIMER_t readTimer;
static POSTIMER_t historyTimer;
static POSSEMA_t  sensorSema;


void sensorAddressStr(char* buf, Sensor* sensor)
{
  int i;

  *buf = '\0';
  for (i = 7; i >= 0; i--) {

    nosSPrintf(buf + strlen(buf), "%02X", (int)sensor->serialNum[i]);
    if (i > 0)
      strcat(buf, "-");
   }
}

static NetSock* sensorSock;

static void history(Sensor* sens)
{
  int   i;

  if (sens->tempCount == MAX_TEMP_HISTORY) {
    for (i = 1; i < sens->tempCount; i++) {

      sens->temps[i - 1] = sens->temps[i];
    }

    sens->tempCount--;
  }

  sens->temps[sens->tempCount++] = sens->temp;
}

static void sendTemperature(Sensor* sens)
{
  char	tempBuf[16];

  tempBuf[0] = 1;
  tempBuf[1] = 0;
  tempBuf[2] = 0;
  tempBuf[3] = 0;
  memcpy(tempBuf + 4, sens->serialNum, 8);
  memcpy(tempBuf + 4 + 8, &sens->temp, sizeof(sens->temp));

  netSockWrite(sensorSock, tempBuf, 16);

  if (sens->tempCount == 0) {
  
    sens->tempMin = sens->temp;
    sens->tempMax = sens->temp;
    history(sens);
  }
  else {
 
    if (sens->temp < sens->tempMin)
      sens->tempMin = sens->temp;

   if (sens->temp > sens->tempMax)
      sens->tempMax = sens->temp;
  }

}

void sensorTask()
{
#ifndef NO_ONEWIRE
  uchar	  rslt;
  int	  portNum;
#endif
  int     s;
  int	  cnt;
  int   hourHalfs;
  char    buf[30];
  Sensor* sens;
  uip_ipaddr_t		  ipaddr;
  char trace[80];

  posMutexLock(sensorMutex);
  nosPrint(TRACENAME "OneWire start !\n");
#if UIP_CONF_IPV6
#ifdef unix
  uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0x02bd, 0x5dff, 0xfe93, 0x2900);
#else
  uip_ip6addr(&ipaddr, 0xfd7b, 0xede2, 0x27c7, 0x41, 0x0, 0x0, 0x0, 0xd);
#endif
#else
  uip_ipaddr(&ipaddr, 192, 168, 65, 13);
#endif

#ifdef VARASTO
  sensorSock = netSockCreateUDP(&ipaddr, 700);
#else
  sensorSock = netSockCreateUDP(&ipaddr, 701);
#endif

  posTaskSleep(MS(1000));

#ifdef NO_ONEWIRE

    sensorData[0].serialNum[0] = 1;
    sensorData[0].temp = 22.5;
    sensorData[1].serialNum[0] = 2;
    sensorData[1].temp = 2.5;
    cnt = 2;

#else
#ifdef unix

  portNum = owAcquireEx("/dev/cuaU0");
  if (portNum < 0) {

    nosPrint(TRACENAME "owAcquire failed\n");
    return;
  }

#else
  portNum = 0;
  if (!owAcquire(portNum, NULL)) {

    nosPrint(TRACENAME "owAcquire failed\n");
    return;
  }
#endif

  nosPrint(TRACENAME "OneWire init ok !\n");
  posTaskSleep(MS(1000));
#endif

  posTimerStart(readTimer);
  posTimerStart(historyTimer);
  hourHalfs = 0;

  while (1) {

    ledOff(GREEN);
    posMutexUnlock(sensorMutex);
    posSemaGet(sensorSema);
    posMutexLock(sensorMutex);
    ledOn(GREEN);

    if (posTimerFired(readTimer)) {

#ifndef NO_ONEWIRE

      cnt = 0;
      rslt = owFirst(portNum, TRUE, FALSE);
      sens = sensorData;

      while (rslt) {

        posTaskSleep(MS(100));

        owSerialNum(portNum, sens->serialNum, TRUE);

        rslt = owNext(portNum, TRUE, FALSE);
        cnt++;
        ++sens;
        if (cnt == MAX_TEMP)
           break;
      }
#endif

      if (cnt == 0)
        ledOn(RED);
      else
        ledOff(RED);

      sens = sensorData;
      for (s = 0; s < cnt; s++) {

        posTaskSleep(MS(100));
#if 1
        nosSPrintf(trace, TRACENAME "%d: Serial=", s);
        sensorAddressStr(buf, sens);
        strcat(trace, buf);
#endif
#ifdef NO_ONEWIRE
        sens->temp += 0.5 * (1 + s);
        if (sens->temp > 35)
           sens->temp = 16 - 2.5 * s;
#else

        float value;

        posMutexUnlock(sensorMutex);
        ReadTemperature(0, sens->serialNum, &value);
        posMutexLock(sensorMutex);
        sens->temp = value;
#endif
#if 1
        nosPrintf("%s Temp=%d.%d oC\n", trace, (int)sens->temp, (int)(sens->temp * 10) % 10);
#endif
        sendTemperature(sens);
        sens++;
      }
    }

    if (posTimerFired(historyTimer)) {

      hourHalfs++;
      if (hourHalfs >= 2) {

        hourHalfs = 0;
        sens = sensorData;
        for (s = 0; s < MAX_TEMP; s++) {

          if (sens->serialNum[0] != '\0')
            history(sens);

          sens++;
        }
      }
    }
  }
}

static POSTIMER_t readTimer;
static POSTIMER_t historyTimer;
static POSSEMA_t  sensorSema;

void initSensors()
{
  sensorMutex  = posMutexCreate();
  historyTimer = posTimerCreate();
  readTimer    = posTimerCreate();
  sensorSema   = posSemaCreate(0);

  P_ASSERT("initSensors", sensorMutex != NULL &&
                          historyTimer != NULL &&
                          readTimer != NULL &&
                          sensorSema != NULL);

  POS_SETEVENTNAME(sensorMutex, "sensorMutex");
  POS_SETEVENTNAME(sensorSema, "sensorSema");

  posTimerSet(readTimer, sensorSema, MS(10000), MS(10000));
  posTimerSet(historyTimer, sensorSema, MS(30 * 60 * 1000 ), MS(30 * 60 * 1000));

  memset(sensorData, '\0', sizeof(sensorData));
}
