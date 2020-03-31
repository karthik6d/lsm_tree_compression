#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdio>
#include <sstream>

#define DEFAULT_STDIN_BUFFER_SIZE 1024

using namespace std;

void create(string db_name){
  cout << db_name << "\n";
}

void load(string path){
  cout << path << "\n";
}

void read(int key){
  cout << key << "\n";
}

void write(int key, int value){
  cout << key << "\t" << value << "\n";
}

void del(int key){
  cout << "in delete\n";
  cout << key << "\n";
}

void update(int key, int update_value){
  cout << key << update_value << "\n";
}


int main(int argc,  char **argv)
{
  char input[DEFAULT_STDIN_BUFFER_SIZE];
  char* output_str = NULL;
  
  while((output_str = fgets(input, DEFAULT_STDIN_BUFFER_SIZE, stdin))){
    string curr = output_str;
    istringstream ss(curr);
    vector<string> elements;
    
    do { 
      string word; 
      ss >> word; 
      elements.push_back(word);
      
    } while (ss);
    
    if(elements.at(0).compare("create") == 0){
      create(elements.at(1));
    }
    else if(elements.at(0).compare("load") == 0){
      load(elements.at(1));
    }
    else if(elements.at(0).compare("read") == 0){
      read(stoi(elements.at(1)));
    }
    else if(elements.at(0).compare("write") == 0){
      write(stoi(elements.at(1)), stoi(elements.at(2)));
    }
    else if(elements.at(0).compare("delete") == 0){
      del(stoi(elements.at(1)));
    }
    else if(elements.at(0).compare("update") == 0){
      update(stoi(elements.at(1)), stoi(elements.at(2)));
    }
    
  }
  
  return 0;
}