#!/bin/sh

main() 
{
    output_file="/data/nxp/wpa.conf"

    if [ -z "$1" ] || [ -z "$2" ]; then
        echo "Usage: $0 <key> <value>"
        exit 1
    fi

    key="$1"
    value="$2"

    # 如果檔案中已存在該 key，更新成 key="value"
    if grep -q "^${key}=" "$output_file"; then
        sed -i "s|^${key}=.*|${key}=\"${value}\"|" "$output_file"
    else
        # 若不存在則新增一行
        echo "${key}=\"${value}\"" >> "$output_file"
    fi

    echo "[OK] Updated ${key}=\"${value}\""
}

main "$1" "$2"