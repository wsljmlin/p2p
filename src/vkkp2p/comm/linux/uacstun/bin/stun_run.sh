#!/bin/sh

EPATH="./uacstun.e"

PIDFILE=${EPATH}.pid
touch $PIDFILE

LOGFILE=stun_log

while [ -f $PIDFILE ]; 
do

echo "run $EPATH .... ";
for c in  8 7 6 5 4 3 2 1 0
do
p=`expr $c + 1`
mv $LOGFILE.$c $LOGFILE.$p 2> /dev/null
done
#./tracker.e | tee t.log.0
        $EPATH
sleep 1
done

