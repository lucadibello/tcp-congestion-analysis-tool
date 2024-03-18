import socket
import time

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"

fin_timeout = None


def success(msg):
    global GREEN, RESET
    print(f"  {GREEN}\u2713{RESET} {msg}")


def error(msg):
    global RED, RESET
    print(f"  {RED}\u2717{RESET} {msg}")


def get_fin_timeout():
    global fin_timeout

    if fin_timeout is not None:
        return fin_timeout

    with open("/proc/sys/net/ipv4/tcp_fin_timeout", "r") as fp:
        fin_timeout = int(fp.readline())
        return fin_timeout


def check_open(port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)

    for i in range(3):
        if sock.connect_ex(("127.0.0.1", port)) == 0:
            sock.close()
            return True
        time.sleep(2)
    sock.close()
    return False
