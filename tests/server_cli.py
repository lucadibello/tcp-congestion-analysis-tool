#!/usr/bin/env python3
import subprocess
from utils import success, error, check_open, get_fin_timeout
import time


def test_help():
    proc = subprocess.Popen(
        ["./server", "--help"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    stdout, stderr = proc.communicate()

    if proc.returncode != 0:
        error("When called with --help the return code should be 0")
        return

    if len(stderr) != 0:
        error("When called with --help the stderr should be empty")
        return

    if len(stdout) == 0:
        error("When called with --help the stdout should contain the help page")
        return

    success("No issues found with the --help option")


def test_default_port():
    proc = subprocess.Popen(
        ["./server"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    time.sleep(5)
    if proc.poll() is not None:
        time.sleep(get_fin_timeout())
        proc = subprocess.Popen(
            ["./server"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        time.sleep(5)

    if proc.poll() is not None:
        error("Failed to start server")
        return

    if not check_open(5000):
        error("By default the server should listen on port 5000/tcp")
        proc.terminate()
        return

    proc.terminate()
    success("The server correctly listens on port 5000/tcp by default")


def test_port():
    proc = subprocess.Popen(
        ["./server", "--port", "5001"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    time.sleep(5)
    if proc.poll() is not None:
        time.sleep(get_fin_timeout())
        proc = subprocess.Popen(
            ["./server", "--port", "5001"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        time.sleep(5)

    if proc.poll() is not None:
        error("Failed to start server")
        return

    if not check_open(5001):
        error("The server should listen on the port provided by the --port flag")
        proc.terminate()
        return

    proc.terminate()
    success("The server correctly listens on the port provided by the --port flag")


def test_unknown_flag():
    proc = subprocess.Popen(
        ["./server", "--unknown"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    stdout, stderr = proc.communicate()

    if proc.returncode == 0:
        error(
            "When called with an unknown flag the return code should be different from 0"
        )
        return

    if len(stderr) == 0:
        error(
            "When called with an unknown flag the stderr should contain the help page"
        )
        return

    if len(stdout) != 0:
        error("When called with an unknown flag the stdout should be empty")
        return

    success("No issues found with handling of unknown flags")


def test_unknown_argument():
    proc = subprocess.Popen(
        ["./server", "unknown"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    stdout, stderr = proc.communicate()

    if proc.returncode == 0:
        error(
            "When called with an unknown argument the return code should be different from 0"
        )
        return

    if len(stderr) == 0:
        error(
            "When called with an unknown argument the stderr should contain the help page"
        )
        return

    if len(stdout) != 0:
        error("When called with an unknown argument the stdout should be empty")
        return

    success("No issues found with handling of unknown argument")


def main():
    print("[+] Testing server cli interface")
    test_help()
    test_default_port()
    test_port()
    test_unknown_argument()
    test_unknown_flag()


if __name__ == "__main__":
    main()
