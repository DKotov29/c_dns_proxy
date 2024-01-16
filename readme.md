
# About
This is a DNS proxy server, made with pain and a bit of c.

Main idea: accept connections, read queries, if domain name in blacklist queried - return refused, otherwise ask top server (on code and config named as boss server) and return what it responds.
Also, it must work good with 10k requests per seconds.



# Build

# Use

# Used libraries 

https://github.com/arp242/toml-c/
https://github.com/spc476/SPCDNS
libevent

# Some proofs


Blacklisted: Used dig to ask something from server![](https://i.imgur.com/9HDHAY3.png)