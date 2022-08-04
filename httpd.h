#pragma once
#ifndef HTTPD_H
#define HTTPD_H

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#define ISspace(x) isspace((int)(x))
#define SERVER_STRING "WebServer: ming httpd/0.1.1\r\n"
using namespace std;
class httpd;
struct mypar{
	httpd* httpnow;
	int client_sock;
};
class httpd {
public:
	//static void* accept_request(void* A,void* from_client){//����http����
	static void* accept_request(void* nowpar){
	mypar* newpar = static_cast<mypar*>(nowpar);
	//httpd *thiscla = static_cast<httpd*>(A);
	httpd *thiscla=(newpar->httpnow);
	//int client = *(int *)from_client;
 	int client = newpar->client_sock;
	char buf[1024];//����洢
	int numchars;//��ȡһ�еĳ���
	char method[255];//�ж���post����get
	char url[255];
	char path[512];//�ļ�����·��
	size_t i=0, j=0;
	struct stat st;
	int cgi = 0;//cgi��־��
	char* query_string = NULL;//��ֹ����Ϣ
	numchars = thiscla->get_line(client, buf, sizeof(buf));//��õ�һ����Ϣ
	while (!ISspace(buf[j]) && (i < sizeof(method) - 1))//��ȡ����
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';//POST GET !!!!!!
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) //�Ȳ���get�ֲ���post
	{
		thiscla->unimplemented(client);
		return NULL;
	}
	if (strcasecmp(method, "POST") == 0)  cgi = 1;//�����post��������cgi
	i = 0;
	while (ISspace(buf[j]) && (j < sizeof(buf))) j++;//��λ��url�Ŀ�ʼ�ط�
	while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	{
		url[i] = buf[j];//��ȡurl��Դ��Ϣ
		i++; j++;
	}
	url[i] = '\0';
	if (strcasecmp(method, "GET") == 0) {
		query_string = url;
		while ((*query_string != '?') && (*query_string != '\0'))
			query_string++;
		if (*query_string == '?')//����в�������Ҫ����cgi����λ�������ĵ�һ��
		{
			cgi = 1;
			*query_string = '\0';//url������Դ�Ŀ�ͷ
			query_string++;
		}
	}
	sprintf(path, "minghttp%s", url);
	if (path[strlen(path) - 1] == '/') strcat(path, "index.html");//�����/�������һ����
	if (stat(path, &st) == -1) {
		while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
			numchars = thiscla->get_line(client, buf, sizeof(buf));
		thiscla->not_found(client);
	}
	else {
		if ((st.st_mode & S_IFMT) == S_IFDIR) {//����������ΪĿ¼, �Զ���test.html
			strcat(path, "/test.html");
		}
		if ((st.st_mode & S_IXUSR) ||
			(st.st_mode & S_IXGRP) ||
			(st.st_mode & S_IXOTH))
			//S_IXUSR:�ļ������߾߿�ִ��Ȩ��
			//S_IXGRP:�û���߿�ִ��Ȩ��
			//S_IXOTH:�����û��߿ɶ�ȡȨ�� 
			cgi = 1;
		if (!cgi) {
			//cout<<1<<endl;
			thiscla->serve_file(client, path);
		}
		else thiscla->execute_cgi(client, path, method, query_string);
	}
	close(client);
	cout << "connection close....client: " << client << '\n' << endl;
	//printf("connection close....client: %d \n", client);
	return NULL;
	}
public:
	void bad_request(int);//��������http����
	void cat(int, FILE*);//ֻ����ʽ��ȡ��Ϣ
	void cannot_execute(int);//������ִ��cgi
	void error_die(const char*);//�����ź�ָ�����˳�
	void execute_cgi(int, const char*, const char*, const char*);//ִ��cgi������
	int get_line(int, char*, int);//����һ����Ϣ
	void headers(int, const char*);//��ȡͷ����Ϣ
	void not_found(int);//����400
	void serve_file(int, const char*);//��ȡ�ļ�����
	int startup(u_short*);//�ͻ�������
	void unimplemented(int);
private:

};


#endif // !HTTPD.H
