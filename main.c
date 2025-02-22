#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#define MAX_PEERS 1000
#define BUFFER_SIZE 4096   

typedef struct{
    char ip[INET_ADDRSTRLEN];
    int port;
} Peer;

Peer peer_list[MAX_PEERS];
int peer_count = 0;
char team_name[50];

SOCKET server_socket;
HANDLE thread;
volatile int running = 1;

int inet_pton_win(int af, const char *src, void *dst){
    struct sockaddr_in addr;
    int addr_size = sizeof(addr);
    if (WSAStringToAddressA((char *)src, af, NULL, (struct sockaddr *)&addr, &addr_size) == 0){
        *(struct in_addr *)dst = addr.sin_addr;
        return 1;
    }
    return 0;
}


void remove_peer(const char *ip, int port){
    for (int i = 0; i < peer_count; i++){
        if (strcmp(peer_list[i].ip, ip) == 0 && peer_list[i].port == port){
            for (int j = i; j < peer_count - 1; j++){
                peer_list[j] = peer_list[j + 1];
            }
            peer_count--;
            break;
        }
    }
}

void add_peer(const char *ip, int port){
    for (int i = 0; i < peer_count; i++){
        if (strcmp(peer_list[i].ip, ip) == 0 && peer_list[i].port == port){
            return;
        }
    }
    if (peer_count < MAX_PEERS){
        strcpy(peer_list[peer_count].ip, ip);
        peer_list[peer_count].port = port;
        peer_count++;
    }
}

void query_peers(){
    printf("\nConnected Peers:\n");
    if (peer_count == 0){
        printf("No connected peers.\n");
        return;
    }
    for (int i = 0; i < peer_count; i++){
        printf("%s:%d\n", peer_list[i].ip, peer_list[i].port);
    }
}

void inet_ntop_win(int af, const void *src, char *dst, size_t size){
    struct sockaddr_in addr;
    addr.sin_family = af;
    addr.sin_addr = *(struct in_addr *)src;
    
    DWORD dst_size = size;
    WSAAddressToStringA((struct sockaddr *)&addr, sizeof(addr), NULL, dst, &dst_size);
}

DWORD WINAPI receive_messages(LPVOID arg){
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    fd_set readfds;
    struct timeval timeout;

    while (running){
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int sel_result = select(0, &readfds, NULL, NULL, &timeout);
        if (sel_result > 0){
            if (FD_ISSET(server_socket, &readfds)){
                SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
                if (client_socket == INVALID_SOCKET){
                    if (!running) break;
                    printf("Accept failed: %d\n", WSAGetLastError());
                    continue;
                }

                memset(buffer, 0, BUFFER_SIZE);
                int recv_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
                if (recv_bytes <= 0){
                    closesocket(client_socket);
                    continue;
                }

                char client_ip[INET_ADDRSTRLEN];
                inet_ntop_win(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

                char *newline = strchr(buffer, '\n');
                if (newline != NULL){
                    int header_len = newline - buffer + 1;
                    char header_line[BUFFER_SIZE];
                    strncpy(header_line, buffer, header_len);
                    header_line[header_len] = '\0';
                    
                    if (strstr(header_line, "FILE") != NULL){
                        int sender_ephemeral_port, sender_listening_port;
                        char sender_team[50];
                        char filename[256];
                        long filesize;
                        int header_parsed = sscanf(header_line, "%d %d %s FILE %s %ld", 
                                                   &sender_ephemeral_port, &sender_listening_port, 
                                                   sender_team, filename, &filesize);
                        if (header_parsed != 5){
                            printf("Invalid file header format received\n");
                            closesocket(client_socket);
                            continue;
                        }

                        FILE *fp = fopen(filename, "wb");
                        if (fp == NULL){
                            printf("Failed to open file for writing: %s\n", filename);
                            closesocket(client_socket);
                            continue;
                        }
                        
                        int file_content_in_buffer = recv_bytes - header_len;
                        if (file_content_in_buffer > 0){
                            fwrite(buffer + header_len, 1, file_content_in_buffer, fp);
                        }
                        long total_received = file_content_in_buffer;
                        while (total_received < filesize){
                            int r = recv(client_socket, buffer, BUFFER_SIZE, 0);
                            if (r <= 0) break;
                            fwrite(buffer, 1, r, fp);
                            total_received += r;
                        }
                        fclose(fp);
                        
                        add_peer(client_ip, sender_listening_port);
                        
                        printf("\nReceived file '%s' (%ld bytes) from %s:%d %s\n", 
                               filename, filesize, client_ip, sender_listening_port, sender_team);
                    }
                    else{
                        int sender_ephemeral_port, sender_listening_port;
                        char sender_team[50];
                        char received_message[BUFFER_SIZE];
                        if (sscanf(buffer, "%d %d %s %[^\n]", 
                                   &sender_ephemeral_port, &sender_listening_port, 
                                   sender_team, received_message) != 4){
                            printf("Invalid message format received\n");
                            closesocket(client_socket);
                            continue;
                        }
                        if (strcmp(received_message, "exit") == 0){
                            remove_peer(client_ip, sender_listening_port);
                            printf("\n%s:%d has exited.\n", client_ip, sender_listening_port);
                            closesocket(client_socket);
                            continue;
                        }
                        printf("\n%s:%d %s %s\n", client_ip, sender_listening_port, sender_team, received_message);
                        add_peer(client_ip, sender_listening_port);
                    }
                }
                else{
                    int sender_ephemeral_port, sender_listening_port;
                    char sender_team[50];
                    char received_message[BUFFER_SIZE];
                    if (sscanf(buffer, "%d %d %s %[^\n]", 
                               &sender_ephemeral_port, &sender_listening_port, 
                               sender_team, received_message) != 4){
                        printf("Invalid message format received\n");
                        closesocket(client_socket);
                        continue;
                    }
                    if (strcmp(received_message, "exit") == 0){
                        remove_peer(client_ip, sender_listening_port);
                        printf("\n%s:%d has exited.\n", client_ip, sender_listening_port);
                        closesocket(client_socket);
                        continue;
                    }
                    printf("\n%s:%d %s %s\n", client_ip, sender_listening_port, sender_team, received_message);
                    add_peer(client_ip, sender_listening_port);
                }

                closesocket(client_socket);
            }
        } 
        else if (sel_result < 0){
            printf("select() failed: %d\n", WSAGetLastError());
        }
    }
    return 0;
}

void send_message(int original_port){
    char ip[INET_ADDRSTRLEN];
    int port;
    char message[BUFFER_SIZE];
    struct sockaddr_in peer_addr, local_addr;
    int ephemeral_port;
    socklen_t addr_len = sizeof(local_addr);
    
    printf("Enter recipient's IP: ");
    scanf("%s", ip);
    printf("Enter recipient's port: ");
    scanf("%d", &port);
    getchar(); 
    printf("Enter your message: ");
    fgets(message, BUFFER_SIZE, stdin);
    message[strcspn(message, "\n")] = 0; 

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET){
        printf("Socket creation failed\n");
        return;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = 0;

    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR){
        printf("Ephemeral port binding failed\n");
        closesocket(sock);
        return;
    }

    if (getsockname(sock, (struct sockaddr *)&local_addr, &addr_len) == SOCKET_ERROR){
        printf("Failed to get ephemeral port\n");
        closesocket(sock);
        return;
    }
    ephemeral_port = ntohs(local_addr.sin_port);

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port);
    if (!inet_pton_win(AF_INET, ip, &peer_addr.sin_addr)){
        printf("Invalid IP address format\n");
        closesocket(sock);
        return;
    }
    
    if (connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0){
        printf("Connection failed\n");
        closesocket(sock);
        return;
    }

    char formatted_message[BUFFER_SIZE];
    sprintf(formatted_message, "%d %d %s %s", ephemeral_port, original_port, team_name, message);

    send(sock, formatted_message, strlen(formatted_message), 0);
    closesocket(sock);
}

void send_file(int original_port){
    char ip[INET_ADDRSTRLEN];
    int port;
    char filepath[256];
    printf("Enter recipient's IP: ");
    scanf("%s", ip);
    printf("Enter recipient's port: ");
    scanf("%d", &port);
    printf("Enter file path: ");
    scanf("%s", filepath);

    FILE *fp = fopen(filepath, "rb");
    if (!fp){
         printf("Failed to open file: %s\n", filepath);
         return;
    }

    fseek(fp, 0, SEEK_END);
    long total_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *filename = strrchr(filepath, '\\');
    if (!filename){
         filename = strrchr(filepath, '/');
    }
    if (filename){
         filename++; 
    } else{
         filename = filepath;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET){
        printf("Socket creation failed\n");
        fclose(fp);
        return;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = 0;

    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR){
        printf("Ephemeral port binding failed\n");
        closesocket(sock);
        fclose(fp);
        return;
    }

    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(sock, (struct sockaddr *)&local_addr, &addr_len) == SOCKET_ERROR){
        printf("Failed to get ephemeral port\n");
        closesocket(sock);
        fclose(fp);
        return;
    }
    int ephemeral_port = ntohs(local_addr.sin_port);

    struct sockaddr_in peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port);
    if (!inet_pton_win(AF_INET, ip, &peer_addr.sin_addr)){
        printf("Invalid IP address format\n");
        closesocket(sock);
        fclose(fp);
        return;
    }

    if (connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0){
        printf("Connection failed\n");
        closesocket(sock);
        fclose(fp);
        return;
    }

    char header[BUFFER_SIZE];
    snprintf(header, BUFFER_SIZE, "%d %d %s FILE %s %ld\n", ephemeral_port, original_port, team_name, filename, total_size);
    if (send(sock, header, strlen(header), 0) == SOCKET_ERROR){
        printf("Failed to send header\n");
        closesocket(sock);
        fclose(fp);
        return;
    }

    char file_buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(file_buffer, 1, BUFFER_SIZE, fp)) > 0){
        size_t total_sent = 0;
        while (total_sent < bytes_read){
            int sent = send(sock, file_buffer + total_sent, bytes_read - total_sent, 0);
            if (sent == SOCKET_ERROR){
                printf("Failed to send file data\n");
                closesocket(sock);
                fclose(fp);
                return;
            }
            total_sent += sent;
        }
    }

    fclose(fp);
    closesocket(sock);
    printf("File '%s' sent successfully.\n", filename);
}

void connect_to_active_peers(int original_port){
    char connect_message[255];
    printf("Enter Broadcast Message: ");
    scanf("%s", connect_message);
    for (int i = 0; i < peer_count; i++){
        char *peer_ip = peer_list[i].ip;
        int peer_port = peer_list[i].port;

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET){
            printf("Socket creation failed for connection to %s:%d\n", peer_ip, peer_port);
            continue;
        }
        
        struct sockaddr_in peer_addr;
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(peer_port);
        if (!inet_pton_win(AF_INET, peer_ip, &peer_addr.sin_addr)){
            printf("Invalid IP address format for %s\n", peer_ip);
            closesocket(sock);
            continue;
        }

        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY;
        local_addr.sin_port = 0;

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR){
            printf("Failed to bind ephemeral port for %s:%d\n", peer_ip, peer_port);
            closesocket(sock);
            continue;
        }

        socklen_t addr_len = sizeof(local_addr);
        if (getsockname(sock, (struct sockaddr *)&local_addr, &addr_len) == SOCKET_ERROR){
            printf("Failed to get ephemeral port for %s:%d\n", peer_ip, peer_port);
            closesocket(sock);
            continue;
        }
        int ephemeral_port = ntohs(local_addr.sin_port);

        if (connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0){
            printf("Connection failed to %s:%d\n", peer_ip, peer_port);
            closesocket(sock);
            continue;
        }

        char connection_message[BUFFER_SIZE];
        sprintf(connection_message, "%d %d %s %s", ephemeral_port, original_port, team_name, connect_message);
        send(sock, connection_message, strlen(connection_message), 0);
        printf("Connection message sent to %s:%d\n", peer_ip, peer_port);
        closesocket(sock);
    }
}

void send_exit_to_all(int original_port){
    for (int i = 0; i < peer_count; i++){
        char *peer_ip = peer_list[i].ip;
        int peer_port = peer_list[i].port;

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET){
            printf("Socket creation failed for exit message to %s:%d\n", peer_ip, peer_port);
            continue;
        }
        
        struct sockaddr_in peer_addr;
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(peer_port);
        if (!inet_pton_win(AF_INET, peer_ip, &peer_addr.sin_addr)){
            printf("Invalid IP address format for %s\n", peer_ip);
            closesocket(sock);
            continue;
        }

        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY;
        local_addr.sin_port = 0;

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR){
            printf("Failed to bind ephemeral port for exit message to %s:%d\n", peer_ip, peer_port);
            closesocket(sock);
            continue;
        }

        socklen_t addr_len = sizeof(local_addr);
        if (getsockname(sock, (struct sockaddr *)&local_addr, &addr_len) == SOCKET_ERROR){
            printf("Failed to get ephemeral port for exit message to %s:%d\n", peer_ip, peer_port);
            closesocket(sock);
            continue;
        }
        int ephemeral_port = ntohs(local_addr.sin_port);

        if (connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0){
            printf("Connection failed for exit message to %s:%d\n", peer_ip, peer_port);
            closesocket(sock);
            continue;
        }

        char exit_message[BUFFER_SIZE];
        sprintf(exit_message, "%d %d %s exit", ephemeral_port, original_port, team_name);
        send(sock, exit_message, strlen(exit_message), 0);
        printf("Exit message sent to %s:%d\n", peer_ip, peer_port);
        closesocket(sock);
    }
}

void start_server(int port){
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET){
        printf("Server socket creation failed\n");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        printf("Bind failed\n");
        exit(1);
    }

    listen(server_socket, 5);
    printf("Server listening on port %d\n", port);
}

void stop_server(){
    running = 0;
    closesocket(server_socket);
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    WSACleanup();
    printf("Server stopped.\n");
}

int main(){
    WSADATA wsa;
    int port;
    
    WSAStartup(MAKEWORD(2, 2), &wsa);

    printf("Enter your team name: ");
    scanf("%s", team_name);
    printf("Enter your port number: ");
    scanf("%d", &port);
    
    start_server(port);

    thread = CreateThread(NULL, 0, receive_messages, NULL, 0, NULL);
    if (thread == NULL){
        printf("Thread creation failed\n");
        return 1;
    }

    int choice;
    while (running){
        printf("\n***** Menu *****\n");
        printf("1. Send message\n");
        printf("2. Query active peers\n");
        printf("3. Connect to active peers\n");
        printf("4. Send file\n");
        printf("0. Quit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice){
            case 1:
                send_message(port);
                break;
            case 2:
                query_peers();
                break;
            case 3:
                connect_to_active_peers(port);
                break;
            case 4:
                send_file(port);
                break;
            case 0:
                send_exit_to_all(port);
                stop_server();
                exit(0);
            default:
                printf("Invalid choice\n");
        }
    }
    return 0;
}
