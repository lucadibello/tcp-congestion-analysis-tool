#!/bin/sh -eu

SCRIPT_NAME="$(basename "$0")"
RATE='10mbit'
INTERFACE='lo'
HELP='no'
CLEAN='no'

function cleanup {
	sudo tc qdisc del dev "$INTERFACE" root
}

# Settings
DELAY="200ms"  # Latency
JITTER="50ms"  # Latency variation delta
LOSS="5%"      # Packet loss likelihood
DUPLICATE="2%" # Likelihood of packet duplication
CORRUPT="1%"   # Packet corruption likelihood
LIMIT="1000"   # Packet queue limit

function setup {
	# Remove previous rate limiting rules
	cleanup

	# Traffic control script
	sudo tc qdisc add dev "$INTERFACE" root netem rate "$RATE" \
		delay "$DELAY" "$JITTER" 50 distribution normal \
		loss "$LOSS" \
		duplicate "$DUPLICATE" \
		corrupt "$CORRUPT" \
		limit "$LIMIT"
}

function print_help {
	cat <<EOF
Usage: $SCRIPT_NAME

-i <iface>, --interface <interface>
                  The interface whose rate needs to be limited (default: lo)
-c, --clean
                  Remove all rate limiting rules from the interface
-r <rate>, --rate <rate>
                  The rate at which the interface should be limited (default: 10mbit)
-h, --help
                  Display this help and exit
EOF
}

while test "$#" -gt 0; do
	case "$1" in
	-c | --clean)
		CLEAN='yes'
		shift
		;;
	-h | --help)
		HELP='yes'
		shift
		;;
	-r | --rate)
		RATE="$2"
		if echo "$RATE" | grep -Evq '^[0-9]+[kmg]bit$'; then
			echo "$RATE is not a valid rate"
			exit 1
		fi
		shift
		shift
		;;
	-i | --interface)
		INTERFACE="$2"
		shift
		shift
		;;
	*)
		echo "Unknown option: $1"
		print_help
		exit 1
		;;
	esac
done

if test "$HELP" = 'yes'; then
	print_help
	exit 0
fi

if test "$CLEAN" = 'yes'; then
	cleanup
	exit 0
fi

setup
etup
