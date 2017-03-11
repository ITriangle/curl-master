#ifndef _HTTP_MANAGER_H_
#define _HTTP_MANAGER_H_

#include "curl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>


// #include "json.h"

using namespace std;

class HttpObject
{
public:
    HttpObject(string sIP, string sPort);
    ~HttpObject();

    /* Must initialize libcurl before any threads are started */
    static bool m_bInitFlag ;//是否进行全局初始化
    static void Curl_Global_Init();
    void Curl_Global_Cleanup();

    bool HttpPost(std::string sCmd, std::string strJson);
    bool HttpPut(std::string sCmd, std::string strJson);
    bool HttpGet(std::string sCmd);
    bool HttpHead(std::string sCmd);
    bool HttpDelete(std::string sCmd);


    bool HttpGet(std::string sCmd, std::string strJson);

    string m_sReplyCache;//应答缓存

    //直接url发送
    static bool HttpPost_urlencoded(std::string sURL, std::string sContext);
private:

    bool curl_init();

    bool curl_perform();

    void curl_exit();

 
    string m_sIP;//IP地址
    string m_sPort;//端口号

    string m_sCurlBase;//url基础部分

    CURL * m_pCurl;//操作的curl句柄
};


#endif