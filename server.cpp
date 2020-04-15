#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdio>
#include <sstream>
#include <cmath>
#include <sys/stat.h>
#include <tuple>
#include "server.h"

using namespace std;
LSM_Tree* current_db;
vector <component*> components;

void read_data_to_LSM(component* curr_comp){
  //Read data from the file
  FILE *fp;
  fp = fopen(curr_comp->filename.c_str(), "r");
  struct stat st;
  size_t col_size=0;
  //Get file size of the col_data
  if (stat(curr_comp->filename.c_str(), &st) == 0){
    col_size = st.st_size;
  }
  int number_members = (col_size / sizeof(int));
  int data_ptr[number_members];
  size_t nmemb = fread(data_ptr, 4, number_members, fp);
  vector<int> *data = new vector<int>;
  
  for(int i = 0; i < (int)nmemb; i++){
    data->push_back(data_ptr[i]);
  }
  curr_comp->values = data;
}

void create(string db_name){
  LSM_Tree* db = (LSM_Tree*)malloc(sizeof(LSM_Tree));
  component* first_comp = (component*)malloc(sizeof(component));
  
  db->name = db_name;
  db->C0 = first_comp;
  
  current_db = db;
}

void load(string path){
  //Read data from CSV File
  ifstream myFile(path);
  //Check to see if the file exists
  if(!myFile.is_open()) throw std::runtime_error("Could not open file");
  
  //Loop through the data
  int component_number = 0;
  int level = 0;
  int count = 0;
  int level_count = 0;
  int val;
  string line;
  FILE *fp;
  int data[DEFAULT_BUFFER_SIZE*2];
  
  while(getline(myFile, line)){
    stringstream ss(line);
  
    //Decide when to push the component to disk
    if(count % (DEFAULT_BUFFER_SIZE*2) == 0 && count != 0){
      string file_name("C");
      file_name.append(to_string(component_number));
      file_name.append(".dat");
  
      //cout << "Writing File" << count << "\n";
      fp = fopen(file_name.c_str(), "w");
      fwrite(data, sizeof(int), sizeof(data)/sizeof(int), fp);
      fclose(fp);
  
      component* new_comp = (component*)malloc(sizeof(component));
      new_comp->values = NULL;
      new_comp->filename = file_name;
      new_comp->next_component = NULL;
      new_comp->level = level;
      new_comp->component_number = component_number;
  
      //Add new_comp to vector
      components.push_back(new_comp);
  
      //Increment Variabless
      level_count += 1;
      component_number += 1;
    }
  
    //Decide when to increment the level based on ratio T
    if(level_count == (int)pow(T, level)){
      level += 1;
      level_count = 0;
    }
  
    // Extract each integer
    while(ss >> val){
        int index = count % (DEFAULT_BUFFER_SIZE*2);
        data[index] = val;
        count += 1;
        // If the next token is a comma, ignore it and move on
        if(ss.peek() == ',') ss.ignore();
    }
  }
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
  char input[1024];
  char* output_str = NULL;
  
  //Parse through workload file
  while((output_str = fgets(input, 1024, stdin))){
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