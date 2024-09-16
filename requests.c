#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"


char* build_request_message(const char *method, const char *url, const char *host, const char *content_type,
                            char **body_data, int body_data_fields_count, char **cookies, int cookies_count,
                            const char *jwt_token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    sprintf(line, "%s %s HTTP/1.1", method, url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (jwt_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", jwt_token);
        compute_message(message, line);
    }
    if (content_type != NULL) {
        sprintf(line, "Content-Type: %s", content_type);
        compute_message(message, line);
    }
    if (body_data != NULL) {
        int len = 0;
        for (int i = 0; i < body_data_fields_count; i++) {
            strcat(body_data_buffer, body_data[i]);
            len += strlen(body_data[i]);
        }
        sprintf(line, "Content-Length: %d", len);
        compute_message(message, line);
    }
    if (cookies != NULL) {
        sprintf(line, "Cookie: %s", cookies[0]);
        for (int i = 1; i < cookies_count; i++) {
            sprintf(line + strlen(line), "; %s", cookies[i]);
        }
        compute_message(message, line);
    }

    compute_message(message, "");

    if (body_data != NULL) {
        strcat(message, body_data_buffer);
    }

    free(line);
    free(body_data_buffer);
    return message;
}

char *send_request_and_receive_response(int socket, char *request_message) {
    send_to_server(socket, request_message);
    return receive_from_server(socket);
}

char *recv_post_req(int socket, char host[16], char *command, char *user[1], char *token) {
    char *request_message = build_request_message("POST", command, host, "application/json", user, 1, NULL, 0, token);
    char *response = send_request_and_receive_response(socket, request_message);
    free(request_message);
    return response;
}

char *recv_get_req(int socket, char host[16], char *command, char *token, char *cookies[1], char *get_delete) {
    char *request_message = build_request_message(
        strcmp(get_delete, "get") == 0 ? "GET" : "DELETE",
        command,
        host,
        NULL,
        NULL,
        0,
        cookies,
        cookies != NULL ? 1 : 0,
        token
    );
    char *response = send_request_and_receive_response(socket, request_message);
    free(request_message);
    return response;
}
