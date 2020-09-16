# Distributed-Terminal

This project is to learn the use of sockets/RPC and thus to develop a terminal which can issue commands to other nodes in the Network(with the help of IP Address and port number).
It also supports pipelining of commands on different nodes i.e. the output of command from node x can be pipelined to command issued to node y directly from terminal.
This is a very basic implementation of the same which supports the following commands: ``` ls, date, mkdir, rmdir, rm, mv, cp, man, wc, uniq, sort. ```

## Compiling
In the source code directory(this director) run:
```
make
```

## Running
In the terminal run the server(s) with specified port and then run the client with with its IP address and port.<br>
In client there will be two options available: one to add host (requires IP Address and port number of Host in the Network) and other to issue command to different hosts as:
```
hostname1 > cmd1 [|| hostname2 > cmd2]*
```
|| is used for pipelining
