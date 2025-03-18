#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 20
#define PORT_NUM 9090

/* The client should log all sent requests and received responses to a log file.
   The log should include a timestamp, the type of operation, and the result. */

struct logins {
    char time[20];
    char sent[10], receive[10];
    char op;
};

int check_data(const char *str);
int check_symbol(char ch);

int main() {
    int sock_fd, len;
    struct sockaddr_in address;
    char DATA[SIZE] = {0};

    // Create a socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost address
    address.sin_port = htons(PORT_NUM); // Port number
    len = sizeof(address);

    // Connect to the server
    if (connect(sock_fd, (struct sockaddr *)&address, len) < 0) {
        perror("Failed to connect to the server");
        close(sock_fd);
        exit(1);
    }

    printf("Connected to the server...\n");

    // Input arithmetic expression
    printf("Enter input in this format (e.g., 1+2 or 5-2): ");
    fgets(DATA, SIZE, stdin);
    //printf("%s",DATA);
    DATA[strcspn(DATA, "\n")] = '\0'; // Remove newline character

    if (!check_data(DATA)) {
        printf("Invalid expression. Please check your input.\n");
        close(sock_fd);
        exit(1);
    }

    // Input username and password
    char user[10] = {0};
    char pass[10] = {0};

    printf("Enter username: ");
    scanf("%9s", user); // Limit input to avoid buffer overflow
    if (send(sock_fd, user, sizeof(user), 0) < 0) {
        perror("Failed to send username");
        close(sock_fd);
        exit(1);
    }
	sleep(0.5);
    printf("Enter password: ");
    scanf("%9s", pass); // Limit input to avoid buffer overflow
    if (send(sock_fd, pass, sizeof(pass), 0) < 0) {
        perror("Failed to send password");
        close(sock_fd);
        exit(1);
    }
    sleep(1);

    // Send arithmetic expression
    if (send(sock_fd, DATA, sizeof(DATA), 0) < 0) {
        perror("Failed to send expression");
        close(sock_fd);
        exit(1);
    }
    printf("Expression sent...\n");
	sleep(1);
    // Send client IP address
    char IPaddress[20] = {0};
    strcpy(IPaddress, inet_ntoa(address.sin_addr));
    if (send(sock_fd, IPaddress, sizeof(IPaddress), 0) < 0) {
        perror("Failed to send IP address");
        close(sock_fd);
        exit(1);
    }
    sleep(1);
    printf("IP address sent...\n");

    // Receive result from the server
    char buffer[100] = {0};
    if (recv(sock_fd, buffer, sizeof(buffer), 0) < 0) {
        perror("Failed to receive result");
        close(sock_fd);
        exit(1);
    }

    if (strcmp(buffer, "re-enter") == 0) {
        printf("Invalid expression. Please try again.\n");
        close(sock_fd);
        exit(1);
    }

    printf("Result: %s\n", buffer);

    // Log the transaction
    const char *dummy = DATA;
    while (*dummy != '\0') {
        if (check_symbol(*dummy)) {
            break;
        }
        dummy++;
    }

    struct logins *loginsnode = (struct logins *)malloc(sizeof(struct logins));
    if (!loginsnode) {
        perror("Memory allocation failed");
        close(sock_fd);
        exit(1);
    }

    time_t ticks = time(NULL);
    strncpy(loginsnode->time, ctime(&ticks), sizeof(loginsnode->time) - 1);
    loginsnode->time[sizeof(loginsnode->time) - 1] = '\0'; // Ensure null termination

    strncpy(loginsnode->sent, DATA, sizeof(loginsnode->sent) - 1);
    loginsnode->sent[sizeof(loginsnode->sent) - 1] = '\0'; // Ensure null termination

    strncpy(loginsnode->receive, buffer, sizeof(loginsnode->receive) - 1);
    loginsnode->receive[sizeof(loginsnode->receive) - 1] = '\0'; // Ensure null termination

    loginsnode->op = *dummy;
    
    
    printf("Transaction logged:\n");
    printf("Time: %s\n", loginsnode->time);
    printf("Sent: %s\n", loginsnode->sent);
    printf("Received: %s\n", loginsnode->receive);
    printf("Operator: %c\n", loginsnode->op);

    // Free allocated memory
    free(loginsnode);

    // Close the socket
    close(sock_fd);
    printf("Connection closed.\n");

    return 0;
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
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '%':
            return 1;
        default:
            return 0;
    }
}
