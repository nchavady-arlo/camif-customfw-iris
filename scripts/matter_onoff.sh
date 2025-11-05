#!/bin/sh

echo "NODE_ID=$2"
echo "ENDPOINT_ID=$3"

# Usage: ./chip-tool onoff <on|off> <node-id> <endpoint-id>

ACTION=$1
NODE_ID=$2
ENDPOINT_ID=$3

chip-tool onoff "$ACTION" "$NODE_ID" "$ENDPOINT_ID" --storage-directory /config/chip-tool
