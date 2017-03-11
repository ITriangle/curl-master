#ifndef _LOCAL_RENAME_H_
#define _LOCAL_RENAME_H_

#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>//输出文件信息  
#include <sys/stat.h>//判断是否目录


using namespace std;

class CopyObject
{
public:
    CopyObject();
    CopyObject(string sSourceDirPath, string sDestDirPath);
    ~CopyObject();

    bool LocalCopyFile(string strFileName);

    bool LocalCopyFile(string strFileName, string sRenamed_FileName);
private:

    bool CopyFile(const char* source_path, const char *destination_path);//复制文件

    string m_sSourceDirPath;//源目录
    string m_sDestDirPath;//目的目录

};


#endif