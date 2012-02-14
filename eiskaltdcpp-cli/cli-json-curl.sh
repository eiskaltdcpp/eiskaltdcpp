#!/bin/sh
# Quick method test script
# based on curl
# by Dmitry Kolosov onyx@z-up.ru 

usage()
{
	echo `basename $0`: ERROR: $* 1>&2
	echo usage: `basename $0` '"method and parameters"'
	exit 1
}

if [ $# -ne 1 ]
then
	usage
fi

SERVER="http://127.0.0.1:3121/"
CURL=/usr/local/bin/curl
METHOD=`echo ${1}|tr -d ' '`

${CURL} --data-binary ${METHOD} -H 'content-type: text/plain;' ${SERVER}

