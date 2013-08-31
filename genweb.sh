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

FILES=`cd www; find * -type f -print |
	fgrep -v .AppleDouble |
	fgrep -v .cgi |
	fgrep -v .DS_Store`

echo "#include <picoos.h>"
echo "#include \"webfiles.h\""

FILENO=0
BYTES=0
for F in $FILES
do
   FILENO=`expr $FILENO + 1`
   echo "// $F"
   case $F in
   *.jpg|*.png|*.gif)
      CAT=cat ;;
   *)
      CAT="gzip -c" ;;
   esac

   B=`$CAT www/$F | wc -c`
   echo $F $B >&2
   BYTES=`expr $BYTES + $B`
   $CAT www/$F | file2c -x "static const unsigned char file_$FILENO[] = {" "};"
done

echo ""
echo "const FileEntry files[] = {"

FILENO=0
for F in $FILES
do
   FILENO=`expr $FILENO + 1`
   case $F in
   *.jpg|*.png|*.gif)
      GZIP=false ;;
   *)
      GZIP=true ;;
   esac

   echo "{ \"$F\", $GZIP, file_$FILENO, sizeof(file_$FILENO) },"
done

echo "{ NULL, false, NULL, 0 }"
echo "};"
