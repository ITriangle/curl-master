#include "copy.h"

CopyObject::CopyObject()
{

}

CopyObject::CopyObject(string sSourceDirPath, string sDestDirPath)
{
  m_sSourceDirPath = sSourceDirPath;//源目录
  m_sDestDirPath = sDestDirPath;//目的目录
}

CopyObject::~CopyObject()
{
  printf("CopyObject:\n");

}


/*复制函数*/
bool CopyObject::CopyFile(const char* source_path, const char *destination_path)//复制文件
{
  char buffer[1024];
  //定义两个文件流，分别用于文件的读取和写入int len;
  FILE *in, *out;
  if ((in = fopen(source_path, "rb")) == NULL) //打开源文件的文件流
  {
    printf("源文件打开失败！\n");
    return false;
  }
  if ((out = fopen(destination_path, "wb")) == NULL) //打开目标文件的文件流
  {
    fclose(in);
    printf("目标文件创建失败！\n");
    return false;
  }
  int len;//len为fread读到的字节长
  while ((len = fread(buffer, 1, 1024, in)) > 0)
  {
    //从源文件中读取数据并放到缓冲区中，第二个参数1也可以写成sizeof(char)
    fwrite(buffer, 1, len, out); //将缓冲区的数据写到目标文件中
  }

  //关闭文件流
  fclose(out);
  fclose(in);

  return true;
}

bool CopyObject::LocalCopyFile(string strFileName)
{
  string strFileOldPathName = m_sSourceDirPath + std::string("/") + strFileName;
  string strFileNewPathName = m_sDestDirPath + std::string("/") + strFileName;

  // printf("---------------------->strFileOldPathName %s!", strFileOldPathName.c_str());
  // printf("---------------------->strFileNewPathName %s!", strFileNewPathName.c_str());

  if (CopyFile(strFileOldPathName.c_str(), strFileNewPathName.c_str()))
  {
    return true;
  }
  else
  {
    return false;
  }


}

bool CopyObject::LocalCopyFile(string strFileName, string sRenamed_FileName)
{
  string strFileOldPathName = m_sSourceDirPath + std::string("/") + strFileName;
  string strFileNewPathName = m_sDestDirPath + std::string("/") + strFileName;

  // printf("---------------------->strFileOldPathName %s!", strFileOldPathName.c_str());
  // printf("---------------------->strFileNewPathName %s!", strFileNewPathName.c_str());

  if (CopyFile(strFileOldPathName.c_str(), strFileNewPathName.c_str()))
  {
    /**
     * 拷贝完成后,就将拷贝好的文件命名为指定的文件名
     */
    string strFileNewFileName = m_sDestDirPath + std::string("/") + sRenamed_FileName;
    rename(strFileNewPathName.c_str(),strFileNewFileName.c_str());

    return true;
  }
  else
  {
    return false;
  }


}





// int main(int argc, char* argv[])
// {

//   return 0;
// }