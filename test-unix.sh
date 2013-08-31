#!/bin/sh

ifconfig tap0 destroy
ifconfig tap0 create
ifconfig tap0 ether 00:bd:5d:93:29:00
ifconfig tap0 inet6 fd7b:ede2:27c7:009c::4 prefixlen 64

echo "try ping6 fe80::2cd:3bff:fe33:575%tap0 "
bin/unix-deb/out/sensor-web

