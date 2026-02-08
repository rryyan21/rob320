#pragma once

// C Standard Library headers
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// C POSIX library headers
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#define DISCOVERY_PORT 8000

extern int ctrl_c_pressed;

// This function uses the gethostname and gethostbyname functions to get the public IP address
// of the current machine. It returns the public IP address as a string.
char *get_public_ip()
{
    char hostbuffer[256];
    struct hostent *host_entry;
    int hostname;
    struct in_addr **addr_list;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1)
    {
        return NULL;
    }

    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL)
    {
        return NULL;
    }

    char *ip;
    addr_list = (struct in_addr **)host_entry->h_addr_list;
    for (int i = 0; addr_list[i] != NULL; i++)
    {
        ip = inet_ntoa(*addr_list[i]);
        if (ip == NULL)
        {
            return NULL;
        }

        if (strncmp(ip, "127", 3) != 0)
        {
            char *ip_copy = strdup(ip);
            if (ip_copy == NULL)
            {
                return NULL;
            }
            return ip_copy;
        }
    }
    return NULL;
}

int connect_until_success(int fd, struct sockaddr_in *address)
{
    ssize_t status = 0;
    while (!ctrl_c_pressed)
    {
        // TODO: Connect to the specified address
        status = connect(fd, (struct sockaddr *)address, sizeof(*address));

        if (status < 0)
        {
            if (errno == EALREADY || errno == EINPROGRESS)
            {
                // TODO: Handle the case where the connection is in progress
                //       (errno is EALREADY or EINPROGRESS)
                // Hint: Just sleep for a bit and try again
                usleep(10000); // Sleep for 10ms
                continue;
            }
            else if (errno == EISCONN)
            {
                // TODO: Handle the case where the connection is already established
                //      (errno is EISCONN)
                // Hint: Return 0 for success
                return 0;
            }
            else
            {
                // If errno is anything else, return -1
                return -1;
            }
        }
        else
        {
            // If the connection is successful, return 0
            return 0;
        }
    }
    // Return 1 if the loop is exited
    return 1;
}

int accept_until_success(int fd, int *client_fd)
{
    int new_fd;
    while (!ctrl_c_pressed)
    {
        // TODO: Accept a connection from a client
        new_fd = accept(fd, NULL, NULL);

        if (new_fd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // TODO: Handle the case where the connection is in progress or no clients are available
                //       (errno is EAGAIN or EWOULDBLOCK)
                // Hint: Just sleep for a bit and try again
                usleep(10000);
                continue;
            }
            else
            {
                // If errno is anything else, return -1
                return -1;
            }
        }
        else
        {
            // TODO: If the connection is successful, set the client file descriptor and return 0
            *client_fd = new_fd;
            return 0;
        }
    }
    // Return 1 if the loop is exited
    return 1;
}

int recv_until_success(int fd, uint8_t *buffer, size_t len)
{
    ssize_t status = 0, bytes_read = 0;
    while (!ctrl_c_pressed)
    {
        // TODO: Receive data from the specified file descriptor
        status = recv(fd, buffer + bytes_read, len - bytes_read, 0);

        if (status < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // TODO: Handle the case where no data is available
                //       (errno is EAGAIN or EWOULDBLOCK)
                // Hint: Just sleep for a bit and try again
                usleep(10000);
                continue;
            }
            else
            {
                // If errno is anything else, return -1
                return -1;
            }
        }
        else
        {
            // TODO: If the data is successfully received, update the number of bytes read
            //       and return 0 if all data has been received
            if (status == 0)
            {
                // Peer closed connection
                return -1;
            }
            bytes_read += status;
            if ((size_t)bytes_read >= len)
            {
                return 0;
            }
            continue;
        }
    }
    // Return 1 if the loop is exited
    return 1;
}

int send_until_success(int fd, uint8_t *buffer, size_t len)
{
    ssize_t status = 0, bytes_sent = 0;
    while (!ctrl_c_pressed)
    {
        // TODO: Send data to the specified file descriptor
        status = send(fd, buffer + bytes_sent, len - bytes_sent, 0);

        if (status < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // TODO: Handle the case where the operation would block
                //       (errno is EAGAIN or EWOULDBLOCK)
                // Hint: Just sleep for a bit and try again
                usleep(10000);
                continue;
            }
            else
            {
                // If errno is anything else, return -1
                return -1;
            }
        }
        else
        {
            // TODO: If the data is successfully sent, update the number of bytes sent
            //       return 0 if all data has been sent
            if (status == 0)
            {
                return -1;
            }
            bytes_sent += status;
            if ((size_t)bytes_sent >= len)
            {
                return 0;
            }
            continue;
        }
    }
    // Return 1 if the loop is exited
    return 1;
}