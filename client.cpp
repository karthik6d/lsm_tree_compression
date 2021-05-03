#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
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
#include <unordered_map>
#include <string> 
#include <stdexcept>
#include <sys/un.h>
#define PORT 8080
#define DEFAULT_STDIN_BUFFER_SIZE 1024
#ifndef SOCK_PATH
#define SOCK_PATH "cs165_unix_socket"
#endif

using namespace std;

typedef enum message_status {
    OK_DONE,
    OK_WAIT_FOR_RESPONSE,
    UNKNOWN_COMMAND
} message_status;


typedef struct message {
    message_status status;
    int length;
    std::string payload;
} message;


void* send_message(int client_socket, string payload) {
    send(client_socket, payload.c_str(), payload.length(), 0);
    cout << "Payload: " << payload << endl;
    char* buffer = (char*)malloc(DEFAULT_STDIN_BUFFER_SIZE);
    read(client_socket, buffer, DEFAULT_STDIN_BUFFER_SIZE);
    
    return buffer;
}


void load(string path, int client_socket) {
  // Read data from CSV File
  ifstream data_file(path, ios::ate);

  int length = data_file.tellg();
  data_file.close();

  FILE* f = fopen(path.c_str(), "r");

  char* data =
      (char*)mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fileno(f), 0);
  fclose(f);

  string data_str(data, data + length);

  if(munmap(data, length) != 0){
    cout << strerror(errno) << endl;
  }

  cout << "reading csv" << endl;

  istringstream ss(data_str);

  string line;

  while (getline(ss, line)) {
    // cout << "\r" << ++i << "/" << lines << flush;

    istringstream ss(line);

    string key_s, value_s;

    getline(ss, key_s, ',');
    getline(ss, value_s, ',');

    string payload = "write," + key_s + "," + value_s;
    char* output = (char*) send_message(client_socket, payload);
    // string output_message(output);
    // cout << "Output Write: " << output_message << endl;
  }
}

int connect_client() 
{
    int client_socket;
    size_t len;
    struct sockaddr_un remote;

    if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("L%d: Failed to create socket.\n", __LINE__);
        return -1;
    }

    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCK_PATH, strlen(SOCK_PATH) + 1);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family) + 1;
    if (connect(client_socket, (struct sockaddr *)&remote, len) == -1) {
        printf("client connect failed: ");
        return -1;
    }


    return client_socket;
}

int main(int argc, char const *argv[])
{
    int client_socket = connect_client();
    string query_file = "queries.dsl";
    ifstream query_file_ss(query_file);
    string output_str;
    vector<int> result;
    
    // Parse through workload file
    while (getline(query_file_ss, output_str)) {
        istringstream ss(output_str);
        vector<string> elements;
        string payload;

        do {
            string word;
            ss >> word;
            elements.push_back(word);
        } while (ss);

        if (elements[0].compare("create") == 0) {
            payload = "create";
            char* output = (char*) send_message(client_socket, payload);
            // string output_message(output);
            // cout << "Output Message: " << output_message << endl;
        } else if (elements[0].compare("load") == 0) {
            load("data.csv", client_socket);
        } else if (elements[0].compare("read") == 0) {
            payload = "read," + elements[1];
            send_message(client_socket, payload);
            char* output = (char*) send_message(client_socket, payload);
            string output_message(output);
            //cout << "Output Message: " << output_message << endl;
            result.push_back(stoi(output_message));

        } else if (elements[0].compare("write") == 0) {
            payload = "write," + elements[1] + "," + elements[2];
            send_message(client_socket, payload);
            char* output = (char*) send_message(client_socket, payload);
            // string output_message(output);
            // cout << "Output Message: " << output_message << endl;

        } else if (elements[0].compare("delete") == 0) {
            payload = "delete," + elements[1];
            send_message(client_socket, payload);

        } else if (elements[0].compare("update") == 0) {
            payload = "update," + elements[1] + "," + elements[2];
            send_message(client_socket, payload);

        } else {
            throw runtime_error("Unknown command");
        }
    }

    cout << "finished workload, writing results to file" << endl;

    ofstream f("hello.res");

    for (int r : result) {
        f << r << endl;
    }

    f.close();

    return 0;
}
