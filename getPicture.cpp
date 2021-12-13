// getPicture.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "spider.h"

int main()
{
	cout << "*********************************************************" << endl;
	cout << "***********************爬取图片系统***********************" << endl;
	cout << "*********************************************************" << endl;

	//创建图片的储存的目录
	CreateDirectory(L"./img", NULL);

	//开始抓取
	string starturl = "https://bbs.byr.cn/#!board/Friends";

	StartCatch(starturl);
	//while (1);
	return 0;
}


void StartCatch(string startUrl)
{
	queue<string> q;
	q.push(startUrl);

	while (!q.empty())
	{
		string cururl = q.front();
		q.pop();

		//解析URL
		if (false == Analyse(cururl))
		{
			std::cout << "解析URL失败，错误码：" << GetLastError() << endl;
			continue;
		}

		//连接服务器
		if (false == Connect())
		{
			cout << "连接服务器失败，错误代码：" << GetLastError() << endl;
			continue;
		}

		//建立ssl连接
		if (false == SSL_Connect())
		{
			cout << "建立SSL连接失败，错误代码：" << GetLastError() << endl;
			continue;
		}

		//获取网页
		string html;
		if (false == Gethtml(html))
		{
			cout << "获取网页数据失败，错误代码：" << GetLastError() << endl;
			continue;
		}
		if (false == RegexIamage(html))
		{
			cout << "获取网页数据失败，错误代码：" << GetLastError() << endl;
			continue;
		}
		//cout << html << endl;
	}
	//释放
	SSL_shutdown(sslHandle);
	SSL_free(sslHandle);
	SSL_CTX_free(sslContext);
	closesocket(g_sock);
	WSACleanup();
}


//解析url
bool Analyse(string url)
{
	char* pUrl = new char[url.length() + 1];
	strcpy(pUrl, url.c_str());

	char* pos = strstr(pUrl, "https://");//找到http://开头的字符串
	if (pos == NULL) return false;
	else pos += 8;//将http://开头省略

	sscanf(pos, "%[^/]%s", g_Host, g_Object);

	delete[] pUrl;
	return true;
}


//建立TCP连接
bool Connect()
{
	//初始化套接字
	WSADATA wsadata;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata)) return false;

	//创建套接字
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET) return false;

	//将域名转换为IP地址
	hostent* p = gethostbyname(g_Host);
	if (p == NULL) return false;

	sockaddr_in sa;
	memcpy(&sa.sin_addr, p->h_addr, 4);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(443);

	if (SOCKET_ERROR == connect(g_sock, (sockaddr*)&sa, sizeof(sockaddr))) return false;
	return true;
}


bool SSL_Connect()
{
	// Register the error strings for libcrypto & libssl

	ERR_load_BIO_strings();
	// SSl库的初始化，载入SSL的所有算法，载入所有的SSL错误信息
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	// New context saying we are a client, and using SSL 2 or 3
	sslContext = SSL_CTX_new(SSLv23_client_method());
	if (sslContext == NULL)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	// Create an SSL struct for the connection
	sslHandle = SSL_new(sslContext);
	if (sslHandle == NULL)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	// Connect the SSL struct to our connection
	if (!SSL_set_fd(sslHandle, g_sock))
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	// Initiate SSL handshake
	if (SSL_connect(sslHandle) != 1)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}

	return true;
}

bool Gethtml(string& html)
{
	char temp1[100];
	sprintf(temp1, "%d", 166);
	string c_get;
	c_get = c_get
		+ "GET " + g_Object + " HTTP/1.1\r\n"
		+ "Host: " + g_Host + "\r\n"
		+ "Content-Type: text/html; charset=UTF-8\r\n"
		//+ "Content-Length:" + temp1 + "\r\n"
		//+ "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36 Edge/16.16299\r\n"
		+ "Connection:Close\r\n\r\n";
	//+ temp;

	SSL_write(sslHandle, c_get.c_str(), c_get.length());

	char buff[101];
	int nreal = 0;

	while ((nreal = SSL_read(sslHandle, buff, 100)) > 0)
	{
		buff[nreal] = '\0';
		html += UtfToGbk(buff);
		//printf("%s\n", buff);
		memset(buff, 0, sizeof(buff));
	}
	//printf("%s\n", html);
	return true;
}

string UtfToGbk(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}


bool RegexIamage(string& html)
{
	smatch mat;
	regex rgx("src=\"(.*(png|svg|jpg))\"");
	string::const_iterator start = html.begin();
	string::const_iterator end = html.end();
	string per;
	int i = 1;
	while (regex_search(start, end, mat, rgx))
	{
		//URL生成
		per = mat[1].str();
		size_t len = per.length();//获取字符串长度
		int nmlen = MultiByteToWideChar(CP_ACP, 0, per.c_str(), len + 1, NULL, 0);//如果函数运行成功，并且cchWideChar为零 //返回值是接收到待转换字符串的缓冲区所需求的宽字符数大小。
		wchar_t* buffer = new wchar_t[nmlen];
		MultiByteToWideChar(CP_ACP, 0, per.c_str(), len + 1, buffer, nmlen);
		//保存路径
		string savepath = "E:\\c++_file\\网络爬虫1\\网络爬虫\\网络爬虫\\img\\" + to_string(i) + ".jpg";
		size_t len1 = savepath.length();
		wchar_t* imgsavepath = new wchar_t[len1];
		int nmlen1 = MultiByteToWideChar(CP_ACP, 0, savepath.c_str(), len1 + 1, imgsavepath, len1);
		cout << mat.str() << endl;
		cout << savepath << endl;
		//下载文件
		HRESULT hr = URLDownloadToFile(NULL, buffer, imgsavepath, 0, NULL);
		if (hr == S_OK)
		{
			cout << "-------ok" << endl;
		}
		start = mat[0].second;
		i++;
	}
	return true;
}

LPCWSTR stringToLPCWSTR(string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);


	return wcstring;
}
