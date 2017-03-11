#include "http.h"

HttpObject::HttpObject(string sIP, string sPort)
{
  m_sIP = sIP;

  //http://localhost:9200/_count?pretty
  m_sCurlBase = "http://" + m_sIP + ":" + sPort + "/" ;

  // //建立连接
  curl_init();

}

HttpObject::~HttpObject()
{
  //断开连接
  if (m_pCurl != NULL)
  {
    curl_exit();
  }
}


/******************类方法****************/
/**
 * 保证单利模式，用类变量作为对象之间通信方式；做到方法线程安全,减少线程里面局部变量的使用
 */
bool HttpObject::m_bInitFlag = false;

/**
 * [HttpObject::Curl_Global_Init description]作为类方法，只会被执行一次啊
 */
void HttpObject::Curl_Global_Init()
{

  if (!HttpObject::m_bInitFlag)
  {
    printf("----------------------**************************************-> Curl_Global_Init \n");
    /**
     * 要求只能执行一次，且在最开始的时候，所以，作为一种类方法，用局部静态变量，封装成只执行一次
     */
    curl_global_init(CURL_GLOBAL_ALL);
    HttpObject::m_bInitFlag = true;
  }

}

/******************m_pCurl api****************/
/**
 * [HttpObject::Curl_Global_Cleanup description]可以不需要条用
 */
void HttpObject::Curl_Global_Cleanup()
{
  curl_global_cleanup();
}

bool HttpObject::curl_init()
{
  // curl_global_init(CURL_GLOBAL_ALL);
  m_pCurl = curl_easy_init();
  if (NULL == m_pCurl)
  {
    fprintf(stderr, "Init m_pCurl failed.\n");
    return false;
  }

  return true;
}

void HttpObject::curl_exit()
{
  curl_easy_cleanup(m_pCurl);
  m_pCurl = NULL;
  // curl_global_cleanup();
}


bool HttpObject::curl_perform()
{
  CURLcode ret = curl_easy_perform(m_pCurl);
  if (ret != CURLE_OK)
  {
    fprintf(stderr, "Perform m_pCurl failed.\n");
    return false;
    // exit(1);
  }
  return true;
}

/****************Http API******************/

/**********写回调函数部分*******/
/**
 * 用来保存HTTP请求值返回的参数
 * I_WRITEDATA_MAX_BUF:最大数
 * cWR_Buf[I_WRITEDATA_MAX_BUF + 1]：缓存数组
 * iWR_Index：多次回调时的保存的偏移量
 */
const int I_WRITEDATA_MAX_BUF = 65535;
char cWR_Buf[I_WRITEDATA_MAX_BUF + 1];
int iWR_Index = 0;

/**
 * [WriteData_CallBack description]写回调函数，用来保存请求返回的数据
 * @param  buffer [description]请求返回数据的buf
 * @param  size   [description]以多少个字节存储，即是存储类型，默认值为1个字节
 * @param  nmemb  [description]存储类型的个数
 * @param  userp  [description]用户传入的缓存地址curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 * @return        [description]返回该类型的个数
 */
size_t WriteData_CallBack( void *buffer, size_t size, size_t nmemb, void *userp )
{
  int segsize = size * nmemb;

  /* Check to see if this data exceeds the size of our buffer. If so,
   * set the user-defined context value and return 0 to indicate a
   * problem to curl.
   */
  if ( iWR_Index + segsize > I_WRITEDATA_MAX_BUF )
  {
    *(int *)userp = 1;
    return 0;
  }

  /* Copy the data from the curl buffer into our buffer */
  memcpy( (void *)&cWR_Buf[iWR_Index], buffer, (size_t)segsize );

  /* Update the write index */
  iWR_Index += segsize;

  /* Null terminate the buffer */
  // printf("Every time iWR_Index :%d\n", iWR_Index);
  cWR_Buf[iWR_Index] = 0;

  /* Return the number of bytes received, indicating to curl that all is okay */
  return segsize;
}



/**
 * [main description]目前测试http协议,GET
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
bool HttpObject::HttpGet(std::string sCmd)
{

  /**
   * 恢复到默认配置
   */
  curl_easy_reset(m_pCurl);

  curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1L);//显示所有的冗余信息
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);//忽略信号
  curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 1);//显示HTTP头部

  //设置url
  string sURL = m_sCurlBase + sCmd ;
  curl_easy_setopt(m_pCurl, CURLOPT_URL, sURL.c_str());
  /* example.com is redirected, so we tell libcurl to follow redirection */
  curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteData_CallBack );//调用写回调


  //执行请求

  if (!curl_perform())
  {
    return false;
  }

  // printf("-----------------------> Times \n");
  // printf("cWR_Buf :\n%s \n", cWR_Buf);

  /**
   * 写到用户缓存中
   */
  m_sReplyCache.clear();
  m_sReplyCache.append(cWR_Buf);

  /**
   * 清空缓存数组
   */
  memset(cWR_Buf, 0, I_WRITEDATA_MAX_BUF + 1);
  iWR_Index = 0;


  return true;
}

/**
 * [main description]目前测试http协议,GET,可添加json参数
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
bool HttpObject::HttpGet(std::string sCmd, std::string strJson)
{

  /**
   * 恢复到默认配置
   */
  curl_easy_reset(m_pCurl);

  curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1L);//显示所有的冗余信息
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);//忽略信号
  curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 1);//显示HTTP头部

  //设置url
  string sURL = m_sCurlBase + sCmd ;
  curl_easy_setopt(m_pCurl, CURLOPT_URL, sURL.c_str());



  // 设置http头部发送的内容类型为JSON
  curl_slist *plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
  curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, plist);

  //设置GET请求
  curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "GET"); /* !!! */

  // 设置要POST的JSON数据
  curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, strJson.c_str());

  /* example.com is redirected, so we tell libcurl to follow redirection */
  curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteData_CallBack );//调用写回调

  //执行请求

  if (!curl_perform())
  {
    return false;
  }

  /**
   * 写到用户缓存中
   */
  m_sReplyCache.clear();
  m_sReplyCache.append(cWR_Buf);

  /**
   * 清空缓存数组
   */
  memset(cWR_Buf, 0, I_WRITEDATA_MAX_BUF + 1);
  iWR_Index = 0;

  return true;
}


bool HttpObject::HttpPost(std::string sCmd, std::string strJson)
{

  // curl_init();
  curl_easy_reset(m_pCurl);

  //curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1L);//显示所有的冗余信息
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);//忽略信号
  curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 1);//显示HTTP头部

  //设置url
  string sURL = m_sCurlBase + sCmd ;
  curl_easy_setopt(m_pCurl, CURLOPT_URL, sURL.c_str());


  // 设置http头部发送的内容类型为JSON
  curl_slist *plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
  curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, plist);
  // 设置要POST的JSON数据
  curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, strJson.c_str());


  //执行请求
  if (!curl_perform())
  {
    return false;
  }

  return true;
}



bool HttpObject::HttpPut(std::string sCmd, std::string strJson)
{

  // curl_init();
  curl_easy_reset(m_pCurl);

  //curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1L);//显示所有的冗余信息
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);//忽略信号
  curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 1);//显示HTTP头部

  //设置url
  string sURL = m_sCurlBase + sCmd ;
  curl_easy_setopt(m_pCurl, CURLOPT_URL, sURL.c_str());
  // 设置http头部发送的内容类型为JSON
  curl_slist *plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
  curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, plist);

  //设置PUT请求
  curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "PUT"); /* !!! */

  // 设置要POST的JSON数据
  curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, strJson.c_str());


  //执行请求
  if (!curl_perform())
  {
    return false;
  }

  return true;
}

bool HttpObject::HttpHead(std::string sCmd)
{

  /**
   * 恢复到默认配置
   */
  curl_easy_reset(m_pCurl);

  //设置url
  string sURL = m_sCurlBase + sCmd ;
  curl_easy_setopt(m_pCurl, CURLOPT_URL, sURL.c_str());

  //curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1L);//显示所有的冗余信息
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);//忽略信号
  curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 1);//显示HTTP头部
  curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "HEAD");

  //执行请求
  if (!curl_perform())
  {
    return false;
  }

  return true;
}

bool HttpObject::HttpDelete(std::string sCmd)
{

  /**
   * 恢复到默认配置
   */
  curl_easy_reset(m_pCurl);

  //设置url
  string sURL = m_sCurlBase + sCmd ;
  curl_easy_setopt(m_pCurl, CURLOPT_URL, sURL.c_str());

  //curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1L);//显示所有的冗余信息
  curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);//忽略信号
  curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 1);//显示HTTP头部
  curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");

  //执行请求
  if (!curl_perform())
  {
    return false;
  }

  return true;
}


/**
 * [HttpPost_urlencoded description]http post 直接url发送 Content-Type: application/x-www-form-urlencoded;的数据
 * @param  sURL     [description]url地址
 * @param  sContext [description]严格的数据格式
 * @return          [description]
 * curl命令实例:curl -d "param1=value1&param2=value2" -X POST http://wechat.huoretao.com/index.php?r=service/place-statistic
 */
bool HttpObject::HttpPost_urlencoded(std::string sURL, std::string sContext)
{
  CURL *curl;
  CURLcode res;
  curl = curl_easy_init();
  if (curl) {
    // struct curl_slist *headers = NULL; /* init to NULL is important */
    // headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded;charset=utf-8");
    // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//显示所有的冗余信息  
    // curl_easy_setopt(curl, CURLOPT_HEADER, 1);//显示HTTP头部
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);//忽略信号

    curl_easy_setopt(curl, CURLOPT_URL, sURL.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);    // 设置 为POST 方法
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sContext.c_str());
    /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
       itself */
    // curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
      return false;
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return true;
}




/****************Test******************/
// int main(int argc, char* argv[])
// {

//   std::string strJson = "{\"project\":\"rapidjson\",\"t\":true,\"stars\":10,\"a\":[0,1,2,3],\"object\":{\"iNum\":2}}";

//   HttpObject fo("127.0.0.1", "9200");

//   // // HttpObject fo1("127.0.0.1", "9200");

//   // fo.HttpPost("index1/type3/1?pretty", strJson);

//   // fo.HttpGet("index1/type3/1?pretty", strJson);

//   // cout << fo.m_sReplyCache << endl;

//   fo.HttpHead("index1/type3/1?pretty");
//   fo.HttpDelete("index1/type3/1?pretty");
//   fo.HttpHead("index1/type3/1?pretty");
//   //
//   // fo.HttpPost("index1/type3/1?pretty", strJson);
//   // fo.HttpHead("index1/type3/1?pretty");

//   return 0;
// }