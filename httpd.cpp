#include <httpd.h>


void httpd::bad_request(int client) {//��������http����
	char buf[1024];
	//����400
	sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "<P>Your browser sent a bad request, ");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "such as a POST without a Content-Length.\r\n");
	send(client, buf, sizeof(buf), 0);
}
void httpd::cat(int client, FILE* resource) {
	char buf[1024];
	fgets(buf, sizeof(buf), resource);//��ȡһ��
	while (!feof(resource))//�ж��ǲ������һ��
	{
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}//ֻ����ʽ��ȡ��Ϣ
void httpd::cannot_execute(int client) {//������ִ��cgi
	char buf[1024];
	//����500
	sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<P>Error prohibited CGI execution.\r\n");//<P>�ǿ�һ����
	send(client, buf, strlen(buf), 0);
}
void httpd::error_die(const char* ername) {//�����ź�ָ�����˳�
	perror(ername);
	exit(1);
}
void httpd::execute_cgi(int client,const char* path,
	const char* method, const char* query_string) {//ִ��cgi������
	char buf[1024];
	int cgi_output[2];//�ܵ�
	int cgi_input[2];
	pid_t pid;
	int status;
	int i;
	char c;
	int numchars = 1;
	int content_length = -1;
	buf[0] = 'A'; buf[1] = '\0';
	if (strcasecmp(method, "GET") == 0) { // �����ײ���Ϣ����ȡcontent-length
		while ((numchars > 0) && strcmp("\n", buf))  
			numchars = get_line(client, buf, sizeof(buf));//�����ײ���Ϣ
	}
	else {//�����post�����������ײ���Ϣ���õ�content-length
		numchars = get_line(client, buf, sizeof(buf));
		while ((numchars > 0) && strcmp("\n", buf)) {
			buf[15] = '\0';
			if (strcasecmp(buf, "Content-Length:") == 0) content_length = atoi(&(buf[16]));
			numchars = get_line(client, buf, sizeof(buf));
		}
		if (content_length == -1) {
			bad_request(client);
			return;
		}
	}
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	if (pipe(cgi_output) < 0) { //�����ܵ�
		cannot_execute(client);
		return;
	}
	if (pipe(cgi_input) < 0) {
		cannot_execute(client);
		return;
	}
	if ((pid = fork()) < 0) {//��������
		cannot_execute(client);
		return;
	}
	if (pid == 0)  /* �ӽ���: ����CGI�ű� */
	{
		char meth_env[255];
		char query_env[255];
		char length_env[255];
		dup2(cgi_output[1], 1);
		dup2(cgi_input[0], 0);//stdin���ض��򵽶�ȡ��
		close(cgi_output[0]);//�ر���cgi_output�еĶ�ͨ��
		close(cgi_input[1]);//�ر���cgi_input�е�дͨ��
		sprintf(meth_env, "REQUEST_METHOD=%s", method);//�������������󷽷�
		putenv(meth_env);
		if (strcasecmp(method, "GET") == 0) {
			//�洢QUERY_STRING
			sprintf(query_env, "QUERY_STRING=%s", query_string);//����������GET����
			putenv(query_env);
		}
		else {   /* POST */
			sprintf(length_env, "CONTENT_LENGTH=%d", content_length);//�洢CONTENT_LENGTH
			putenv(length_env);
		}
		execl(path, path, NULL);//ִ��CGI�ű�
		exit(0);
	}
	else {
		close(cgi_output[1]);
		close(cgi_input[0]);
		if (strcasecmp(method, "POST") == 0)
			for (i = 0; i < content_length; i++)
			{
				recv(client, &c, 1, 0);
				write(cgi_input[1], &c, 1);
			}
		//��ȡcgi�ű���������
		while (read(cgi_output[0], &c, 1) > 0)
			//���͸������
		{
			send(client, &c, 1, 0);
		}
		//���н����ر�
		close(cgi_output[0]);
		close(cgi_input[1]);
		waitpid(pid, &status, 0);
	}
}
int httpd::get_line(int sock, char* buf, int size) {//����һ��http��Ϣ,���س�����Ϣ
	int i = 0;
	char c = '\0';
	int n;
	while ((i < size - 1) && (c != '\n')) {//\r\n��ʾ���ǻس�����
		n = recv(sock, &c, 1, 0);
		if (n > 0) {//˵����������
			if (c == '\r') {//˵�����ܵ�ĩβ��
				n = recv(sock, &c, 1, MSG_PEEK);
				if ((n > 0) && (c == '\n')) // ���һ�ν��ǻ��з����˳�
					recv(sock, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c; i++;
		}
		else c='\n';//�˳�ѭ����
	}
	buf[i] = '\0';
	return(i);
}
void httpd::headers(int client, const char* filename) {//��ȡͷ����Ϣ
	char buf[1024];
	(void)filename;
	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}
void httpd::not_found(int client) {//����400
	char buf[1024];
	sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}
void httpd::serve_file(int client, const char* filename) {//��ȡ�ļ�����
	FILE* resource = NULL;
	int numchars = 1;
	char buf[1024];
	buf[0] = 'A';//��֤\n�Ա���ͨ��
	buf[1] = '\0';
	cout<<filename<<endl;
	while ((numchars > 0) && strcmp("\n", buf)) {
	    numchars = get_line(client, buf, sizeof(buf));
	   
	} /* read & discard headers */
	resource = fopen(filename, "r");//ֻ����ʽ���ļ�
	
	if (resource == NULL) not_found(client);
	else
	{
		headers(client, filename);
		cat(client, resource);
	}
	fclose(resource);
}
int httpd::startup(u_short* port) {//�ͻ�������
	int httpfd = 0; int option = 1;
	struct sockaddr_in name;//���������ĵ�ַ��
	httpfd = socket(PF_INET, SOCK_STREAM, 0);//��ʾipv4�Լ������ݣ�����TCPЭ��
	if (httpfd == -1) {//����socketʧ��
		error_die("socket");//�ú��������ֱ�ӱ������������
	}
	socklen_t optlen = sizeof(option);
	setsockopt(httpfd, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);//���׽��ּ���������ѡ��򿪵�ַ���ù���
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);//��������
	if (bind(httpfd, (struct sockaddr*)&name, sizeof(name)) < 0)
		error_die("bind");
	if (*port == 0) //��̬���ж˿ڴ���
	{
		socklen_t namelen = sizeof(name);
		if (getsockname(httpfd, (struct sockaddr*)&name, &namelen) == -1)
			error_die("getsockname");
		*port = ntohs(name.sin_port);
	}
	if (listen(httpfd, 5) < 0)
		error_die("listen");
	return(httpfd);
};
void httpd::unimplemented(int client) {
	char buf[1024];
	//����501˵����Ӧ����û��ʵ��
	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}
