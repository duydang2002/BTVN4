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
#define MAX_LEN 128
#define MAX_LINE 128

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
   FILE *f = fopen("database.txt","r");
    if (f==NULL){
        printf("Error opening file\n");
        return 0;
        }
    char data[MAX_LINE][MAX_LEN];
    int line =0;
    while (!feof(f) && !ferror(f))
    {
       if (fgets(data[line],MAX_LEN,f)!=NULL)
        line++;
    }
   for (int i = 0; i < line; i++) {
    data[i][strcspn(data[i], "\n")] = '\0'; // Loại bỏ ký tự '\n'
}
    fclose(f);

                                

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];

    int users[64];      // Mang socket client da dang nhap
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
                 char *msg = "Enter the format 'UserName Password' to login ";
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
                    // buf[ret] = 0;
                    buf[strcspn(buf,"\n")]= '\0';
                    // printf ("%ld",strcspn(buf,'\n'));
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    int client = fds[i].fd;

                        int j = 0;
                        for (; j < num_users; j++)
                            if (users[j] == client) break;

                        if (j == num_users)
                        {
                            // Chua dang nhap
                            // Xu ly cu phap yeu cau dang nhap
                            
                            char account[32], passW[32], tmp[32];
                            ret = sscanf(buf, "%s%s%s", account, passW, tmp);
                            if (ret == 2)
                            {
                                int found =0;
                                for (int i=0;i<line;i++){
                                char acc[64], pass[64];;
                                sscanf(data[i],"%s%s",acc,pass);
                                if (strcmp(account,acc)==0 && strcmp(passW,pass)==0){
                                    found =1;
                                    break;
                                }
}
                                // Dang nhap thanh cong
                                if (found){
                                    users[num_users]=client;
                                    num_users++;
                                    char *msg = "Dang Nhap Thanh Cong\n";
                                    send(client, msg, strlen(msg), 0);
                                }
                                else{
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
                            char *command = malloc(strlen(buf)+1);
                            sprintf(command,"%s > out.txt",buf);
                           
                            int result = system(command);
                            
                            if (result == -1) {
                                char *msg = "Failed to execute command\n";
                                send(client, msg, strlen(msg), 0);
                            }
                            
                            // Kiem tra ket qua thuc thi lenh
                            if (result == 0) {
                                char *msg = "Command executed successfully.\n";
                                send(client, msg, strlen(msg), 0);
                                char result[256];
                                FILE *file = fopen("out.txt","r");
                                if (file == NULL) {
                                    perror("Cannot Opening File");
                                    return 0;
                                }
                                
                                while (!feof(f) && !ferror(f))
                                {
                                    if  (fgets(result,sizeof(result),file)!= NULL){
                                    send(client,result,strlen(result),0);
                                    }
                                }
                                fclose(file);
                              
                            } else {
                                char *msg = "Command execution failed.\n";
                                send(client, msg, strlen(msg), 0);
                                
                            }
    
                        }
                }
            }
    }

    close(listener);    

    return 0;
}