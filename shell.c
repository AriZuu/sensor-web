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

#include <picoos.h>
#include <picoos-net.h>
#include "sys/socket.h"

#ifndef unix
#include "lpc_reg.h"
#endif

#include "sensor-web.h"
#include <string.h>

static void help(NetTelnet* t, char* buf)
{
  strcpy(buf, "help       display this list\n");
#if UIP_CONF_STATISTICS == 1
  strcat(buf, "uip-stats  network statistics\n");
#endif
#if POSCFG_FEATURE_DEBUGHELP == 1
  strcat(buf,"os-stats   network statistics\n");
#endif
  strcat(buf, "exit       exit shell\n");

  telnetWrite(t, buf);
}

#if UIP_CONF_STATISTICS == 1
static void ipStats(NetTelnet* t, char* buf)
{
  char* ptr = buf;

  strcpy(ptr, "Prot     Sent Received  Dropped   ChkErr  SynDrop   ReXmit\n");
  ptr += strlen(ptr);
  strcpy(ptr, "==== ======== ======== ======== ======== ======== ========\n");
  ptr += strlen(ptr);

  nosSPrintf(ptr, "IP   %8d %8d %8d\n", (int)uip_stat.ip.sent,
                                         (int)uip_stat.ip.recv,
                                         (int)uip_stat.ip.drop,
                                         (int)uip_stat.ip.chkerr);

  ptr += strlen(ptr);
  nosSPrintf(ptr, "ICMP %8d %8d %8d %8d\n", (int)uip_stat.icmp.sent,
                                             (int)uip_stat.icmp.recv,
                                             (int)uip_stat.icmp.drop,
                                             (int)uip_stat.icmp.chkerr);

  ptr += strlen(ptr);
  nosSPrintf(ptr, "TCP  %8d %8d %8d %8d %8d %8d\n", (int)uip_stat.tcp.sent, 
                                                     (int)uip_stat.tcp.recv,
                                                     (int)uip_stat.tcp.drop,
                                                     (int)uip_stat.tcp.chkerr,
                                                     (int)uip_stat.tcp.syndrop,
                                                     (int)uip_stat.tcp.rexmit);

  ptr += strlen(ptr);
  nosSPrintf(ptr, "UDP  %8d %8d %8d %8d\n", (int)uip_stat.udp.sent,
                                             (int)uip_stat.udp.recv,
                                             (int)uip_stat.udp.drop,
                                             (int)uip_stat.udp.chkerr);

  telnetWrite(t, buf);
}
#endif

#if POSCFG_FEATURE_DEBUGHELP == 1
static void osStats(NetTelnet* t, char* buf)
{
  int taskCount = 0;
  int eventCount = 0;
  struct PICOTASK* task;
  struct PICOEVENT* event;
#ifndef unix
  int freeStack;
  char* sp;
#endif

 // posTaskSchedLock();
  task = picodeb_tasklist;
  while (task != NULL) {

#ifndef unix

    freeStack = 0;

    sp = (char*)task->handle->stack;
    while (*sp == PORT_STACK_MAGIC) {
      ++sp;
      ++freeStack;
    }

    nosSPrintf(buf, "task %s unused stack %d\n", task->name, freeStack);
    telnetWrite(t, buf);

#endif

    taskCount++;
    task = task->next;
  }

  event = picodeb_eventlist;
  while (event != NULL) {

#ifndef unix

    nosSPrintf(buf, "event %s\n", event->name);
    telnetWrite(t, buf);

#endif

    eventCount++;
    event = event->next;
  }

//  posTaskSchedUnlock();

  nosSPrintf(buf, "%d tasks\n%d events/mutexes/semaphores\n", taskCount, eventCount);
  telnetWrite(t, buf);

#if NOSCFG_FEATURE_CPUUSAGE != 0

  nosSPrintf(buf, "CPU usage is %d%%\n", (int)nosCpuUsage());
  telnetWrite(t, buf);

#endif
}
#endif

void shellSessionTask(void* arg)
{
  NetTelnet tel;
  int sock = (intptr_t) arg;
  int i;
  bool go = true;
  static char buf[512];

#ifndef unix
  LED_IOCLR = LED_GREEN;
#endif

  telnetInit(&tel, sock);
  telnetWrite(&tel, "Pico[OS " POS_VER_S "\n");

  do {

    telnetWrite(&tel, "Ready>");
    telnetFlush(&tel);

    i = telnetReadLine(&tel, buf, sizeof(buf), MS(60000));
    if (i <= 0)
      break;

    buf[i - 1] = '\0';

    if (!strcmp(buf, "exit"))
      go = false;
    else if (!strcmp(buf, "help"))
      help(&tel, buf);
#if UIP_CONF_STATISTICS == 1
    else if (!strcmp(buf, "uip-stats"))
      ipStats(&tel, buf);
#endif
#if POSCFG_FEATURE_DEBUGHELP == 1
    else if (!strcmp(buf, "os-stats"))
    osStats(&tel, buf);
#endif
    else
      help(&tel, buf);

  } while (go);

#ifndef unix
  LED_IOSET = LED_GREEN;
#endif

  closesocket(sock);
}

#if UIP_CONF_IPV6
static struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
#endif

void shellTask(void* arg)
{
  int lsn;
  socklen_t addrlen;

#if UIP_CONF_IPV6

  struct sockaddr_in6 me;
  struct sockaddr_in6 peer;

  me.sin6_family = AF_INET6;
  me.sin6_addr = in6addr_any;
  me.sin6_port = htons(23);

#else

  struct sockaddr_in me;
  struct sockaddr_in peer;

  me.sin_family = AF_INET;
  me.sin_addr.s_addr = INADDR_ANY;
  me.sin_port = htons(23);

#endif

  lsn = socket(AF_INET, SOCK_STREAM, 0);
  
  bind(lsn, (struct sockaddr*)&me, sizeof(me));
  listen(lsn, 5);

  while (true) {

    int s = accept(lsn, (struct sockaddr*)&peer, &addrlen);
    if (s == -1)
      continue;

    POSTASK_t task;

    task = posTaskCreate(shellSessionTask, (void*)s, 1, 700);
    if (task == NULL) {

#if NOSCFG_FEATURE_CONOUT == 1
      nosPrint("net: out of tasks.");
#endif
      closesocket(s);
    }

    POS_SETTASKNAME(task, "sess");
  }
}
void initShell()
{
  POSTASK_t task;

  task = posTaskCreate(shellTask, NULL, 2, 300);
  POS_SETTASKNAME(task, "shell");
}

