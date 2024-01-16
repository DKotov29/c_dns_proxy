#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lib/toml-c-master/toml-c.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> // for udp

// dns decode/encode
#include "./lib/SPCDNS/src/dns.h"
#include "./lib/SPCDNS/src/codec.c"

// libevent
#include "./lib/libevent/include/event2/event.h"
#include "./lib/libevent/include/event2/buffer.h"
#include "./lib/libevent/include/event2/bufferevent.h"

struct SharedContext {
};

char **black_list;
size_t blacklist_len;

#define BUFSIZE 1024

bool naive_is_blacklisted(char *checking, char **blacklist, size_t blacklist_len) {
    for (size_t i = 0; i < blacklist_len; i++) {
        char *found;
        char *b_i = blacklist[i];
        if ((found = strstr(checking, b_i)) != NULL) {
            if ((size_t) (found - checking) + strlen(b_i) ==
                strlen(checking)) { // because if google.com blacklisted, we expect *.google.com blacklisted and not gogle.com*
                printf("yeah\n");
                return true;
            }
        }
    }
    return false;
}

struct dns_query_t *buildResponse(int id, bool query, enum dns_rcode rcode) {
    dns_query_t *pQuery = malloc(sizeof(dns_query_t));
//    for(size_t i = 0; i<sizeof(dns_query_t); i++){
//        *((char*) pQuery) = 0;
//    }
    if (!pQuery)
        return NULL;
    pQuery->id = id;
    // 0 = request, 1 = response
    pQuery->query = query;

    pQuery->rcode = rcode;

    pQuery->questions = NULL;
    pQuery->answers = NULL;
    pQuery->nameservers = NULL;
    pQuery->additional = NULL;
    return pQuery;
}

struct dns_query_t *refused;

/* This function is called whenever there's incoming data to handle. */
static void acceptDatagram(evutil_socket_t fd, short events, void *context) {
    char buf[BUFSIZE];
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    ssize_t n = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0) {
        perror("ERROR in recvfrom");
        return;
    }

    char hostaddrp[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(clientaddr.sin_addr), hostaddrp, INET_ADDRSTRLEN) == NULL) {
        perror("ERROR on inet_ntop");
        return;
    }

    // decode
    dns_packet_t reply[DNS_BUFFER_UDP];
    dns_decoded_t decoded[DNS_DECODEBUF_4K];
    dns_query_t *result;
    size_t replysize = n;
    size_t decodesize;
    memcpy(reply, buf, n);

    /* reply contains a DNS packet, and replysize is set */
    decodesize = sizeof(decoded);

    enum dns_rcode rc = dns_decode(decoded, &decodesize, reply, replysize);

    result = (dns_query_t *) decoded;
    printf("id: %i\n", result->id);
    printf("A: %s\n", result->questions->name);

    if (result->questions) {
        // name for blacklist: result->questions->name
        if (naive_is_blacklisted(result->questions->name, black_list, blacklist_len)) {
            // if (strcmp(result->questions->name, "www.google.com.")) {
            refused->id = result->id;
            dns_packet_t buf2[1024];
            size_t len = sizeof(buf2);
            dns_encode(buf2, &len, refused);
            n = sendto(fd, buf2, len, 0, (struct sockaddr *)&clientaddr, clientlen);
        }
    }
    // back
//    dns_packet_t a;
//    dns_encode(&a, );
    n = sendto(fd, buf, n, 0, (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) {
        perror("ERROR in sendto");
    }
}


// static void do_read(evutil_socket_t fd, short events, void * context) {}
// static void do_write(evutil_socket_t fd, short events, void * context) {}

int main() {
    FILE *fp = fopen("config.toml", "r");
    if (!fp) {
        fprintf(stderr, "cannot open config.toml\n");
        exit(1);
    }
    char errbuf[200];
    toml_table_t *tbl = toml_parse_file(fp, errbuf, sizeof(errbuf));
    if (!tbl) {
        fprintf(stderr, "parsing toml file error: %s\n", errbuf);
        exit(1);
    }
    fclose(fp);

    toml_value_t boss_server = toml_table_string(tbl, "boss-server");
    toml_array_t *arr = toml_table_array(tbl, "black-list");
    int l = toml_array_len(arr);
    blacklist_len = l;
    black_list = CALLOC(l, sizeof(char *));
    {
        bool string_error = false;
        for (int i = 0; i < l; i++) {
            toml_value_t stringg = toml_array_string(arr, i);
            black_list[i] = stringg.u.s;
            string_error |= !stringg.ok;
        }
        if (!boss_server.ok || string_error) {
            fprintf(stderr,
                    "problem while parsing toml file, not found \"boss-server\" string or \"black-list\" array of strings");
            exit(1);
        }
    }

    refused = buildResponse(0, false, RCODE_REFUSED);

    struct event_base *eb = event_base_new();
    if (!eb) {
        exit(1);
    }
    struct SharedContext *sharedContext = malloc(sizeof(struct SharedContext));

    // register udp socket, bind
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < -1) {
        fprintf(stderr, "problem while socket creating");
        exit(1);
    }
    evutil_make_socket_nonblocking(sockfd);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53000);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "problem while socket binding");
        exit(1);
    }
    struct event *ev = event_new(eb, sockfd, EV_READ | EV_PERSIST, acceptDatagram, sharedContext);
    event_add(ev, NULL);;

    event_base_dispatch(eb);


    for (int i = 0; i < l; i++) {
        free(black_list[i]);
    }
    toml_free(tbl);

    return 0;
}