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

#include <stdio.h>
#include <string.h>

#include <picoos.h>
#include <picoos-u.h>
#include <picoos-net.h>

#if UOSCFG_FAT
#include "diskio.h"
#endif

#include "sensor-web.h"

void ledOn(Led led)
{
}

void ledOff(Led led)
{
}

void initBoard()
{
}

#if UOSCFG_FAT
/*
 * FAT fs routines for "disk drive", ie. disk.fat file.
 */
FILE* disk = NULL;

DSTATUS disk_status(BYTE drv)
{
  return 0;
}


DSTATUS disk_initialize(BYTE drv)
{
  disk = fopen("disk.fat", "r");
  if (disk == NULL)
    return STA_NOINIT;

  return 0;
}

DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, UINT count)
{
  int i;

  i = fseek(disk, sector * 512, SEEK_SET);
  i = fread(buff, 512, count, disk);
  return RES_OK;
}

#endif
