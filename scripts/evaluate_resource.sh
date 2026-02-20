while [ 1 ]; do free -m; echo 256 > /sys/devices/system/miu/miu0/bw; cat /sys/devices/system/miu/miu0/bw; sleep 2; done
