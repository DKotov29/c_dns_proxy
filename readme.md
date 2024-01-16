
# About
This is a DNS proxy server, made with pain and a bit of c.

Main idea: accept connections, read queries, if domain name in blacklist queried - return refused, otherwise ask top server (on code and config named as boss server) and return what it responds.
Also, it must work good with 10k requests per seconds.



# Build
libevent-dev needed for building, also this code is not supported on windows. After cloning , in cloned directory command "gcc -Wall main.c -L. -levent -lm -levent_core -o oh_no" can be used to create executable file and command "./oh_no" to execute that file.

# Use

With start of program you can query this dns server, for example by command "dig @127.0.0.1 -p 53 google.com"

# Used libraries 

https://github.com/arp242/toml-c/

https://github.com/spc476/SPCDNS

libevent

https://github.com/troydhanson/uthash/

# Some proofs

Blacklisted: Used dig to ask something from server![](https://i.imgur.com/9HDHAY3.png)
To top server forwarded: ![](https://i.imgur.com/Qwz13kl.png)