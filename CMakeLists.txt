cmake_minimum_required(VERSION 3.22)
project(c_dns_proxy C)

set(CMAKE_C_STANDARD 23)
#find_package(Libevent REQUIRED)


add_executable(c_dns_proxy main.c lib/SPCDNS/src/codec.o)
target_link_libraries(c_dns_proxy #libevent::core
        m)