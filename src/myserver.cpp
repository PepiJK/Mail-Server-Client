/* myserver.cpp 
BIF3C1 Josef Koch if18b061
Protokollaufbau von SEND, LIST, READ, DEL, QUIT -> implementiert wie in der Angabe
Es fehlen: LOGIN, LDAP Anbindung, Sperren von Clients bei fehlerhaften Login
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <mutex>
#include "myhelper/myhelper.h"
#include "mysocket/mysocket.h"

using namespace std;

mutex mtx;

void serverThread(MyHelper helper, MySocket &serverSocket, int new_socket, string argDir)
{
   string command, buffer;
   do
   {
      command = serverSocket.recvMessage(new_socket);

      if (command == "SEND\n")
      {
         string sender = serverSocket.recvMessage(new_socket);
         string receiver = serverSocket.recvMessage(new_socket);
         string subject = serverSocket.recvMessage(new_socket);
         string message = serverSocket.recvMessage(new_socket);

         // read message part of mail till .\n occours
         while (1)
         {
            buffer = serverSocket.recvMessage(new_socket);
            if (buffer == ".\n")
            {
               break;
            }
            message += buffer;
         }

         // check for length violations, +1 because of \n
         if (sender.length() > 9 || receiver.length() > 9 || subject.length() > 81)
         {
            serverSocket.sendMessage("ERR - sender or reveiver or subject length is too long\0", new_socket);
         }
         else
         {
            // write file with data in directory of user who reveivces the mail
            string fullDir = argDir + '/' + receiver.substr(0, receiver.length() - 1);
            int mailId;

            if (access(fullDir.c_str(), F_OK) == -1)
            {
               mkdir(fullDir.c_str(), 0700);
               cout << "created directory " << fullDir << endl;
            }

            // critical section because filename gets created and saved on disk
            // could otherwise cause one file overwriting the other
            mtx.lock();

            // get number of files in directory, + 1 for new index
            mailId = helper.filesInDirectory(fullDir) + 1;
            string filename = fullDir + '/' + to_string(mailId) + '_' + subject.substr(0, subject.length() - 1) + ".txt";

            // check if filename exists -> increment index till filename is unique
            // it can happend that the filename with the same index and subjects exists when a file before gets deleted
            // solution would be the use of a UUID, however this is not implemented in standard c+++
            // so this rather "ugly" solution is used
            while (1)
            {
               if (helper.fileExists(filename))
               {
                  mailId++;
                  filename = fullDir + '/' + to_string(mailId) + '_' + subject.substr(0, subject.length() - 1) + ".txt";
               }
               else
               {
                  break;
               }
            }

            // create file, write mail data to file
            ofstream file;
            file.open(filename.c_str(), ios::out);
            file << sender + receiver + subject + message + ".\n\0";
            cout << "Mail " << subject.substr(0, subject.length() - 1) << " from user "
                 << sender.substr(0, sender.length() - 1) << " safed to " << filename << endl;
            serverSocket.sendMessage("OK\0", new_socket);
            file.close();
            mtx.unlock();
         }
      }

      else if (command == "LIST\n")
      {
         string user = serverSocket.recvMessage(new_socket);
         string dirPath = argDir + '/' + user.substr(0, user.length() - 1);

         // check if user exists
         if (access(dirPath.c_str(), F_OK) == -1)
         {
            buffer = "0 mails for user " + user;
            serverSocket.sendMessage(buffer.c_str(), new_socket);
         }
         else
         {
            vector<string> mails;

            // get every mail subject from directory and send to client
            mails = helper.subjectsInDirectory(dirPath);
            buffer = to_string(mails.size()) + " mails for user " + user;
            serverSocket.sendMessage(buffer.c_str(), new_socket);

            // at least 1 message lies in direcotry -> send list of messages to client
            if (mails.size() > 0)
            {
               cout << "Sent list of mails from user " << user.substr(0, user.length() - 1) << " to client" << endl;
               for (unsigned int i = 0; i < mails.size(); i++)
               {
                  // check if client is ready to recv message, without this check weird things happen
                  buffer = serverSocket.recvMessage(new_socket);
                  if (buffer == "OK")
                  {
                     serverSocket.sendMessage(mails[i].c_str(), new_socket);
                  }
               }
            }
         }
      }

      else if (command == "READ\n")
      {
         string user = serverSocket.recvMessage(new_socket);
         string mailNumber = serverSocket.recvMessage(new_socket);
         ofstream file;

         user = user.substr(0, user.length() - 1);

         if (helper.stringIsInt(mailNumber))
         {
            unsigned int mailIndex = stoi(mailNumber.substr(0, mailNumber.length() - 1)) - 1;
            string line, fulldir, dirPath = argDir + '/' + user;
            ifstream file;
            vector<string> mails;

            // check if user exists
            if (access(dirPath.c_str(), F_OK) == -1)
            {
               serverSocket.sendMessage("ERR - user does not exist\0", new_socket);
            }
            else
            {
               // get every file from directory
               mails = helper.filenamesInDirecotry(dirPath);

               // check if requested mail actually exists
               if (mailIndex < mails.size())
               {
                  fulldir = dirPath + '/' + mails[mailIndex];
                  mtx.lock();
                  file.open(fulldir.c_str());
                  if (file.is_open())
                  {
                     cout << "Sent email " << mails[mailIndex] << " informaiton from user " << user << " to client" << endl;
                     serverSocket.sendMessage("OK\0", new_socket);
                     while (getline(file, line))
                     {
                        line = line + '\n';
                        // check if client is ready to recv message, without this check weird things happen
                        buffer = serverSocket.recvMessage(new_socket);
                        if (buffer == "OK")
                        {
                           serverSocket.sendMessage(line.c_str(), new_socket);
                        }
                     }
                  }
                  else
                  {
                     serverSocket.sendMessage("ERR - cannot open mail file\0", new_socket);
                  }
                  file.close();
                  mtx.unlock();
               }
               else
               {
                  serverSocket.sendMessage("ERR - mail does not exist\0", new_socket);
               }
            }
         }
         else
         {
            serverSocket.sendMessage("ERR - mail number is not an integer\0", new_socket);
         }
      }

      else if (command == "DEL\n")
      {
         string user = serverSocket.recvMessage(new_socket);
         string mailNumber = serverSocket.recvMessage(new_socket);
         ofstream file;

         user = user.substr(0, user.length() - 1);

         if (helper.stringIsInt(mailNumber))
         {
            unsigned int mailIndex = stoi(mailNumber.substr(0, mailNumber.length() - 1)) - 1;
            string fulldir, dirPath = argDir + '/' + user;
            ifstream file;
            vector<string> mails;

            // check if user exists
            if (access(dirPath.c_str(), F_OK) == -1)
            {
               serverSocket.sendMessage("ERR - user does not exist\0", new_socket);
            }
            else
            {
               // get every file from directory
               mails = helper.filenamesInDirecotry(dirPath);

               // check if requested mail actually exists
               if (mailIndex < mails.size())
               {
                  fulldir = dirPath + '/' + mails[mailIndex];
                  mtx.lock();
                  if (remove(fulldir.c_str()) == 0)
                  {
                     cout << "Deleted mail " + mails[mailIndex] << " from user " << user << endl;
                     serverSocket.sendMessage("OK\0", new_socket);
                  }
                  else
                  {
                     serverSocket.sendMessage("ERR - cannot delete mail\0", new_socket);
                  }
                  mtx.unlock();
               }
               else
               {
                  serverSocket.sendMessage("ERR - mail does not exist\0", new_socket);
               }
            }
         }
         else
         {
            serverSocket.sendMessage("ERR - mail number is not an integer\0", new_socket);
         }
      }

   } while (command != "QUIT\n");
   cout << "Client disconnected" << endl;
   close(new_socket);
}

int main(int argc, char *argv[])
{
   MyHelper helper;
   helper.checkServerArguments(argc, argv);

   MySocket serverSocket(stoi(argv[1]));
   serverSocket.createSocket();
   serverSocket.bindSocket();
   serverSocket.listenSocket();

   int new_socket;

   cout << "Waiting for connections..." << endl;
   while (1)
   {
      new_socket = serverSocket.acceptNewConnection();
      // create a new thread everytime a new client connects to the server
      // detach because we allow the execution of the thread to continue independently
      thread(serverThread, helper, ref(serverSocket), new_socket, argv[2]).detach();
   }
   return EXIT_SUCCESS;
}
