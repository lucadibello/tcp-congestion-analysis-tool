#!/usr/bin/env python3


import subprocess
from utils import success, error


def test_help():
    proc = subprocess.Popen(
        ["./client", "--help"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
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


def test_unknown_flag():
    proc = subprocess.Popen(
        ["./client", "--unknown"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
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


def test_no_address():
    proc = subprocess.Popen(
        ["./client"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    stdout, stderr = proc.communicate()

    if proc.returncode == 0:
        error(
            "When called with no server address the return code should be different from 0"
        )
        return

    if len(stderr) == 0:
        error(
            "When called with no server address the stderr should contain the help page"
        )
        return

    if len(stdout) != 0:
        error("When called with no server address the stdout should be empty")
        return

    success("No issues found with handling of no server address provided")


def test_unknown_flag():
    proc = subprocess.Popen(
        ["./client", "--unknown"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
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


def test_unknown_congestion():
    proc = subprocess.Popen(
        ["./client", "-c", "notexisting"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = proc.communicate()

    if proc.returncode == 0:
        error(
            "When called with an invalid congestion control algorithm the return code should be different from 0"
        )
        return

    if len(stderr) == 0:
        error(
            "When called with an invalid congestion control algorithm the stderr should contain some error"
        )
        return

    if len(stdout) != 0:
        error(
            "When called with an invalid congestion control algorithm the stdout should be empty"
        )
        return

    success("No issues found with handling of invalid congestion control algorithms")


def main():
    print("[+] Testing basic client cli interface")
    test_help()
    test_no_address()
    test_unknown_flag()
    test_unknown_congestion()


if __name__ == "__main__":
    main()
