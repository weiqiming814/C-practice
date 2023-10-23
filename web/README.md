# Web server & client

A simple web server and a client to crawl website.

## Requirements

This project is only tested on Linux, although this may work on OSx or WSL. It requires a working version of GCC, GDB and make in your path.

## Usage
### Get Web program
Clone the PC project and the related sub modules:
```
git clone --recursive weiqiming/web at master · weiqiming814/weiqiming (github.com)
```
## Use
The following steps can be used with Make on a Unix-like system. This may also work on other OSes but has not been tested.

1. Ensure Make is installed, i.e. the `make` command works on the terminal.
2. Type `make`.
3. The binary will be in runing, and can be run by typing that command.
4. Type `./server` to running the web server, and you can connect it by wetsite, curl and client in this work. It will listen on the port in `myhttpd_conf`, and print the infomation in `index_html`.
5. The website of server is `http://localhost:8080.
6. Type `./client url` can runing the web client, and crawl webwite "url".
