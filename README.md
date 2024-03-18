# TCP Congestion Control Analysis Tool

This small tool can be used to send a certain amount of random bytes to a server, given its IP address (or domain name, resolved via DNS) and port, using a specific TCP Congestion Control algorithm. This tool is useful in pair with some network analysis tools, such as Wireshark, to analyze the behavior of different TCP Congestion Control algorithms in different network conditions.

The tool is written in C and uses exclusively standard C libraries, so it can be compiled and run on any Linux machine.

## Getting Started

To compile the tool, run the following command:

```bash
make
```

This will generate the `client` executable file, which can be used via the command line.


> Unfortunately this code cannot be compiled under MacOS due to the impossibility of changing the TCP Congestion Control algorithm in Socket API. For this reason, a working DevContainer is provided in the repository, which can be used to run the code in a Linux environment.

## Usage

To display the usage of the tool, run the following command:

```bash
./client -h 
```

The output will be:

```bash
Usage: ./client [options] <server>

Options:
  -p, --port <port>          The port the server is listening on (default: 5000)
  -n, --size <size>          The number of pseudo-random bytes to send to the server (default: 1000)
  -c, --congestion <algo>    Congestion control algorithm to use (default: cubic)
      Available algorithms: reno, cubic, vegas
  -h, --help                 Display this help and exit
```

### Examples

This command will send 1000 pseudo-random bytes to the server at `10.20.1.20:5000` using the `reno` TCP Congestion Control algorithm:

```bash
./client -p 5000 -n 1000 -c reno 10.20.1.20
```

This command will send 1000 pseudo-random bytes to the server with DNS name `server.local` using the `cubic` TCP Congestion Control algorithm:

```bash
./client -n 1000 -c cubic server.local
```
