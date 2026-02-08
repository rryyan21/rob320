#include "util.h"
#include "messages.h"

ChattersMessage chatters;

int ctrl_c_pressed = 0;
void sigint_handler(int signum) {
    if (signum == SIGINT) {
        ctrl_c_pressed = 1;
    }
}

// Called when a user sends a register message
void handle_register(UserMessage* msg) {
    printf("Registering user %s\n", msg->user.name);
    for (int i = 0; i < 32; i++) {
        if (chatters.users[i].port == 0) {
            chatters.users[i] = msg->user;
            break;
        }
    }
}

// Called when a user sends a deregister message
void handle_deregister(UserMessage* msg) {
    printf("Deregistering user %s\n", msg->user.name);
    for (int i = 0; i < 32; i++) {
        if (strcmp(chatters.users[i].name, msg->user.name) == 0) {
            memset(&chatters.users[i], 0, sizeof(User));
            break;
        }
    }
}

// Called when a user sends a request message
void handle_request(int client_fd) {
    uint8_t* chatters_data = encode_chatters_message(&chatters);
    int status = send_until_success(client_fd, chatters_data, sizeof(ChattersMessage));
    if (status < 0) {
        perror("send");
        return;
    }
}

int main(int argc, char* argv[]) {
    char* public_ip = get_public_ip();
    if (public_ip == NULL) {
        public_ip = strdup("127.0.0.1");
    }

    // Register signal handler
    signal(SIGINT, sigint_handler);

    int status;
    int server_fd;
    struct sockaddr_in address;

    // Create a socket file descriptor for the server
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // Set the socket to non-blocking mode
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // Set the socket to reuse the address
    // This allows the server to bind to the same address after a crash
    int opt = 1;
    status = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (status < 0) {
        perror("setsockopt");
        return 1;
    }

    // Initialize the sockaddr_in struct for the server
    address.sin_family = AF_INET; // AF_INET specifies IPv4 protocol family
    address.sin_addr.s_addr = inet_addr(public_ip); // We will use our public IP address (the first IP address that is not the loopback address)
    address.sin_port = htons(DISCOVERY_PORT); // htons() converts the port number to network byte order

    // Bind the server to the specified address
    // Notice that we cast the sockaddr_in struct to a sockaddr struct
    // This is a rudimentary form of polymorphism in C
    // sockaddr_in stands for "socket address internet"
    // There are other types of socket addresses, such as sockaddr_un for Unix domain sockets
    // or sockaddr_in6 for IPv6 addresses
    status = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    if (status < 0) {
        perror("bind");
        return 1;
    }

    // Listen for up to 32 incoming connections on the socket
    // This means that the server can handle up to 32 clients at once
    status = listen(server_fd, 32);
    if (status < 0) {
        perror("listen");
        return 1;
    }

    while (!ctrl_c_pressed) {
        // Accept a connection from a client
        int client_fd;
        status = accept_until_success(server_fd, &client_fd);
        if (status != 0) {
            return 1;
        }

        // Receive a UserMessage from the client
        uint8_t buffer[sizeof(UserMessage)] = {0};
        status = recv_until_success(client_fd, buffer, sizeof(UserMessage));
        if (status != 0) {
            return 1;
        }

        // Decode the buffer into a UserMessage
        UserMessage* msg = decode_user_message(buffer, sizeof(UserMessage));
        if (msg == NULL) {
            fprintf(stderr, "Failed to decode chat message\n");
            return 1;
        }

        // Handle the message based on the opcode
        switch (msg->opcode) {
            case 0: 
                handle_request(client_fd);
                break;
            case 1:
                handle_register(msg);
                break;
            case 2:
                handle_deregister(msg);
                break;
            default:
                fprintf(stderr, "Invalid opcode\n");
                break;
        }

        // Close the connection to the client
        close(client_fd);
    }

    // Close the server socket
    close(server_fd);

    // Free the public_ip string
    free(public_ip);
}