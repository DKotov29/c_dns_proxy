
# About
This is a DNS proxy server, made with pain and a bit of c.

Main idea: accept connections, read queries, if domain name in blacklist queried - return refused, otherwise ask top server (on code and config named as boss server) and return what it responds.




# Build
libevent-dev needed for building, also this code is not supported on windows. After cloning , in cloned directory command "gcc -Wall main.c -L. -levent -lm -levent_core -lpthread -levent_pthreads -O3 -o oh_no" can be used to create executable file and command "./oh_no" to execute that file.

# Use

With start of program you can query this dns server, for example by command "dig @127.0.0.1 -p 53 google.com"

When the program starts, it will also recognize config, such as top server and blacklist, which can be changed if it happens as in the example.

# Used libraries 

https://github.com/arp242/toml-c/

https://github.com/spc476/SPCDNS

libevent

https://github.com/troydhanson/uthash/

# Some proofs

Blacklisted:
Used dig to ask something from server 

![](https://i.imgur.com/9HDHAY3.png)

To top server forwarded:

![](https://i.imgur.com/Qwz13kl.png)