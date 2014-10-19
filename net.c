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
#include "net/dhcpc.h"

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

void dhcpc_configured(const struct dhcpc_state *s)
{
  nosPrintf("Got IP address %d.%d.%d.%d\n", uip_ipaddr_to_quad(&s->ipaddr));
  nosPrintf("Got netmask %d.%d.%d.%d\n", uip_ipaddr_to_quad(&s->netmask));
  nosPrintf("Got DNS server %d.%d.%d.%d\n", uip_ipaddr_to_quad(&s->dnsaddr));
  nosPrintf("Got default router %d.%d.%d.%d\n", uip_ipaddr_to_quad(&s->default_router));
  nosPrintf("Lease expires in %ld seconds\n", uip_ntohs(s->lease_time[0])*65536ul + uip_ntohs(s->lease_time[1]));

  uip_sethostaddr(&s->ipaddr);
  uip_setnetmask(&s->netmask);
  uip_setdraddr(&s->default_router);
}

void dhcpc_unconfigured(const struct dhcpc_state *s)
{
  nosPrintf("DHCP lease lost.\n");
  uip_ipaddr_t ipaddr;

  uip_ipaddr(&ipaddr, 0,0,0,0);
  uip_sethostaddr(&ipaddr);
}

void initNetwork()
{
  nosPrint("Starting network.\n");

  uip_setethaddr(ethaddr);

#if !UIP_CONF_IPV6 && defined(unix)

  uip_ipaddr_t ipaddr;

  uip_ipaddr(&ipaddr, 192,168,0,2);
  uip_sethostaddr(&ipaddr);

  uip_ipaddr(&ipaddr, 192,168,0,1);
  uip_setdraddr(&ipaddr);

  uip_ipaddr(&ipaddr, 255, 255, 255, 0);
  uip_setnetmask(&ipaddr);

#endif

  netInit();

#if !UIP_CONF_IPV6 && !defined(unix)

  dhcpc_init(&uip_lladdr, sizeof(uip_lladdr));
  dhcpc_request();

#endif
}

