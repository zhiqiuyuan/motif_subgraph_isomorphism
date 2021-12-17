#include <iostream>
#include <string>
#include <set>
#include <vector>

#include <io.h>

#include <fstream>
#include <ctime>
#include <fcntl.h>
using namespace std;

void getFileNames(string path, vector<string> &files);
void getFileNames_notRec(string path, vector<string> &files);
void getFileNames_withoutPrefix(string path, vector<string> &files);
void getFileNames_withoutPrefix_notRec(string path, vector<string> &files);

//1
//输出文件名
int main(int argc, char **argv)
{
    string path = argv[1];
    vector<string> fnames;
    //getFileNames(path,fnames);
    getFileNames_withoutPrefix_notRec(path, fnames);
    for (auto name : fnames)
    {
        cout << name << endl;
    }
    return 0;
}

//path:开始查找的目录
void getFileNames(string path, vector<string> &files)
{
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            //如果是目录,递归查找
            //如果不是,把文件绝对路径存入vector中
            if ((fileinfo.attrib & _A_SUBDIR))
            {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    getFileNames(p.assign(path).append("\\").append(fileinfo.name), files);
            }
            else
            {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

void getFileNames_notRec(string path, vector<string> &files)
{
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            files.push_back(p.assign(path).append("\\").append(fileinfo.name));
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

void getFileNames_withoutPrefix(string path, vector<string> &files)
{
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            //如果是目录,递归查找
            //如果不是,把文件绝对路径存入vector中
            if ((fileinfo.attrib & _A_SUBDIR))
            {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    getFileNames(p.assign(fileinfo.name), files);
            }
            else
            {
                files.push_back(p.assign(fileinfo.name));
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

//不递归查找下级目录
void getFileNames_withoutPrefix_notRec(string path, vector<string> &files)
{
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            files.push_back(p.assign(fileinfo.name));
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}
