#!/bin/sh

EPATH="./uacstun.restart"

PIDFILE=${EPATH}.pid
touch $PIDFILE

LOGFILE=${EPATH}.pid

while [ -f $PIDFILE ]; 
do
	echo "try restart stun every 300S..."
	killall uacstun.e
	sleep 300
done
