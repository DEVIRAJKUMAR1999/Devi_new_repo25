#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define PORT_NUM 9090

struct logins {
    char time[20];
    char send[10], receive[10];
    char clientip[20];
    char User[10];
    char Password[10];
    struct logins *link;
};

struct logid {
    char User[10] = "devi";
    char Password[10] = "pass";
};

void math_calculation(int num1, int num2, char *str, char ch);
void create_node(struct logins **head, char *str_send, char *str_receive, char *client_address);
int check_data(const char *str);
int check_symbol(char ch);
long int own_atoi(char *str);

int authentication(const char *user, const char *password) {
    struct logid login;
    if (strcmp(login.User, user) == 0 && strcmp(login.Password, password) == 0) {
        return 1;
    } else {
        return 0;
    }
}

void freenode(struct logins *head) {
    struct logins *travel = head;
    while (travel != NULL) {
        struct logins *temp = travel;
        travel = travel->link;
        free(temp);
    }
}

int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    char receive_buffer[100] = {0}, send_buffer[100] = {0}, receive_buffer_ip[20] = {0};
    struct sockaddr_in server_address, client_address;

    struct logins *head = NULL; // Initialize head to NULL

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT_NUM);
    server_len = sizeof(server_address);

    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    if (listen(server_sockfd, 5) < 0) {
        perror("Listen failed");
        close(server_sockfd);
        exit(1);
    }

    printf("Server waiting...\n");

    while (1) {
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
        if (client_sockfd < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected...\n");

        // Receive username and password
        recv(client_sockfd, receive_buffer, sizeof(receive_buffer), 0);
        char username[10], password[10];
        strncpy(username, receive_buffer, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';

        recv(client_sockfd, receive_buffer, sizeof(receive_buffer), 0);
        strncpy(password, receive_buffer, sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
        if (authentication(username, password)) {
            printf("Login success...\n");
        } else {
            printf("Login failed...\n");
            close(client_sockfd);
            continue;
        }
	sleep(1);
        // Receive arithmetic expression
        recv(client_sockfd, receive_buffer, sizeof(receive_buffer), 0);
        printf("Received expression: %s\n", receive_buffer);
	sleep(1);
        // Receive client IP address
        recv(client_sockfd, receive_buffer_ip, sizeof(receive_buffer_ip), 0);
        printf("Client IP address: %s\n", receive_buffer_ip);

        if (!check_data(receive_buffer)) {
            printf("Invalid data received...\n");
            send(client_sockfd, "re-enter", 10, 0);
            close(client_sockfd);
            continue;
        }

        // Extract numbers and operator
        int num1, num2;
        char operator_char;
        char temp_buffer[100];
        strcpy(temp_buffer, receive_buffer);

        int i = 0;
        while (temp_buffer[i] != '\0') {
            if (check_symbol(temp_buffer[i])) {
                operator_char = temp_buffer[i];
                break;
            }
            i++;
        }
	send(client_sockfd,"from server ",10,0);
	sleep(1);
        num1 = atoi(temp_buffer);
        num2 = atoi(temp_buffer + i + 1);
math_calculation(num1, num2, send_buffer, operator_char);

        if (strcmp(send_buffer, "error") == 0) {
            send(client_sockfd, "re-enter", 10, 0);
            printf("Error in math calculation...\n");
            close(client_sockfd);
            continue;
        } else {
            send(client_sockfd, send_buffer, 10, 0);
            sleep(1);
            printf("Sent result: %s\n", send_buffer);
        }

        // Log the transaction
        create_node(&head, send_buffer, receive_buffer, receive_buffer_ip);

        printf("Closing client connection...\n");
        close(client_sockfd);
    }

    // Free the linked list
    freenode(head);
    close(server_sockfd);
    return 0;
}

void math_calculation(int num1, int num2, char *str, char ch) {
    long int num = 1;
    switch (ch) {
        case '+': snprintf(str, 20, "%d", num1 + num2); break;
        case '-': snprintf(str, 20, "%d", num1 - num2); break;
        case '*': snprintf(str, 20, "%d", num1 * num2); break;
        case '/':
            if (num2 == 0) {
                snprintf(str, 20, "error");
            } else {
                snprintf(str, 20, "%.2f", (float)num1 / num2);
            }
            break;
        case '^':
            while (num2 > 0) {
                num2--;
                num *= num1;
            }
            snprintf(str, 20, "%ld", num);
            break;
        case '%':
            if (num2 == 0) {
                snprintf(str, 20, "error");
            } else {
                snprintf(str, 20, "%d", num1 % num2);
            }
            break;
        default: snprintf(str, 20, "error"); break;
    }
}

void create_node(struct logins **head, char *str_send, char *str_receive, char *client_address) {
    struct logins *newnode = (struct logins *)malloc(sizeof(struct logins));
    if (!newnode) {
        perror("Memory allocation failed");
        return;
    }

    newnode->link = NULL;
    strncpy(newnode->send, str_send, sizeof(newnode->send) - 1);
    strncpy(newnode->receive, str_receive, sizeof(newnode->receive) - 1);
    strncpy(newnode->clientip, client_address, sizeof(newnode->clientip) - 1);

    time_t ticks = time(NULL);
    strncpy(newnode->time, ctime(&ticks), sizeof(newnode->time) - 1);

    if (*head == NULL) {
        *head = newnode;
    } else {
        struct logins *traversal = *head;
        while (traversal->link != NULL) {
            traversal = traversal->link;
        }
        traversal->link = newnode;
    }
}

int check_data(const char *str) {
    int flag = 0;
    while (*str != '\0') {
        if ((*str >= '0' && *str <= '9') || *str == ' ') {
            str++;
            continue;
        } else if (check_symbol(*str)) {
            flag++;
            str++;
            continue;
        } else {
            return 0;
        }
    }
    return (flag == 1); // Ensure only one operator is present
}

int check_symbol(char ch) {
    switch (ch) {
        case '+': case '-': case '*': case '/': case '^': case '%': return 1;
        default: return 0;
    }
}
