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
#include "sensor-web.h"

static struct uip_eth_addr ethaddr = {

/*
 * Use different ethernet addresses for
 * different cases.
 */

#ifdef VARASTO
  { 0x00, 0xbd, 0x3b, 0x33, 0x05, 0xca }
#else
#ifdef unix
  { 0x00, 0xcd, 0x3b, 0x33, 0x05, 0x75 }
#else
  { 0x00, 0xbd, 0x3b, 0x33, 0x04, 0xd5 }
#endif
#endif

};

static int acceptHook(NetSock* sock, int lport)
{
  POSTASK_t task;

  task = posTaskCreate(lport == 80 ? httpdTask : shellTask, (void*)sock, 2, 1100);
  if (task == NULL) {

    nosPrint("net: out of tasks.");
    return -1;
  }

  POS_SETTASKNAME(task, lport == 80 ? "httpd" : "shell");
  return 0;
}

void initNetwork()
{
  nosPrint("Starting network.\n");

#if UIP_CONF_IPV6

  uip_setethaddr(ethaddr);

#else

  uip_ipaddr_t ipaddr;

#ifdef unix

  uip_ipaddr(&ipaddr, 192,168,0,2);

#else
#ifdef VARASTO

  uip_ipaddr(&ipaddr, 192,168,60,202);

#else

  uip_ipaddr(&ipaddr, 192, 168, 60, 213);

#endif /* VARASTO */
#endif /* unix */

  uip_setethaddr(ethaddr);
  uip_sethostaddr(&ipaddr);

#ifdef unix

  uip_ipaddr(&ipaddr, 192,168,0,1);

#else

  uip_ipaddr(&ipaddr, 192, 168, 60, 1);

#endif /* unix */

  uip_setdraddr(&ipaddr);
  uip_ipaddr(&ipaddr, 255, 255, 255, 0);
  uip_setnetmask(&ipaddr);

#endif /* UIP_CONF_IPV6 */

  netInit();
  netSockAcceptHookSet(acceptHook);

  uip_listen(uip_htons(80));
  uip_listen(uip_htons(23));
}

