#pragma once

#include <iostream>
#include <Windows.h>
#include <string>
#include <queue>
#include <regex>
#include <urlmon.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "urlmon.lib")
#pragma comment( lib, "libeay32.lib" )
#pragma comment( lib, "ssleay32.lib" )

#pragma comment(lib, "WS2_32")  // 链接到WS2_32.lib

using namespace std;

char g_Host[MAX_PATH];
char g_Object[MAX_PATH];

SOCKET g_sock;
SSL* sslHandle;
SSL_CTX* sslContext;
BIO* bio;


//开始抓取
void StartCatch(string startUrl);
//解析URL
bool Analyse(string url);
//连接服务器
bool Connect();
//建立SSl连接
bool SSL_Connect();
//得到html
bool Gethtml(string& html);
//UTF转GBK
std::string UtfToGbk(const char* utf8);
//正则表达式
bool RegexIamage(string& html);

LPCWSTR stringToLPCWSTR(std::string orig);
