#include "myhelper.h"

MyHelper::MyHelper() {}

void MyHelper::checkServerArguments(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " <port> <directory>" << endl;
        exit(EXIT_FAILURE);
    }

    if (!stringIsInt(argv[1]) || stoi(argv[1]) < 1024 || stoi(argv[1]) > 65535)
    {
        cout << "Invalid port" << endl;
        exit(EXIT_FAILURE);
    }

    if (opendir(argv[2]) == NULL)
    {
        perror("directory error");
        exit(EXIT_FAILURE);
    }
}

void MyHelper::checkClientArguments(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " <address> <port>" << endl;
        exit(EXIT_FAILURE);
    }

    if (!stringIsInt(argv[2]) || stoi(argv[2]) < 1024 || stoi(argv[2]) > 65535)
    {
        cout << "Invalid Port" << endl;
        exit(EXIT_FAILURE);
    }
}

bool MyHelper::stringIsInt(string string)
{
    try
    {
        stoi(string);
    }
    catch (invalid_argument &msg)
    {
        return false;
    }
    catch (out_of_range &msg)
    {
        return false;
    }
    return true;
}

int MyHelper::filesInDirectory(string fullDir)
{
    int amountOfFiles = 0;
    dir = opendir(fullDir.c_str());

    while ((dirent = readdir(dir)) != NULL)
    {
        buffer = dirent->d_name;
        if (buffer != "." && buffer != "..")
        {
            amountOfFiles++;
        }
    }
    closedir(dir);
    return amountOfFiles;
}

vector<string> MyHelper::filenamesInDirecotry(string fullDir)
{
    vector<string> files;
    dir = opendir(fullDir.c_str());

    while ((dirent = readdir(dir)) != NULL)
    {
        buffer = dirent->d_name;
        if (buffer != "." && buffer != "..")
        {
            files.push_back(buffer);
        }
    }
    closedir(dir);
    return files;
}

vector<string> MyHelper::subjectsInDirectory(string fullDir)
{
    vector<string> files;
    dir = opendir(fullDir.c_str());

    while ((dirent = readdir(dir)) != NULL)
    {
        buffer = dirent->d_name;
        if (buffer != "." && buffer != "..")
        {
            buffer = buffer.substr(0, buffer.length() - 4);
            files.push_back(buffer);
        }
    }
    closedir(dir);
    return files;
}

bool MyHelper::fileExists(string filename)
{
    struct stat buf;
    if(stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}
MyHelper::~MyHelper() {}