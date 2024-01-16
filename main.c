#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lib/toml-c-master/toml-c.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> // for udp

#define MAX_CONNECTIONS 20000

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
    char **black_list = CALLOC(l, sizeof(char *));
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

    // todo приймати dns запити на стандартному порту
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < -1) {
        fprintf(stderr, "problem while socket creating");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53000);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "problem while socket binding");
        exit(1);
    }

//    if (listen(sockfd, 10) < 0) {
//        printf("Error while listening\n");
//        return -1;
//    }

    for (int i = 0; i < l; i++) {
        printf("%s\n", black_list[i]);
    }
    printf("boss: %s\n", boss_server.u.s);

    // test udp
    ssize_t n;
    char *buf;
    struct sockaddr_in clientaddr;
    int clientlen = sizeof(clientaddr);
#define BUFSIZE 1024
    struct hostent *hostp;
    char *hostaddrp;
    //while (1)
    {

        buf = malloc(BUFSIZE); // todo

        n = recvfrom(sockfd, buf, BUFSIZE, 0,
                     (struct sockaddr *) &clientaddr, &clientlen);
        if (n < 0)
            printf("ERROR in recvfrom");

        /*
         * gethostbyaddr: determine who sent the datagram
         */
        hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr,
                              sizeof(clientaddr.sin_addr.s_addr),
                              AF_INET);
        if (hostp == NULL)
            printf("ERROR on gethostbyaddr");
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
            printf("ERROR on inet_ntoa\n");
        /*printf("server received %d bytes\n", n); */

        /*
         * sendto: echo the input back to the client
         */
        n = sendto(sockfd, buf, n, 0,
                   (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0)
            printf("ERROR in sendto");
    }
// ~test udp




    // обробити, якщо в чорному списку - надіслати not resolved або адресу в локальній мережі, якщо ні - надіслати те що вищий днс сервер надасть у відповідь на тей же запит
    for (int i = 0; i < l; i++) {
        free(black_list[i]);
    }
    toml_free(tbl);

    return 0;
}
