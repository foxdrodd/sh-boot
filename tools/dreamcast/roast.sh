#! /bin/sh
#	$Id: roast.sh,v 1.1 2001/06/07 04:47:07 sugioka Exp $
#	Copyright (C) 2000 YAEGASHI Takeshi <yaegashi@ma.kcom.ne.jp>

SCRAMBLE=./scramble
CDRECORD="cdrecord dev=0,0,0"
MKISOFS=mkisofs

IPIMAGE=IP.BIN
BOOTIMAGE=1ST_READ.BIN

if [ -z "$1" ]; then
    echo "Usage: $0 <boot image to roast>"
    exit 1
fi

if [ ! -f $IPIMAGE ]; then
    echo "You need $IPIMAGE!"
    exit 1
fi

$SCRAMBLE $1 $BOOTIMAGE || exit 1

dd if=/dev/zero bs=2352 count=300 | $CDRECORD -multi -audio - || exit 1

MSINFO=`$CDRECORD -msinfo`

$MKISOFS -C $MSINFO $BOOTIMAGE \
	| ( cat $IPIMAGE ; dd bs=2048 skip=16 ) \
	| $CDRECORD -multi -xa1 -

rm -f $BOOTIMAGE
