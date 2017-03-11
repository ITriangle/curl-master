#include "ftp.h"

FTPObject::FTPObject()
{

}

FTPObject::FTPObject(string sIP, string sUserKey, string sLocalDirPath, string sFtpDirPath)
{
  m_sIP = sIP;
  m_sUserKey = sUserKey;

  /* "/home/wl/wlcode/libcurl/upload.txt" */
  m_sLocalDirPath = sLocalDirPath ;

  /*  "ftp://wlftp:wanglong@127.0.0.1:21/test/upload.txt"   */
  m_sFtpDirPath = sFtpDirPath ;

  m_sCurlBase = "ftp://" + sUserKey + "@" + m_sIP + ":21" ;

  m_pCurl = NULL;

  /**
   * 建立全局连接
   */
  // Curl_Global_Init();

  // cout << "m_sIP : " << m_sIP << endl;
  // cout << "m_sUserKey : " << m_sUserKey << endl;
  // cout << "m_sLocalDirPath : " << m_sLocalDirPath << endl;
  // cout << "m_sFtpDirPath : " << m_sFtpDirPath  << endl;
  // cout << "m_sCurlBase : " << m_sCurlBase << endl;

}

FTPObject::~FTPObject()
{
  printf("FTPObject:m_pCurl\n");
  if (m_pCurl != NULL)
  {
    curl_easy_cleanup(m_pCurl);
    m_pCurl = NULL;
  }
}


/*****************util api******************/
int FTPObject::get_file_size(FILE *file)
{
  int size = 0;
  fseek(file, 0L, SEEK_END);
  size = ftell(file);
  fseek(file, 0L, SEEK_SET);
  return size;
}

/******************m_pCurl api****************/
/**
 * 保证单利模式，用类变量作为对象之间通信方式；做到方法线程安全,减少线程里面局部变量的使用
 */
bool FTPObject::m_bInitFlag = false;

void FTPObject::Curl_Global_Init()
{
  if (!FTPObject::m_bInitFlag)
  {
    printf("----------------------**************************************-> Curl_Global_Init \n");
    /**
     * 要求只能执行一次，且在最开始的时候，所以，作为一种类方法，用局部静态变量，封装成只执行一次
     */
    curl_global_init(CURL_GLOBAL_ALL);
    FTPObject::m_bInitFlag = true;
  }

}

void FTPObject::Curl_Global_Cleanup()
{
  curl_global_cleanup();
}

bool FTPObject::curl_init()
{
  // curl_global_init(CURL_GLOBAL_ALL);
  m_pCurl = curl_easy_init();
  if (NULL == m_pCurl)
  {
    fprintf(stderr, "Init m_pCurl failed.\n");
    return false;
  }

  //忽略信号
  // curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);


  return true;
}

void FTPObject::curl_exit()
{
  curl_easy_cleanup(m_pCurl);
  m_pCurl = NULL;
  // curl_global_cleanup();
}


void FTPObject::curl_set_upload_opt( const char *cURL, FILE *file)
{

  curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(m_pCurl, CURLOPT_FORBID_REUSE, 1);
  curl_easy_setopt(m_pCurl, CURLOPT_FRESH_CONNECT, 1);
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(m_pCurl, CURLOPT_URL, cURL);
  curl_easy_setopt(m_pCurl, CURLOPT_READDATA, file);
  curl_easy_setopt(m_pCurl, CURLOPT_UPLOAD, 1);
  curl_easy_setopt(m_pCurl, CURLOPT_INFILESIZE, get_file_size(file));
  curl_easy_setopt(m_pCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);
//  curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1);
}

void FTPObject::curl_set_upload_rename_opt( const char *cURL, FILE *file, struct curl_slist *headerlist)
{

  curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 10L);
  curl_easy_setopt(m_pCurl, CURLOPT_FORBID_REUSE, 1);
  curl_easy_setopt(m_pCurl, CURLOPT_FRESH_CONNECT, 1);
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(m_pCurl, CURLOPT_URL, cURL);
  curl_easy_setopt(m_pCurl, CURLOPT_POSTQUOTE, headerlist);
  curl_easy_setopt(m_pCurl, CURLOPT_READDATA, file);
  curl_easy_setopt(m_pCurl, CURLOPT_UPLOAD, 1);
  curl_easy_setopt(m_pCurl, CURLOPT_INFILESIZE, get_file_size(file));
  curl_easy_setopt(m_pCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);
//  curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1);
}

void FTPObject::curl_set_download_opt( const char *cURL,  FILE *file)
{
  curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(m_pCurl, CURLOPT_FORBID_REUSE, 1);
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(m_pCurl, CURLOPT_URL, cURL);
  curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, file);
//  curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1);
}



bool FTPObject::curl_perform()
{
  if (m_pCurl == NULL)
    return false;

  CURLcode ret = curl_easy_perform(m_pCurl);
  if (ret != CURLE_OK)
  {
    fprintf(stderr, "Perform m_pCurl failed!err:%s.\n",curl_easy_strerror(ret));
    return false;
    // exit(1);
  }
  return true;
}

/****************ftp upload & download api******************/
bool FTPObject::FTP_UpFile(string sFilename)
{
  string sLocalFilePath = m_sLocalDirPath + "/" + sFilename;
  string sCurl = m_sCurlBase + m_sFtpDirPath + "/" + sFilename;

  cout << "sLocalFilePath " << sLocalFilePath << endl;
  cout << "sCurl" << sCurl << endl;

  FILE *fp = fopen(sLocalFilePath.c_str(), "rb");
  if (NULL == fp)
  {
    fprintf(stderr, "Open file failed at %s:%d\n", __FILE__, __LINE__);
    // fclose(fp);
    return false;
  }

  if (!curl_init())
  {
    fclose(fp);
    return false;
  }
  curl_set_upload_opt(sCurl.c_str(), fp);
  if (!curl_perform())
  {
    fclose(fp);
    return false;
  }

  curl_exit();
  fclose(fp);

  return true;
}

bool FTPObject::FTP_UpRenameFile(string sFilename, string sRenamed_FileName)
{
  string sLocalFilePath = m_sLocalDirPath + "/" + sFilename;
  string sCurl = m_sCurlBase + m_sFtpDirPath + "/" + sFilename;

  cout << "sLocalFilePath " << sLocalFilePath << endl;
  cout << "sCurl" << sCurl << endl;

  FILE *fp = fopen(sLocalFilePath.c_str(), "rb");
  if (NULL == fp)
  {
    fprintf(stderr, "Open file failed at %s:%d\n", __FILE__, __LINE__);
    // fclose(fp);
    return false;
  }




  if (!curl_init())
  {
    fclose(fp);
    return false;
  }

  struct curl_slist *headerlist = NULL;
  string sBuf1 = "RNFR " + sFilename;
  string sBuf2 = "RNTO " + sRenamed_FileName;
  // string sBuf1 = "RNFR 440500-671153497-1476099582-00271-ST_SOURCE_SJ_0001.bcp.ok";
  // string sBuf2 = "RNTO renamed-and-fine.txt";
  headerlist = curl_slist_append(headerlist, sBuf1.c_str());
  headerlist = curl_slist_append(headerlist, sBuf2.c_str());
  curl_set_upload_rename_opt(sCurl.c_str(), fp, headerlist);


  if (!curl_perform())
  {
    fclose(fp);
    return false;
  }

  curl_exit();
  fclose(fp);

  return true;
}

bool FTPObject::FTP_DwonFile(string sFilename )
{
  string sLocalFilePath = m_sLocalDirPath + "/" + sFilename;
  string sCurl = m_sCurlBase + m_sFtpDirPath + "/" + sFilename;

  cout << "sLocalFilePath " << sLocalFilePath << endl;
  cout << "sCurl" << sCurl << endl;

  //打开，不存在则创建文件
  FILE *fp = fopen(sLocalFilePath.c_str(), "wab");
  if (NULL == fp)
  {
    fprintf(stderr, "Open file failed at %s:%d\n", __FILE__, __LINE__);
    // fclose(fp);
    return false;
  }

  //执行下载文件
  if (!curl_init())
  {
    fclose(fp);
    return false;
  }
  curl_set_download_opt(sCurl.c_str(), fp);
  if (!curl_perform())
  {
    fclose(fp);
    return false;
  }

  curl_exit();
  fclose(fp);

  return true;
}

// int main(int argc, char* argv[])
// {

//   FTPObject fo("127.0.0.1","wlftp:wanglong","/home/wl/wlcode/libcurl-mastr","");


//   if(fo.FTP_UpFile("upload.txt"))
//   {
//     cout << "success" << endl;
//   }
//   else
//   {
//     cout << "failed" << endl;
//   }

//   return 0;
// }