#!/bin/sh
# Simple DTBO generator for embedded Linux systems

# Exit on any error
set -e

# Simple output functions (no colors)
print_info() { echo "[INFO] $1"; }
print_success() { echo "[SUCCESS] $1"; }
print_error() { echo "[ERROR] $1"; }

output_dir="/tmp/cert"   
# Show usage
show_usage() {
    cat << EOF
Simple DTBO Generator for Embedded Linux

Usage: $0 <dts_template> <birth_cert_file> <config_file>

Arguments:
    dts_template      Path to DTS template file
    birth_cert_file   Path to birth certificate file (.crt)
    config_file       Configuration file

Config file format:
	MAC1=AA:BB:CC:DD:EE:FF
	MAC2=AA:BB:CC:DD:EE:FF
	SERIAL=SN1234567890
	MODEL=VMC6060
	HW_VERSION=1.0
	WIFI_CC=US
	SKU=SKU-1234
	UUID=12345678-1234-1234-1234-123456789abc
	LENS_TYPE=standard
	ALS_LLUX=10
	ALS_HLUX=100
	ALS_THLUX=50
	WLED_PWM_LP=20,20
	WLED_PWM_HP=20,20
	WLED_VL_CP=30,30,30
	WLED_VH_HP=40,40,40
EOF
}

# Validate MAC address
validate_mac() {
    case "$1" in
        *[!0-9A-Fa-f:]*) return 1 ;;
        *) 
            # Count colons - should be 5
            colons=$(echo "$1" | tr -cd ':' | wc -c)
            [ "$colons" -eq 5 ] || return 1
            ;;
    esac
    return 0
}

# Validate serial number (13 alphanumeric chars)
validate_serial() {
    [ ${#1} -eq 13 ] || return 1
    case "$1" in
        *[!A-Za-z0-9]*) return 1 ;;
    esac
    return 0
}

# Validate UUID format
validate_uuid() {
    case "$1" in
        ????????-????-????-????-????????????) return 0 ;;
        *) return 1 ;;
    esac
}

# Validate numeric value
validate_numeric() {
    case "$1" in
        ''|,*|*,|*,,*|*[!0-9,]*) return 1 ;;  # Invalid patterns
        *) return 0 ;;  # Valid numeric or CSV numeric
    esac
}

# Load configuration from file
load_config() {
    config_file="$1"
    
    if [ ! -f "$config_file" ]; then
        print_error "Config file not found: $config_file"
        exit 1
    fi

    print_info "Loading config: $config_file"
    
    # Read config file line by line
    while IFS='=' read -r key value; do
        # Skip empty lines and comments
        case "$key" in
            ''|'#'*) continue ;;
        esac
        
        # Remove any quotes from value
        value=$(echo "$value" | sed 's/^"//;s/"$//')
        
        # Set variables
        case "$key" in
            MAC1) MAC1="$value" ;;
			MAC2) MAC2="$value" ;;
            SERIAL) SERIAL="$value" ;;
            MODEL) MODEL="$value" ;;
            HW_VERSION) HW_VERSION="$value" ;;
            WIFI_CC) WIFI_CC="$value" ;;
            SKU) SKU="$value" ;;
            UUID) UUID="$value" ;;
            DESCRIPTION) DESCRIPTION="$value" ;;
            LENS_TYPE) LENS_TYPE="$value" ;;
            PARTNER_ID) PARTNER_ID="$value" ;;
            ALS_LLUX) ALS_LLUX="$value" ;;
            ALS_HLUX) ALS_HLUX="$value" ;;
            ALS_THLUX) ALS_THLUX="$value" ;;
            WLED_PWM_LP) WLED_PWM_LP="$value" ;;
            WLED_PWM_HP) WLED_PWM_HP="$value" ;;
            WLED_VL_CP) WLED_VL_CP="$value" ;;
            WLED_VH_HP) WLED_VH_HP="$value" ;;
        esac
    done < "$config_file"
    
    # Validate required fields
    [ -n "$MAC1" ] || { print_error "MAC1 not set"; exit 1; }
	[ -n "$MAC2" ] || { print_error "MAC2 not set"; exit 1; }
    [ -n "$SERIAL" ] || { print_error "SERIAL not set"; exit 1; }
    [ -n "$UUID" ] || { print_error "UUID not set"; exit 1; }
    
    # Validate formats
    validate_mac "$MAC1" || { print_error "Invalid MAC1 format"; exit 1; }
	validate_mac "$MAC2" || { print_error "Invalid MAC2 format"; exit 1; }
    validate_serial "$SERIAL" || { print_error "Invalid SERIAL format"; exit 1; }
    validate_uuid "$UUID" || { print_error "Invalid UUID format"; exit 1; }
    
    # Validate numeric fields if set
    [ -z "$ALS_LLUX" ] || validate_numeric "$ALS_LLUX" || { print_error "ALS_LLUX must be numeric"; exit 1; }
    [ -z "$ALS_HLUX" ] || validate_numeric "$ALS_HLUX" || { print_error "ALS_HLUX must be numeric"; exit 1; }
    [ -z "$ALS_THLUX" ] || validate_numeric "$ALS_THLUX" || { print_error "ALS_THLUX must be numeric"; exit 1; }
    [ -z "$WLED_PWM_LP" ] || validate_numeric "$WLED_PWM_LP" || { print_error "WLED_PWM_LP must be numeric"; exit 1; }
    [ -z "$WLED_PWM_HP" ] || validate_numeric "$WLED_PWM_HP" || { print_error "WLED_PWM_HP must be numeric"; exit 1; }
    [ -z "$WLED_VL_CP" ] || validate_numeric "$WLED_VL_CP" || { print_error "WLED_VL_CP must be numeric"; exit 1; }
    [ -z "$WLED_VH_HP" ] || validate_numeric "$WLED_VH_HP" || { print_error "WLED_VH_HP must be numeric"; exit 1; }
    
    print_success "Config loaded and validated"
}

# Generate DTBO
generate_dtbo() {
    template_file="$1"
    birth_cert_file="$2"

    output_dts="$output_dir/device_info.dts"
    output_dtbo="$output_dir/device_info.dtbo"
    
    # Create output directory
    mkdir -p "$output_dir"
    
    # Copy birth certificate
    birth_cert_name=$(basename "$birth_cert_file")
    cp "$birth_cert_file" "$output_dir/$birth_cert_name"

    # Copy and process template
    cp "$template_file" "$output_dts"
    
    # Update birth cert path in DTS
    sed -i "s|/incbin/(\"birth_certificate.crt\")|/incbin/(\"$birth_cert_name\")|g" "$output_dts"
	
    # Substitute variables
    sed -i "s/%MAC1%/$MAC1/g" "$output_dts"
	sed -i "s/%MAC2%/$MAC2/g" "$output_dts"
    sed -i "s/%SERIAL%/$SERIAL/g" "$output_dts"
    sed -i "s/%MODEL%/$MODEL/g" "$output_dts"
    sed -i "s/%HW_VERSION%/$HW_VERSION/g" "$output_dts"
    sed -i "s/%WIFI_CC%/$WIFI_CC/g" "$output_dts"
    sed -i "s/%SKU%/$SKU/g" "$output_dts"
    sed -i "s/%UUID%/$UUID/g" "$output_dts"
    sed -i "s/%DESCRIPTION%/$DESCRIPTION/g" "$output_dts"
    sed -i "s/%LENS_TYPE%/$LENS_TYPE/g" "$output_dts"
    sed -i "s/%PARTNER_ID%/$PARTNER_ID/g" "$output_dts"
    sed -i "s/%ALS_LLUX%/$ALS_LLUX/g" "$output_dts"
    sed -i "s/%ALS_HLUX%/$ALS_HLUX/g" "$output_dts"
    sed -i "s/%ALS_THLUX%/$ALS_THLUX/g" "$output_dts"
    sed -i "s/%WLED_PWM_LP%/$WLED_PWM_LP/g" "$output_dts"
    sed -i "s/%WLED_PWM_HP%/$WLED_PWM_HP/g" "$output_dts"
    sed -i "s/%WLED_VL_CP%/$WLED_VL_CP/g" "$output_dts"
    sed -i "s/%WLED_VH_HP%/$WLED_VH_HP/g" "$output_dts"
    
    print_success "DTS generated: $output_dts"
    
    # Check for dtc compiler
    if ! command -v dtc >/dev/null 2>&1; then
        print_error "dtc compiler not found"
        exit 1
    fi
    
    # Compile to DTBO
    print_info "Compiling DTBO..."
    cd "$output_dir"
    if dtc -I dts -O dtb -o device_info.dtbo device_info.dts; then
        cd - >/dev/null
        print_success "DTBO compiled: $output_dtbo"
    else
        cd - >/dev/null
        print_error "DTBO compilation failed"
        exit 1
    fi
}

# Generate DTS from .dtbo
generate_dts() 
{
	dtbo_file="$1"
	dtc -I dtb -O dts -o test.dts -@ dtbo_file
}

# Write .dtbo to factory
write2factory() 
{
	print_success "DTBO write to factory!"

    output_dtbo="$output_dir/device_info.dtbo"

	flash_eraseall /dev/mtd2
	nandwrite -p /dev/mtd2 $output_dtbo
	sync
	sleep 1
}
# Main function
main()
{

    template_file=/data/arlo_cert/iris_device_info_template.dts
    birth_cert_file=/data/arlo_cert/dsc4_encrypt.crt
    config_file=/data/arlo_cert/dts_config.txt

    # Validate files exist
    [ -f "$template_file" ] || { print_error "Template not found: $template_file"; exit 1; }
    [ -f "$birth_cert_file" ] || { print_error "Birth cert not found: $birth_cert_file"; exit 1; }    
    [ -f "$config_file" ] || { print_error "Config not found: $config_file"; exit 1; }
    
    # Check birth cert extension
    case "$birth_cert_file" in
        *.crt|*.CRT) ;;
        *) print_error "Birth cert must have .crt extension"; exit 1 ;;
    esac
	
    print_info "Simple DTBO Generator"
    print_info "Template: $template_file"
    print_info "Birth cert: $birth_cert_file"    
    print_info "Config: $config_file"
    
    # Load config and generate
    load_config "$config_file"
    generate_dtbo "$template_file" "$birth_cert_file"
	
    print_success "DTBO generation complete!"
    echo "Output: ./output/device_info.dtbo"
	write2factory
}

# Run main
main "$@"

