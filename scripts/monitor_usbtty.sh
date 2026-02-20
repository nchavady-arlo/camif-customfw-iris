#!/bin/sh
#
# Auto-start getty on USB gadget serial (ttyGS0)
#

TTY="ttyGS0"
BAUD="115200"
TERM="vt100"
LOG="/var/log/ttyGS0-getty.log"

# 最大回退秒數，避免 getty 一直 crash 吃 CPU
MAX_BACKOFF=10
BACKOFF=2

echo "[INFO] starting getty manager for /dev/$TTY" >> "$LOG"

# 讓 script 可以被正常停止（systemd/busybox 會送 SIGTERM）
trap 'echo "[INFO] stopped" >> "$LOG"; exit 0' INT TERM

while true; do

    # 等裝置出現（熱插拔友善）
    if [ ! -e "/dev/$TTY" ]; then
        sleep 2
        continue
    fi

    echo "[INFO] launching getty on /dev/$TTY" >> "$LOG"

    # -L: local line（不使用 modem 控制線）
    setsid /sbin/getty -L "$TTY" "$BAUD" "$TERM"

    RC=$?
    echo "[WARN] getty exited (rc=$RC)" >> "$LOG"

    # 退避避免瘋狂重啟
    sleep "$BACKOFF"
    [ "$BACKOFF" -lt "$MAX_BACKOFF" ] && BACKOFF=$((BACKOFF + 2))
done