#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

#define MAX_CLIENTS 64

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];

    int users[64];      // Mang socket client da dang nhap
    char *user_ids[64]; // Mang luu tru id cua client da dang nhap
    int num_users = 0;  // So client da dang nhap

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        printf("ret = %d\n", ret);

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if (nfds < MAX_CLIENTS)
            {
                printf("New client connected: %d\n", client);
                 char *msg = "Enter the format 'client_id: client_name' to chat \n";
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                send(fds[nfds].fd,msg,strlen(msg),0);
                nfds++;
               
                
            }
            else
            {
                printf("Too many connections\n");
                close(client);
            }
        }

        for (int i = 1; i < nfds; i++)
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Client %d disconnected.\n", fds[i].fd);
                    
                    for (int l = 0; l < num_users; l++)
                    {
                        if (users[l]==fds[i].fd){
                            users[l]=0;
                            free(user_ids[l]);
                            num_users--;
                        }
                        /* code */
                    }
                    close(fds[i].fd);
                    // Xoa phan tu i khoi mang
                    if (i < nfds - 1){
                        fds[i] = fds[nfds - 1];
                    }
                        nfds--;
                        i--;
                    
                    
                }
                else
                {
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    int client = fds[i].fd;

                        int j = 0;
                        for (; j < num_users; j++)
                            if (users[j] == client) break;

                        if (j == num_users)
                        {
                            // Chua dang nhap
                            // Xu ly cu phap yeu cau dang nhap
                            
                            char cmd[32], id[32], tmp[32];
                            ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
                            if (ret == 2)
                            {
                                if (strcmp(cmd, "client_id:") == 0)
                                {
                                    char *msg = "Dung cu phap. Gui tin nhan.(neu muon gui tin nhan rieng thi dung cu pha [name] msg)\n";
                                    send(client, msg, strlen(msg), 0);

                                    int k = 0;
                                    for (; k < num_users; k++)
                                        if (strcmp(user_ids[k], id) == 0) break;
                                    
                                    if (k < num_users)
                                    {
                                        char *msg = "ID da ton tai. Yeu cau nhap lai.\n";
                                        send(client, msg, strlen(msg), 0);
                                    }
                                    else
                                    {
                                        users[num_users] = client;
                                        user_ids[num_users] = malloc(strlen(id) + 1);
                                        strcpy(user_ids[num_users], id);
                                        num_users++;
                                    }                                    
                                }
                                else
                                {
                                    char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                                    send(client, msg, strlen(msg), 0);
                                }
                            }
                            else
                            {
                                char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                                send(client, msg, strlen(msg), 0);
                            }
                        }
                        else
                        {
                            // Da dang nhap
                            char sendbuf[512];
                            char cmd[32],temp[32],name[32],msg[256];
                            
                            // Kiem tra format [ten] msg
                            if (strchr(buf,'[')==buf && strchr(buf,']')>buf+1){
                            int ret1 = sscanf(buf,"%s%s",cmd,temp);

                            if (ret1==2){
                                
                                // lay ten tu [ten] roi kiem tra
                                sscanf(cmd,"[%[^]]]",name);
                                strcpy(msg,strchr(buf,']')+1);
                                int n=0;
                                for (;n<num_users;n++){
                                    if (strcmp(user_ids[n],name)==0){
                                        sprintf(sendbuf, "[%s]: %s", user_ids[j], msg);
                                        send(users[n], sendbuf, strlen(sendbuf), 0);
                                    }
                                }

                                break;
                            }
                            }

                            sprintf(sendbuf, "%s: %s", user_ids[j], buf);
                            for (int k = 0; k < num_users; k++)
                                if (users[k] != client)
                                    send(users[k], sendbuf, strlen(sendbuf), 0);
                        }
                }
            }
    }

    close(listener);    

    return 0;
}
