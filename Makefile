#
# Copyright (c) 2012-2013, Ari Suutari <ari@stonepile.fi>.
# All rights reserved. 
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote
#     products derived from this software without specific prior written
#     permission. 
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.

RELROOT = ../picoos/

PORT ?= lpc2xxx
BUILD ?= DEBUG
THUMB=yes

ifeq '$(PORT)' 'lpc2xxx'
LD_SCRIPTS=lpc2129-nova.ld
endif

include $(RELROOT)make/common.mak

NANO = 1
TARGET = sensor-web
SRC_TXT =	sensor-web.c \
		sensors.c disk.c  net.c  shell.c httpd.c webfiles.c

SRC_HDR = 
SRC_OBJ =
SRC_LIB =

ifeq '$(PORT)' 'unix'
CDEFINES += NO_ONEWIRE
endif

CDEFINES += SMALL_MEMORY_TARGET

ifeq '$(PORT)' 'unix'
DIR_USRINC +=	../OneWire/lib/userial 
endif

DIR_CONFIG = $(CURRENTDIR)/config
DIR_OUTPUT = $(CURRENTDIR)/bin
ifneq '$(PORT)' 'unix'
MODULES += ../picoos-ow
endif
MODULES += ../picoos-net ../picoos-micro

ifneq '$(PORT)' 'unix'
POSTLINK1 = arm-none-eabi-size $(TARGETOUT)
POSTLINK2 = arm-none-eabi-objcopy -O ihex $(TARGETOUT) $(TARGET).hex
endif

#
# Generate webfiles.c from www directory files.
# amcharts package must be downloaded from amcharts.com
# and extracted to www/amcharts before running this.
#
ifeq '$(PORT)' 'unix'
webfiles.c: $(wildcard www/*.html www/*.js www/*.css)
	sh genweb.sh > webfiles.c

webfiles:
	sh genweb.sh > webfiles.c
endif

include $(MAKE_OUT)

#
# Distribution zip.
#
dist:
	rm -f ../dist/sensor-web-`date +%Y%m%d`.zip
	cd ..; zip -qr dist/sensor-web-`date +%Y%m%d`.zip sensor-web -x "*/.*" "*/bin/*" "*.launch" "*.patch" "*.fat" "*.hex" "*.cgi" "*/webfiles.c" "*/www/amcharts/*"
