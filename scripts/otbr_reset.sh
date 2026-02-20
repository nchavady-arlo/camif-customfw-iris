rm -rf /data/openthread
mkdir -p /data/openthread

echo "ot-ctl factoryreset"
ot-ctl factoryreset
sleep 1

echo "ot-ctl extaddr 166e0a000000000f"
ot-ctl extaddr 166e0a000000000f
sleep 0.5

echo "ot-ctl seteui64 0x4251e000004ff4b8"
ot-ctl seteui64 0x4251e000004ff4b8
sleep 0.5

echo "ot-ctl dataset channelmask 0x7fff800"
ot-ctl dataset channelmask 0x7fff800
sleep 0.5

echo "ot-ctl dataset securitypolicy 3600 onrc"
ot-ctl dataset securitypolicy 3600 onrc
sleep 0.5

echo "ot-ctl dataset channel 12"
ot-ctl dataset channel 12
sleep 0.5

echo "ot-ctl dataset panid 0xfac0"
ot-ctl dataset panid 0xfac0
sleep 0.5

echo "ot-ctl dataset extpanid 000db80000000000"
ot-ctl dataset extpanid 000db80000000000
sleep 0.5

echo "ot-ctl dataset networkname IRIS"
ot-ctl dataset networkname IRIS
sleep 0.5

echo "ot-ctl dataset networkkey 00112233445566778899aabbccddeeff"
ot-ctl dataset networkkey 00112233445566778899aabbccddeeff
sleep 0.5

echo "ot-ctl dataset meshlocalprefix FD00:0DB6:0000:0000::"
ot-ctl dataset meshlocalprefix FD00:0DB6:0000:0000::
sleep 0.5

echo "ot-ctl dataset pskc 01230123012301230123012301230123"
ot-ctl dataset pskc 01230123012301230123012301230123
sleep 0.5

echo "ot-ctl dataset activetimestamp 1"
ot-ctl dataset activetimestamp 1
sleep 0.5

echo "ot-ctl txpower 15"
ot-ctl txpower 15
sleep 0.5

echo "ot-ctl rloc16"
ot-ctl rloc16
sleep 0.5

echo "ot-ctl ipaddr mleid"
ot-ctl ipaddr mleid
sleep 0.5

echo "ot-ctl mode rdn"
ot-ctl mode rdn
sleep 0.5

echo "ot-ctl routerselectionjitter 1"
ot-ctl routerselectionjitter 1
sleep 0.5

echo "ot-ctl dataset commit active"
ot-ctl dataset commit active
sleep 0.5

echo "ot-ctl ifconfig up"
ot-ctl ifconfig up
sleep 0.5

echo "ot-ctl thread start"
ot-ctl thread start
sleep 10
