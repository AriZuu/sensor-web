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
#include <picoos-u.h>
#include <picoos-net.h>
#include "sensor-web.h"
#include "sys/socket.h"

#define TRACENAME "httpd: "

#include <string.h>

typedef struct
{
  char* fileType;
  char* mimeType;
} Mime;

const Mime mimeTypes[] = {

    { "html", "text/html" },
    { "png", "image/png" },
    { "css", "text/css" },
    { "js", "text/javascript" },
    { "txt", "text/plain" }
};

#define SOCK_BUF_SIZE UIP_TCP_MSS
#define T2I(t) (int)(t)
#define T2D(t) (((int)(10 * t)) % 10)

static int serveCgi(UosFile* sock, char* url, char* buf)
{
  char* ptr;
  int i;
  int j;
  bool first;
  Sensor* s = sensorData;
  char addr[30];

  if (!strcmp(url, "/front_data.cgi")) {

    posMutexLock(sensorMutex);
    ptr = buf;
    for (i = 0; i < MAX_TEMP; i++) {

      if (i == 0) {

        strcpy(ptr, "[");
        ptr = ptr + 1;
      }

      sensorAddressStr(addr, s);
      nosSPrintf(ptr, "{\"sensor\":\"%s\"", addr);
      ptr += strlen(ptr);

      nosSPrintf(ptr, ",\"temp\":%d.%d", T2I(s->temp), T2D(s->temp));
      ptr += strlen(ptr);

      nosSPrintf(ptr, ",\"tempMin\":%d.%d", T2I(s->tempMin), T2D(s->tempMin));
      ptr += strlen(ptr);

      nosSPrintf(ptr, ",\"tempMax\":%d.%d", T2I(s->tempMax), T2D(s->tempMax));
      ptr += strlen(ptr);

      strcpy(ptr, ",\"tempHistory\":[");
      ptr += strlen(ptr);

      j = s->tempCount - 5;
      if (j < 0)
        j = 0;

      first = true;
      for (;j < s->tempCount; j++) {

        if (!first) {

           *ptr = ',';
           ++ptr;
         }

        first = false;
        nosSPrintf(ptr, "%d.%d", T2I(s->temps[j]), T2D(s->temps[j]));
        ptr += strlen(ptr);
      }

      strcpy(ptr, "]}");
      ptr += strlen(ptr);

      if (i == MAX_TEMP - 1 || s[1].serialNum[0] == 0) {
        strcat(ptr, "]");
        ptr += 1;
        uosFileWrite(sock, buf, ptr - buf);
        break;
      }
      else {

        strcat(ptr, ",");
        ptr += 1;
        if (SOCK_BUF_SIZE - (ptr - buf) < 128) {

          uosFileWrite(sock, buf, ptr - buf);
          ptr = buf;
        }
      }

      s++;
    }

    posMutexUnlock(sensorMutex);
  }
  else if (!strcmp(url, "/temp_trend_data.cgi")) {

    posMutexLock(sensorMutex);
    sensorAddressStr(addr, s);
    ptr = buf;
    for (i = 0; i < s->tempCount; i++) {

      if (i == 0) {

        strcpy(ptr, "[");
        ptr = ptr + 1;
      }

      nosSPrintf(ptr, "{\"sensor\":\"%s\",\"timeStamp\":%d,\"temp\":%d.%d}", addr, i - s->tempCount + 1,
                                                                              T2I(s->temps[i]), T2D(s->temps[i]));
      ptr += strlen(ptr);

      if (i == s->tempCount - 1) {

        strcat(ptr, "]");
        ptr += 1;
        uosFileWrite(sock, buf, ptr - buf);
      }
      else {
        strcat(ptr, ",");
        ptr += 1;
        if (SOCK_BUF_SIZE - (ptr - buf) < 128) {

          uosFileWrite(sock, buf, ptr - buf);
          ptr = buf;
        }
      }
    }

    posMutexUnlock(sensorMutex);
  }
  else
    uosFileWrite(sock, "No-No", 5);

  return 0;
}

static int http(UosFile* sock)
{
  char buf[SOCK_BUF_SIZE];
  char* ptr;
  char* tok;
  char req[20];
  char url[80];
  int i;
  int bytes;
  bool cgi;
  bool gz;
  UosFile* file;
  int len;

  i = netSockReadLine(sock, buf, sizeof(buf), MS(5000));
  if (i <= 0) {

    nosPrint(TRACENAME "timeout.\n");
    return -1;
  }

  if (buf[i - 1] != '\n') {
   
    nosPrintf(TRACENAME "request too long, %d bytes: %s\n", i, buf);
    return -1;
  }

  buf[i - 1] = '\0';
  nosPrintf(TRACENAME "%s  mss %d\n", buf, UIP_TCP_MSS);
  ptr = strtok_r(buf, " ", &tok);
  if (ptr == NULL)
    return -1;

  if (strlen(ptr) > sizeof(req) - 1) {

    nosPrint(TRACENAME "verb too long\n");
    return -1;
  }

  strcpy(req, ptr);

  ptr = strtok_r(NULL, " ", &tok);
  if (ptr == NULL)
    return -1;

  if (strlen(ptr) > sizeof(url) - 1) {

    nosPrint(TRACENAME "url too long\n");
    return -1;
  }

  strcpy(url, ptr);

  do {

    i = netSockReadLine(sock, buf, sizeof(buf) - 1, MS(5000));
    if (i <= 0) {
      nosPrint(TRACENAME "timeout reading headers.\n");
      return -1;
    }

    if (buf[i - 1] == '\n')
      buf[i - 1] = '\0';

  } while (buf[0] != '\0');

  if (strcmp(req, "GET")) {

    strcpy(buf, "HTTP/1.1 500 Bad verb\r\n");
    uosFileWrite(sock, buf, strlen(buf));
    return 0;
  }

  if (!strcmp(url, "/"))
    strcpy(url, "/index.html");

  ptr = strchr(url, '?');
  if (ptr != NULL)
    *ptr = '\0';
  if (!strcmp(url + strlen(url) - 4, ".cgi"))
    cgi = true;
  else {
    cgi = false;

    gz = false;
    file = uosFileOpen(url, 0, 0);
    if (file == NULL) {

      strcpy(buf, url);
      strcat(buf, ".gz");
      file = uosFileOpen(buf, 0, 0);
      gz = true;
    }
  }

  if (!cgi && file == NULL) {

    buf[0] = '\0';
    strcat(buf, "HTTP/1.1 500 Not found\r\n");
    strcat(buf, "Server: Pico[OS " POS_VER_S "\r\n");
    strcat(buf, "Connection: close\r\n");
    strcat(buf, "\r\n");
    strcat(buf, "not found");
    uosFileWrite(sock, buf, strlen(buf));
  }
  else {

    buf[0] = '\0';
    strcat(buf, "HTTP/1.1 200 Ok\r\n");
    strcat(buf, "Server: Pico[OS " POS_VER_S "\r\n");
    strcat(buf, "Connection: close\r\n");

    if (!cgi) {
      ptr = strrchr(url, '.');
      if (ptr != NULL) {

        for (i = 0; i < (int)(sizeof(mimeTypes) / sizeof(Mime)); i++)
          if (!strcmp(mimeTypes[i].fileType, ptr + 1)) {

            strcat(buf, "Content-Type: ");
            strcat(buf, mimeTypes[i].mimeType);
            strcat(buf, "\r\n");
            break;
          }
      }

      if (gz)
        strcat(buf, "Content-Encoding: gzip\r\n");
    }
    else
      strcat(buf, "Content-Type: text/plain\r\n");

    if (cgi)
      strcat(buf, "Cache-control: max-age=10\r\n");
    else
      strcat(buf, "Cache-control: max-age=3600\r\n");

    strcat(buf, "\r\n");
    uosFileWrite(sock, buf, strlen(buf));

    if (cgi)
      bytes = serveCgi(sock, url, buf);
    else {

      bytes = 0;
      do {
   
        len = uosFileRead(file, buf, sizeof(buf));
        if (len > 0) {
          bytes += len;
          i = uosFileWrite(sock, buf, len);
        }
      }while (len > 0);

      uosFileClose(file);
    }
    return bytes;
  }

  return 0;
}

void httpClientTask(void* arg)
{
  int sockfd = (intptr_t)arg;
  UosFile* sock = uosFile(sockfd);
  int bytes;

  bytes = http(sock);
  closesocket(sockfd);

  // nosPrintf("http done, %d bytes\n", bytes);
}

#if UIP_CONF_IPV6
static struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
#endif

void httpdTask(void* arg)
{
  int lsn;
  socklen_t addrlen;

#if UIP_CONF_IPV6

  struct sockaddr_in6 me;
  struct sockaddr_in6 peer;

  me.sin6_family = AF_INET6;
  me.sin6_addr = in6addr_any;
  me.sin6_port = htons(80);

#else

  struct sockaddr_in me;
  struct sockaddr_in peer;

  me.sin_family = AF_INET;
  me.sin_addr.s_addr = INADDR_ANY;
  me.sin_port = htons(80);

#endif

  lsn = socket(AF_INET, SOCK_STREAM, 0);
  
  bind(lsn, (struct sockaddr*)&me, sizeof(me));
  listen(lsn, 5);

  while (true) {

    int s = accept(lsn, (struct sockaddr*)&peer, &addrlen);

    if (s == -1)
      continue;

    POSTASK_t task;
    task = posTaskCreate(httpClientTask, (void*)(intptr_t)s, 1, 1200);
    if (task == NULL) {

#if NOSCFG_FEATURE_CONOUT == 1
      nosPrint(TRACENAME "out of tasks.");
#endif
      closesocket(s);
    }

    POS_SETTASKNAME(task, "httpc");
  }
}
void initHttpd()
{
  POSTASK_t task;
  task = posTaskCreate(httpdTask, NULL, 2, 400);
  POS_SETTASKNAME(task, "httpd");
}

