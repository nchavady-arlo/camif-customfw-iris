#!/bin/sh

# OWI services

#export DEBUG=yes
#PREFIX="/customer/origin"
PREFIX="/origin"
AGENT_CMD="$PREFIX/ow-sensing-agent -p $PREFIX -c $PREFIX/sounder.conf -H $PREFIX"
DO_EXIT=""

echo "nameserver 8.8.8.8" > /tmp/resolv.conf

ifconfig lo up

ntpdate time.nist.gov

mkdir /var/log
#cp -r $PREFIX/etc/cert /etc/
#cp -r /data/third_party_dir/usr/lib/* /lib/

trap 'DO_EXIT=true' TERM INT

ulimit -s 128

#check if this script or the agent is running
EXEC=${0##*/}
THIS_PID=$$

RUNNING_PIDS=$(top -b -n1 | awk '{cmd = match($9, /'$EXEC'|ow-sensing-agent/); if (cmd > 0 && $1 != '$THIS_PID' && $2 != '$THIS_PID') { if (pids == "") pids = $1; else pids = pids " " $1;}}; END {print pids}')

if [ "$RUNNING_PIDS" ]; then
        echo "ow-sensing-agent is running ($RUNNING_PIDS), exiting"
        exit 1
fi

# Check if List-1 file doesn't exist
if [ ! -f /config/origin/wpa_list1.json ]; then
        CHANNEL=$(iw mlan0 info 2>/dev/null | awk '$1 == "channel" {print $2}')
        BSSID=$(iwconfig mlan0 2>/dev/null | grep -oE 'Access Point: ([0-9A-Fa-f:]{17})' | awk '{print $3}')
        MO_MAC=$(ifconfig mlan0 2>/dev/null | grep -oE 'HWaddr ([0-9A-Fa-f:]{17})' | awk '{print $2}')
        IPV4=$(ip route 2>/dev/null | awk '/^default via / {print $3}')

        echo "Channel: $CHANNEL"
        echo "BSSID: $BSSID"
        echo "MO_MAC: $MO_MAC"
        echo "IPV4: $IPV4"

        # Set monitor mode and bring up rtap interface
        mlanutl mlan0 netmon 1 4 127 $CHANNEL
        ifconfig rtap up
        # # call iot scan program
        ./iotscan_temp 20 $MO_MAC $BSSID $IPV4

        mlanutl mlan0 netmon 0
        
fi

# If not, run ow-sensing-agent

echo "running with process stack size $(ulimit -s)"

$AGENT_CMD &
PID=$!

# main loop checks if owi-sensing.sh died
while [ ! "$DO_EXIT" ]; do
        sleep 1
        if ! kill -0 $PID > /dev/null 2>&1 ; then
                echo "$PID died, restarting"
                sleep 3
                $AGENT_CMD &
                PID=$!
        fi
done

kill TERM $PID
echo "Received termination signal, stop owi-sensing service and exit..."
exit 0

