/* myclient.cpp 
BIF3C1 Josef Koch if18b061
Protokollaufbau von SEND, LIST, READ, DEL, QUIT -> implementiert wie in der Angabe
Es fehlen: LOGIN, LDAP Anbindung, Sperren von Clients bei fehlerhaften Login
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "myhelper/myhelper.h"
#include "mysocket/mysocket.h"

#define BUF 1024

using namespace std;

int main(int argc, char *argv[])
{
   MyHelper helper;
   helper.checkClientArguments(argc, argv);

   MySocket clientSocket(argv[1], stoi(argv[2]));
   clientSocket.createSocket();
   clientSocket.connectSocket();

   char buffer[BUF], command[BUF];

   cout << clientSocket.recvMessage() << endl;

   while (1)
   {
      fgets(command, BUF, stdin);
      buffer[strlen(command)] = '\0';
      clientSocket.sendMessage(command);

      if (strcmp(command, "SEND\n\0") == 0)
      {
         // sender, reveiver, subject
         for (int i = 0; i < 4; i++)
         {
            fgets(buffer, BUF, stdin);
            buffer[strlen(buffer)] = '\0';
            clientSocket.sendMessage(buffer);
         }
         // email message
         do
         {
            fgets(buffer, BUF, stdin);
            if (strncmp(buffer, ".\n", 2) == 0)
            {
               buffer[strlen(buffer)] = '\0';
            }
            clientSocket.sendMessage(buffer);
         } while (strcmp(buffer, ".\n\0") != 0);
         cout << clientSocket.recvMessage() << endl;
      }

      else if (strcmp(command, "LIST\n\0") == 0)
      {
         // user
         fgets(buffer, BUF, stdin);
         buffer[strlen(buffer)] = '\0';
         clientSocket.sendMessage(buffer);

         string mailSubject, numberOfMails;
         numberOfMails = clientSocket.recvMessage();
         cout << numberOfMails;

         int mailAmount = stoi(numberOfMails.substr(0, numberOfMails.find(' ')).c_str());
         if (mailAmount > 0)
         {
            for (int i = 1; i <= mailAmount; i++)
            {
               // tell the server to start sending the list of mails
               clientSocket.sendMessage("OK\0");
               mailSubject = clientSocket.recvMessage();
               mailSubject = mailSubject.substr(mailSubject.find('_') + 1, mailSubject.length()) + ' ' + to_string(i);
               cout << mailSubject << endl;
            }
         }
      }

      else if (strcmp(command, "READ\n\0") == 0)
      {
         // user
         fgets(buffer, BUF, stdin);
         buffer[strlen(buffer)] = '\0';
         clientSocket.sendMessage(buffer);

         // mail number
         fgets(buffer, BUF, stdin);
         buffer[strlen(buffer)] = '\0';
         clientSocket.sendMessage(buffer);

         string mailOutput;
         string resp = clientSocket.recvMessage();
         cout << resp << endl;

         if (resp == "OK")
         {
            while (1)
            {
               // send Server the ok to start receiving the mail
               clientSocket.sendMessage("OK\0");
               mailOutput = clientSocket.recvMessage();
               if (mailOutput == ".\n")
               {
                  break;
               }
               cout << mailOutput;
            }
         }
      }

      else if (strcmp(command, "DEL\n\0") == 0)
      {
         // user
         fgets(buffer, BUF, stdin);
         buffer[strlen(buffer)] = '\0';
         clientSocket.sendMessage(buffer);

         // mail number
         fgets(buffer, BUF, stdin);
         buffer[strlen(buffer)] = '\0';
         clientSocket.sendMessage(buffer);

         cout << clientSocket.recvMessage() << endl;
      }

      else if (strcmp(command, "QUIT\n\0") == 0)
      {
         break;
      }

      else
      {
         cout << "Unknown command!" << endl;
      }
   }
   return EXIT_SUCCESS;
}
