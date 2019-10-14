#ifndef myhelper_H
#define myhelper_H

#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>

using namespace std;

class MyHelper
{

private:
    struct dirent *dirent;
    DIR *dir;
    string buffer;

public:
    MyHelper();
    void checkServerArguments(int argc, char *argv[]);
    void checkClientArguments(int argc, char *argv[]);
    bool stringIsInt(string string);
    int filesInDirectory(string dir);
    vector<string> filenamesInDirecotry(string dir);
    vector<string> subjectsInDirectory(string dir);
    bool fileExists(string filename);
    ~MyHelper();
};
#endif