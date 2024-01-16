
# About
This is a DNS proxy server, made with pain and a bit of c.

Main idea: accept connections, read queries, if domain name in blacklist queried - return ip in local network, otherwise ask top server (on code and config names as boss server) and return what it responds.
Also, it must work good with 10k requests per seconds.

# Build

# Use

# Used libraries 

https://github.com/arp242/toml-c/