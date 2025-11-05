#!/bin/sh
main()
{
	case "$1" in
	on)
		chip-tool onoff on 1 1 --storage-directory /config/chip-tool
		;;
	off)
		chip-tool onoff off 1 1 --storage-directory /config/chip-tool
		;;
	red)
		chip-tool colorcontrol move-to-hue-and-saturation 0 254 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	orange)
		chip-tool colorcontrol move-to-hue-and-saturation 20 254 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	yellow)
		chip-tool colorcontrol move-to-hue-and-saturation 42 254 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	green)
		chip-tool colorcontrol move-to-hue-and-saturation 85 254 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	blue)
		chip-tool colorcontrol move-to-hue-and-saturation 170 254 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	pink)
		chip-tool colorcontrol move-to-hue-and-saturation 230 200 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	purple)
		chip-tool colorcontrol move-to-hue-and-saturation 200 254 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	white)
		chip-tool colorcontrol move-to-hue-and-saturation 0 0 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	up)
		chip-tool levelcontrol step 0 25 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	down)
		chip-tool levelcontrol step 1 25 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	level)
		brightness=$(echo "$2" | tr -d '[:space:]%' )
		
		case "$brightness" in
			10)  level_val=10 ;;
			20)  level_val=37 ;;
			30)  level_val=65 ;;
			40)  level_val=93 ;;
			50)  level_val=121 ;;
			60)  level_val=149 ;;
			70)  level_val=177 ;;
			80)  level_val=205 ;;
			90)  level_val=231 ;;
			100) level_val=254 ;;
			*)
				echo "Error: Brightness must be one of 10,20,...,100 (got '$2')"
				exit 1
				;;
		esac

		echo "Mapped brightness: ${2}% → Level ${level_val}"
		chip-tool levelcontrol move-to-level "$level_val" 0 0 0 1 1 --storage-directory /config/chip-tool
		;;
	*)
		echo "Matter Light command error $1!"
		;;
	esac
}

main "$@"