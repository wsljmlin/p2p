#!/bin/sh

touch ./tracker.run

while [ -f ./tracker.run ]; 
do

echo "run tracker .... ";
for c in  8 7 6 5 4 3 2 1 0
do
p=`expr $c + 1`
mv t.log.$c t.log.$p 2> /dev/null
done
./tracker.e
sleep 10
done

