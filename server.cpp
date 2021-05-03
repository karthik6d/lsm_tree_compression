#include "server.h"
#include "lsm_tree.h"

#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <arpa/inet.h>
#define PORT 8080
#define DEFAULT_STDIN_BUFFER_SIZE 2048
#ifndef SOCK_PATH
#define SOCK_PATH "cs165_unix_socket"
#endif

using namespace std;

vector<workload_entry> workload;

void create(string db_name) {
  LSM_Tree* db = new LSM_Tree();
  db->name = db_name;
  current_db = db;

  mkdir("data", 0755);
}

int connect_server_client(string ip, int port_number) 
{
    int client_socket;
    size_t len;
    struct sockaddr_in remote;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("L%d: Failed to create socket.\n", __LINE__);
        return -1;
    }

    remote.sin_family=AF_INET;
    remote.sin_port=htons(port_number);
    remote.sin_addr.s_addr= inet_addr(ip.c_str());
 
    cout << "Before connecting" << endl;
    if (connect(client_socket, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
        printf("client connect failed: ");
        printf("Value of errno: %d\n", errno);
        return -1;
    }
    cout << "After connecting" << endl;

    return client_socket;
}

void handle_distributed_writes(string ip, int port_number, string message) {
  int client_socket = connect_server_client(ip, port_number);
  send(client_socket, message.c_str(), message.length(), 0);
  char* buffer = (char*)malloc(DEFAULT_STDIN_BUFFER_SIZE);
  read(client_socket, buffer, DEFAULT_STDIN_BUFFER_SIZE);
}

int setup_server() {
  int server_socket;
  size_t len;
  struct sockaddr_in server_address;

  printf("Attempting to setup server...\n");

  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      printf("L%d: Failed to create socket.\n", __LINE__);
      return -1;
  }

  server_address.sin_family=AF_INET;
  server_address.sin_port=htons(PORT);
  server_address.sin_addr.s_addr=INADDR_ANY;

  int binder = ::bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
  if (binder == -1) {
      printf("L%d: Socket failed to bind.\n", __LINE__);
      return -1;
  }

  if (listen(server_socket, 5) == -1) {
      printf("L%d: Failed to listen on socket.\n", __LINE__);
      return -1;
  }

  return server_socket;
}

void handle_client(int client_socket) {
  int done = 0;
  int length = 0;
  char* buf = (char*)malloc(1024);

  do {
    length = recv(client_socket, buf, (size_t) 1024, 0);
    char actual_buf[(int)length + 1];
    strncpy(actual_buf, buf, length);
    actual_buf[(int)length] = 0;
    string message(actual_buf);

    if (length < 0) {
      printf("Client connection close!\n");
    }
    else if(length == 0) {
      done = 1;
    }

    if (!done) {
      if(message == "create") {
        create("name");
        string send_message = "OK";
        if (send(client_socket, (void*) send_message.c_str(), send_message.length(), 0) == -1) {
          printf("Failed to send message.");
          exit(1);
        }
      }

      vector<string> result;
      string message_copy(message);
      stringstream s_stream(message); //create string stream from the string
      while(s_stream.good()) {
        string substr;
        getline(s_stream, substr, ','); //get first string delimited by comma
        result.push_back(substr);
      }

      if(result.at(0) == "read") {
        pair<read_result, int> r = current_db->read(stoi(result.at(1)));
        string send_message = to_string(r.second);
        if (send(client_socket, (void*) send_message.c_str(), send_message.length(), 0) == -1) {
          printf("Failed to send message.");
          exit(1);
        }
      }

      if(result.at(0) == "write") {
        current_db->write(stoi(result.at(1)), stoi(result.at(2)));
        // We use multi-threading here to 
        // 1) Create a new client socket on this machine
        // 2) Use that client socket to send the write message to the other machines (2)
        // 3) These should be multi-threaded and I await for both "OK" Messages from the other servers
        // 4) Now all these are in sync, and we can tell the client we are ready for more writes
        int port1 = 10001;
        int port2 = 10002;
        string ip1 = 'idk';
        string ip2 = 'idk1';
        thread th1(handle_distributed_writes, ip1, port1, message_copy);
        thread th2(handle_distributed_writes, ip2, port2, message_copy);
        th1.join()
        th2.join()

        string send_message = "OK";
        if (send(client_socket, (void*) send_message.c_str(), send_message.length(), 0) == -1) {
          printf("Failed to send message.");
          exit(1);
        }
      }
      free(buf);
      buf = (char*)malloc(1024);
    }
  } while (!done);

  close(client_socket);
}

int main(int argc, char** argv) {
  int server_socket = setup_server();
  if (server_socket < 0 ) { 
    exit(1);
  }

  printf("Waiting for a connection %d ...\n", server_socket);

  struct sockaddr_un remote;
  socklen_t t = sizeof(remote);
  int client_socket = 0;

  if ((client_socket = accept(server_socket, (struct sockaddr *)&remote, &t)) == -1) {
      printf("L%d: Failed to accept a new connection.\n", __LINE__);
      exit(1);
  }

  handle_client(client_socket);
  return 0;
}
