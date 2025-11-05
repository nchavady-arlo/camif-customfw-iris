#!/bin/sh

MODE=$1
NODE_ID=$2
SSID=$3
PASSWORD=$4
PIN_CODE=$5
DISCRIMINATOR=$6

case "$MODE" in
  on_network_device)
    echo "MODE=on_network_device"
    echo "NODE_ID=$NODE_ID"
    echo "PIN_CODE=$SSID"  
    chip-tool pairing onnetwork "$NODE_ID" "$SSID" --storage-directory /config/chip-tool
    ;;
  wifi_device)
    echo "MODE=wifi_device"
    echo "NODE_ID=$NODE_ID"
    echo "SSID=$SSID"
    echo "PASSWORD=$PASSWORD"
    echo "PIN_CODE=$PIN_CODE"
    echo "DISCRIMINATOR=$DISCRIMINATOR"
    chip-tool pairing ble-wifi "$NODE_ID" "$SSID" "$PASSWORD" "$PIN_CODE" "$DISCRIMINATOR" --storage-directory /config/chip-tool
    ;;
esac
