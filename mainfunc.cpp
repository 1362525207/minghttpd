#include <stdio.h>
#include <httpd.h>
#include <iostream>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
using namespace std;

int main()
{
    httpd httpnow;
    struct mypar par;
    int server_sock = -1;
    u_short port = 8000;
    int client_sock = -1;
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);
    pthread_t newthread;

    server_sock = httpnow.startup(&port);
    cout << "http server_sock is " << server_sock << " the port is: " << port<<endl;
    while (true)
    {
        client_sock = accept(server_sock,(struct sockaddr*)&client_name,&client_name_len);
	  par.client_sock=client_sock;
	  par.httpnow = &httpnow;
        if (client_sock == -1)
            httpnow.error_die("accept");
        /* accept_request(client_sock); */
        if (pthread_create(&newthread, NULL, httpd::accept_request, (void*)(&par)) != 0)
            perror("pthread_create");
    }

    close(server_sock);
    return 0;
}

