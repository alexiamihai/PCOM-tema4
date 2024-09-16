#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* sockfd, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct HOSTent, getHOSTbyname */
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define REGISTER_CMD 1
#define LOGIN_CMD 2
#define LOGOUT_CMD 3
#define ENTER_LIBRARY_CMD 4
#define GET_BOOKS_CMD 5
#define GET_BOOK_CMD 6
#define ADD_BOOK_CMD 7
#define DELETE_BOOK_CMD 8
#define EXIT_CMD 9
#define UNKNOWN_CMD 0

typedef enum
{
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL
} JSON_FieldType;

typedef struct
{
    char *key;
    void *value;
    JSON_FieldType type;
} JSON_Field;

int get_command_type(char *command)
{
    if (strncmp(command, "register", 8) == 0)
        return REGISTER_CMD;
    if (strncmp(command, "login", 6) == 0)
        return LOGIN_CMD;
    if (strncmp(command, "logout", 6) == 0)
        return LOGOUT_CMD;
    if (strncmp(command, "enter_library", 13) == 0)
        return ENTER_LIBRARY_CMD;
    if (strncmp(command, "get_books", 9) == 0)
        return GET_BOOKS_CMD;
    if (strncmp(command, "get_book", 8) == 0)
        return GET_BOOK_CMD;
    if (strncmp(command, "add_book", 8) == 0)
        return ADD_BOOK_CMD;
    if (strncmp(command, "delete_book", 11) == 0)
        return DELETE_BOOK_CMD;
    if (strncmp(command, "exit", 4) == 0)
        return EXIT_CMD;
    return UNKNOWN_CMD;
}

char *create_json_object(JSON_Field fields[], size_t num_fields)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    for (size_t i = 0; i < num_fields; i++)
    {
        switch (fields[i].type)
        {
        case JSON_STRING:
            json_object_set_string(root_object, fields[i].key, (char *)fields[i].value);
            break;
        case JSON_NUMBER:
            json_object_set_number(root_object, fields[i].key, *(double *)fields[i].value);
            break;
        case JSON_BOOL:
            json_object_set_boolean(root_object, fields[i].key, *(int *)fields[i].value);
            break;
        }
    }

    char *serialized_string = json_serialize_to_string(root_value);
    return serialized_string;
}

bool validate_input(const char *input)
{
    return (strchr(input, ' ') == NULL);
}
bool check_empty(char *str)
{
    return (strcmp(str, "") != 0);
}
bool validate_and_convert_input(const char *input, int *number)
{
    char temp[strlen(input) + 1];
    strcpy(temp, input);
    temp[strcspn(temp, "\n")] = '\0';
    *number = atoi(temp);
    if (*number <= 0)
    {
        printf("ERROR!: Format gresit! Mai incearca!\n");
        return false;
    }

    return true;
}
bool validate_and_convert_double_input(const char *input, double *number)
{
    char temp[strlen(input) + 1];
    strcpy(temp, input);

    temp[strcspn(temp, "\n")] = '\0';
    *number = atoi(temp);
    if (*number <= 0)
    {
        printf("ERROR!: Format gresit! Mai incearca!\n");
        return false;
    }

    return true;
}

void processCookie(char *response, int *logged_in, char *cookies[])
{
    char *cookie = strstr(response, "Set-Cookie: ");
    if (cookie == NULL)
    {
        printf("ERROR! Utilizatorul nu s-a putut loga!\n");
        *logged_in = 0;
    }
    else
    {
        printf("SUCCESS! Utilizatorul s-a logat cu succes!\n");
        *logged_in = 1;
        strtok(cookie, ";");
        cookie += 12;
        cookies[0] = cookie;
    }
}

void processToken(char *response, int *entered_library, char *token)
{
    char *token_response = strstr(response, "token");
    if (token_response == NULL)
    {
        printf("ERROR!: Utilizatorul nu are acces la biblioteca!\n");
        *entered_library = 0;
    }
    else
    {
        *entered_library = 1;
        printf("SUCCESS! Utilizatorul are acces la biblioteca!\n");
        token_response += 8;
        memset(token, 0, BUFLEN);
        strcpy(token, token_response);
        token[strlen(token) - 2] = '\0';
    }
}

int main(int argc, char *argv[])
{
    int logged_in = 0, entered_library = 0;
    int sockfd;

    char *user[1], *addbook[1];
    char command[BUFLEN];
    char *username, *password, *response, *res;

    char route[BUFLEN], book_nr[BUFLEN], token[BUFLEN];
    int book_id;

    char *cookies[1];

    while (1)
    {
        fgets(command, BUFLEN, stdin);
        command[strcspn(command, "\n")] = 0;

        int command_type = get_command_type(command);

        switch (command_type)
        {
        case REGISTER_CMD:
            username = calloc(BUFLEN, sizeof(char));
            password = calloc(BUFLEN, sizeof(char));

            printf("username=");
            fgets(username, BUFLEN, stdin);
            username[strcspn(username, "\n")] = 0;
            int ok_user = validate_input(username);

            printf("password=");
            fgets(password, BUFLEN, stdin);
            password[strcspn(password, "\n")] = 0;
            int ok_pass = validate_input(password);

            if (!ok_user || !ok_pass)
            {
                printf("ERROR! Parola sau user invalid. Te rog, nu folosi spatii!\n");
                free(username);
                free(password);
                continue;
            }

            JSON_Field user_fields_register[] = {
                {"username", username, JSON_STRING},
                {"password", password, JSON_STRING}};

            user[0] = create_json_object(user_fields_register, sizeof(user_fields_register) / sizeof(JSON_Field));

            sockfd = open_connection(HOST, PORT_NUM, AF_INET, SOCK_STREAM, 0);
            response = recv_post_req(sockfd, HOST, REGISTER, user, NULL);
            char *successful = strstr(response, "HTTP/1.1 2");
            close_connection(sockfd);

            if (successful)
            {
                printf("SUCCESS! Utilizator inregistrat cu succes!\n");
            }
            else
            {
                printf("ERROR! Nu am putut crea contul.\n");
            }
            free(username);
            free(password);
            json_free_serialized_string(user[0]);
            break;

        case LOGIN_CMD:
            username = calloc(BUFLEN, sizeof(char));
            password = calloc(BUFLEN, sizeof(char));

            printf("username=");
            fgets(username, BUFLEN, stdin);
            username[strcspn(username, "\n")] = 0;
            int ok_user_l = validate_input(username);

            printf("password=");
            fgets(password, BUFLEN, stdin);
            password[strcspn(password, "\n")] = 0;
            int ok_pass_l = validate_input(password);

            if (!ok_user_l || !ok_pass_l)
            {
                printf("ERROR! Parola sau user invalid. Te rog, nu folosi spatii!\n");
                free(username);
                free(password);
                continue;
            }

            JSON_Field user_field_login[] = {
                {"username", username, JSON_STRING},
                {"password", password, JSON_STRING}};

            user[0] = create_json_object(user_field_login, sizeof(user_field_login) / sizeof(JSON_Field));

            if (logged_in)
            {
                printf("ERROR! Utilizator deja logat.\n");
            }
            else
            {
                sockfd = open_connection(HOST, PORT_NUM, AF_INET, SOCK_STREAM, 0);
                response = recv_post_req(sockfd, HOST, LOGIN, user, NULL);
                close_connection(sockfd);

                processCookie(response, &logged_in, cookies);
            }

            free(username);
            free(password);
            json_free_serialized_string(user[0]);
            break;

        case LOGOUT_CMD:
            if (logged_in == 1)
            {
                logged_in = 0;
                entered_library = 0;

                sockfd = open_connection(HOST, PORT_NUM, AF_INET, SOCK_STREAM, 0);
                recv_get_req(sockfd, HOST, LOGOUT, token, cookies, "get");
                close_connection(sockfd);

                printf("SUCCESS! Utilizatorul s-a delogat cu succes!\n");
            }
            else
            {
                printf("ERROR! Utlizatorul nu e logat!\n");
            }
            break;

        case ENTER_LIBRARY_CMD:
            if (logged_in && !entered_library)
            {
                sockfd = open_connection(HOST, PORT_NUM, AF_INET, SOCK_STREAM, 0);
                response = recv_get_req(sockfd, HOST, ACCESS, token, cookies, "get");
                processToken(response, &entered_library, token);
                close_connection(sockfd);
            }
            else if (!logged_in)
            {
                printf("ERROR: Utilizatorul nu este logat.\n");
            }
            else
            {
                printf("ERROR: Utilizatorul este deja in biblioteca.\n");
            }
            break;

        case GET_BOOKS_CMD:
            if (entered_library == 1)
            {
                sockfd = open_connection(HOST, PORT_NUM, AF_INET, SOCK_STREAM, 0);
                res = recv_get_req(sockfd, HOST, BOOKS, token, cookies, "get");
                close_connection(sockfd);
                char *find_list = strstr(res, "[");
                if (find_list)
                {
                    printf("%s\n", find_list);
                }
                else
                {
                    printf("ERROR: N-am gasit carti.\n");
                }
            }
            else
            {
                printf("ERROR: Utilizatorul nu este in biblioteca.\n");
            }
            break;

        case GET_BOOK_CMD:
            printf("id=");
            fgets(book_nr, BUFLEN, stdin);
            if (!validate_and_convert_input(book_nr, &book_id))
            {
                continue;
            }
            if (entered_library == 1)
            {
                sprintf(route, "%s/%d", BOOKS, book_id);
                sockfd = open_connection(HOST, PORT_NUM, AF_INET, SOCK_STREAM, 0);
                res = recv_get_req(sockfd, HOST, route, token, cookies, "get");
                char *successful404 = strstr(res, "HTTP/1.1 404 Not Found");
                if (successful404)
                {
                    printf("ERROR: Nu am gasit cartea cautata!\n");
                }
                else
                {
                    char *find_list = strstr(res, "{");
                    if (find_list)
                    {
                        printf("%s\n", find_list);
                    }
                    else
                    {
                        printf("ERROR: N-am gasit cartea.\n");
                    }
                }
                close_connection(sockfd);
            }
            else
            {
                printf("ERROR: Utilizatorul nu este in biblioteca.\n");
            }
            break;

        case ADD_BOOK_CMD:
            if (entered_library == 1)
            {
                char titlu[BUFLEN];
                printf("title=");
                fgets(titlu, BUFLEN, stdin);
                titlu[strcspn(titlu, "\n")] = 0;
                int ok_title = check_empty(titlu);

                char author[BUFLEN];
                printf("author=");
                fgets(author, BUFLEN, stdin);
                author[strcspn(author, "\n")] = 0;
                int ok_author = check_empty(author);

                char genre[BUFLEN];
                printf("genre=");
                fgets(genre, BUFLEN, stdin);
                genre[strcspn(genre, "\n")] = 0;
                int ok_genre = check_empty(genre);

                char publisher[BUFLEN];
                printf("publisher=");
                fgets(publisher, BUFLEN, stdin);
                publisher[strcspn(publisher, "\n")] = 0;
                int ok_publisher = check_empty(publisher);

                double pages;
                char page_number[BUFLEN];
                printf("page_count=");
                fgets(page_number, BUFLEN, stdin);
                if (!validate_and_convert_double_input(page_number, &pages))
                {
                    break;
                }

                if (!ok_author || !ok_genre || !ok_title || !ok_publisher)
                {
                    printf("ERROR! Format gresit! Mai incearca.\n");
                    break;
                }

                JSON_Field book_fields[] = {
                    {"title", titlu, JSON_STRING},
                    {"author", author, JSON_STRING},
                    {"genre", genre, JSON_STRING},
                    {"publisher", publisher, JSON_STRING},
                    {"page_count", &pages, JSON_NUMBER}};

                addbook[0] = create_json_object(book_fields, sizeof(book_fields) / sizeof(JSON_Field));

                sockfd = open_connection(HOST, PORT_NUM, AF_INET, SOCK_STREAM, 0);
                response = recv_post_req(sockfd, HOST, BOOKS, addbook, token);
                close_connection(sockfd);

                if (strstr(response, "HTTP/1.1 2"))
                {
                    printf("SUCCESS! Am adaugat cartea cu succes!\n");
                }
                else
                {
                    printf("ERROR: Cartea nu a fost adaugata!\n");
                }
                json_free_serialized_string(addbook[0]);
            }
            else
            {
                printf("ERROR: Utilizatorul nu este in biblioteca.\n");
            }
            break;

        case DELETE_BOOK_CMD:
            printf("id=");
            fgets(book_nr, BUFLEN, stdin);
            validate_and_convert_input(book_nr, &book_id);
            if (entered_library == 1)
            {
                sprintf(route, "%s/%d", BOOKS, book_id);
                sockfd = open_connection(HOST, PORT_NUM, AF_INET, SOCK_STREAM, 0);
                res = recv_get_req(sockfd, HOST, route, token, cookies, "delete");
                char *successfuldelete = strstr(res, "HTTP/1.1 404 Not Found");
                if (successfuldelete)
                {
                    printf("ERROR: Nu am gasit cartea cautata!\n");
                }
                else
                {
                    printf("SUCCESS! Cartea a fost stearsa cu succes!\n");
                }
                close_connection(sockfd);
            }
            else
            {
                printf("ERROR: Utilizatorul nu este in biblioteca.\n");
            }
            break;

        case EXIT_CMD:
            return 0;

        case UNKNOWN_CMD:
            printf("Comanda necunoscuta: %s\n", command);
            break;

        default:
            printf("Comanda invalida!\n");
            break;
        }
    }
    return 0;
}
