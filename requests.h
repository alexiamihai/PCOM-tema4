#ifndef _REQUESTS_
#define _REQUESTS_

char *build_request_message(const char *method, const char *url, const char *host, const char *content_type,
                            char **body_data, int body_data_fields_count, char **cookies, int cookies_count,
                            const char *jwt_token);

char *send_request_and_receive_response(int socket, char *request_message);

char *recv_post_req(int socket, char host[16], char *command, char *user[1], char *token);

char *recv_get_req(int socket, char host[16], char *command, char *token, char *cookies[1], char *get_delete);

#endif
