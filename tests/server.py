#!/usr/bin/env python3


import subprocess
from utils import success, error, check_open, get_fin_timeout
import time
import socket
import random
import threading


class Client(threading.Thread):
    def __init__(self, port):
        super().__init__()
        self._port = port
        self._failed = False

    def run(self):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5)
            sock.connect(("127.0.0.1", self._port))
            sock.send(bytearray(random.getrandbits(8) for _ in range(100)))
        except:
            self._failed = True

    @property
    def failed(self):
        return self._failed


server = None


def setup():
    global server
    server = subprocess.Popen(
        ["./server"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    time.sleep(5)
    if server.poll() is not None:
        time.sleep(get_fin_timeout())
        server = subprocess.Popen(
            ["./server"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        time.sleep(5)

    if server.poll() is not None:
        error("Failed to start server")
        return False
    return True


def cleanup():
    server.terminate()


def test_single_client():
    global server
    client = Client(5000)
    client.start()
    client.join()

    if server.returncode is not None:
        error("The server failed to handle a single connection")
        return

    if client.failed:
        error("The server failed to handle a single connection")
        return

    success("The server managed to handle a single connection")


def test_multiple_clients():
    global server

    clients = [Client(5000) for _ in range(30)]
    for client in clients:
        client.start()
    for client in clients:
        client.join()

    if server.returncode is not None:
        error("The server failed to handle multiple connections")
        return

    for client in clients:
        if client.failed:
            error("The server failed to handle multiple connections")
            return

    success("The server managed to handle multiple connections")


def main():
    global server
    print("[+] Testing server basic functionality")
    if not setup():
        return
    test_single_client()
    if server.returncode is not None:
        return
    test_multiple_clients()
    cleanup()


if __name__ == "__main__":
    main()
