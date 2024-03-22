#!/bin/sh -eu


SCRIPT_NAME="$(basename "$0")"
RATE='10mbit'
INTERFACE='lo'
HELP='no'
CLEAN='no'

function cleanup {
    sudo tc qdisc del dev "$INTERFACE" root
}

function setup {
    sudo tc qdisc add dev "$INTERFACE" root netem rate "$RATE"
    sudo tc qdisc change dev "$INTERFACE" root netem delay 100ms 50ms 50 distribution normal
    sudo tc qdisc change dev "$INTERFACE" root netem limit 10
    # -- Additional options to force packet loss, duplication, corruption, reordering, and gaps --

    # Tell system to randomly drop 1% of the packets on the specified network interface
		sudo tc qdisc change dev "$INTERFACE" root netem loss 1%
		# Tell system to randomly duplicate 1% of the packets on the specified network interface.
		sudo tc qdisc change dev "$INTERFACE" root netem duplicate 1%
		# Tell system o randomly corrupt 0.1% of the packets on the specified network interface.
		sudo tc qdisc change dev "$INTERFACE" root netem corrupt 0.1%
		# Tell system to randomly reorder 25% of the packets, with a correlation of 50%, on the specified network interface.
		sudo tc qdisc change dev "$INTERFACE" root netem reorder 25% 50%
		# Tell system to introduce gaps between packets, causing a burst of 10 packets to be sent, followed by a gap.
		sudo tc qdisc change dev "$INTERFACE" root netem gap 10
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
	-c|--clean)
	    CLEAN='yes'
	    shift
	    ;;
	-h|--help)
	    HELP='yes'
	    shift
	    ;;
	-r|--rate)
	    RATE="$2"
	    if echo "$RATE" | grep -Evq '^[0-9]+[kmg]bit$'; then
		echo "$RATE is not a valid rate"
		exit 1
	    fi
	    shift
	    shift
	    ;;
	-i|--interface)
	    INTERFACE="$2"
	    shift
	    shift
	    ;;
	*)
	    echo "Unknown option: $1"
	    print_help
	    exit 1
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
