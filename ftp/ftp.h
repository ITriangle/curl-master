#ifndef _FTP_MANAGER_H_
#define _FTP_MANAGER_H_

#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include "curl.h"

using namespace std;

class FTPObject
{
public:
    FTPObject();
    FTPObject(string sIP, string sUserKey, string sLocalDirPath, string sFtpDirPath);
    ~FTPObject();

    /* Must initialize libcurl before any threads are started */
    static bool m_bInitFlag ;//是否进行全局初始化
    static void Curl_Global_Init();

    void Curl_Global_Cleanup();

    /*upload file to ftp server*/
    bool FTP_UpFile(string sFilename);

    /*download file from ftp server*/
    bool FTP_DwonFile(string sFilename);


    bool FTP_UpRenameFile(string sFilename, string sRenamed_FileName);


private:

    bool curl_init();

    bool curl_perform();

    void curl_exit();

    void curl_set_upload_opt(const char *cURL,  FILE *file);

    void curl_set_download_opt(const char *cURL,  FILE *file);

    void curl_set_upload_rename_opt( const char *cURL, FILE *file, struct curl_slist *headerlist);

    /*get the size of file */
    int get_file_size(FILE *file);



    string m_sIP;/*ftp server IP*/
    string m_sUserKey;/*username:password*/
    
    string m_sCurlBase;//url基础部分

    string m_sLocalDirPath;//本地文件目录
    string m_sFtpDirPath;//ftp服务文件目录

    CURL * m_pCurl;//操作的curl句柄
};


#endif