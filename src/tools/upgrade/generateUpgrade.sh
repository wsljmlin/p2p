#!/bin/sh

cp $vodpeerbin/vkkp2p_android.e . -rf

date=`stat vkkp2p_android.e | awk '/\+/{print $1}' | tail -1`

cd upgradeScript
shd1=`./run.sh | awk -F: '/new download file hash/{print $2}'`
echo "shd1=$shd1"
cat > readme.txt << eof
shd1=$shd1
$date
name=vodpeer.e
eof

#genge ate update packet
packet=vodpeerUpgrade.tar.gz
tar cvfz $packet readme.txt vodpeer.e
mv $packet ../
cd -

echo "
********************************************
*********generate vodpeer update packet
*********packet: $packet
********************************************
"
