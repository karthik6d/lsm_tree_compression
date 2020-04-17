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

int read_data_to_LSM(component* curr_comp, int valuesin){
  //Read data from the file
  FILE *fp;

  if(valuesin >= curr_comp->element_count){
    int breakval = curr_comp-> element_count + 1;
    return breakval;
  }
  fp = fopen(curr_comp->filename.c_str(), "r");
  fseek (fp , 4*valuesin , SEEK_SET);
  //struct stat st;
  //size_t col_size=0;
  //Get file size of the col_data

  //if (stat(curr_comp->filename.c_str(), &st) == 0){
    //col_size = st.st_size;
  //}


  //int number_members = (col_size / sizeof(int));
  int data_ptr[DEFAULT_BUFFER_SIZE];
  size_t nmemb = fread(data_ptr, 4, DEFAULT_BUFFER_SIZE, fp);
  fclose(fp);
  vector<int> *data = new vector<int>;

  for(int i = 0; i < (int)nmemb; i++){
    data->push_back(data_ptr[i]);
  }
  curr_comp->values = data;
  int total_elements_read = valuesin + (int)nmemb;
  return total_elements_read;
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
  int intswritten = 0
  int val;
  string line;
  FILE *fp;
  int data[DEFAULT_BUFFER_SIZE];
  int componentsize = 0;

  while(getline(myFile, line)){
    stringstream ss(line);

    if (intswritten == 0){
      componentsize = (int)pow(T, level) * DEFAULT_BUFFER_SIZE;
      string file_name("C");
      file_name.append(to_string(component_number));
      file_name.append(".dat");

      component* new_comp = (component*)malloc(sizeof(component));
      new_comp->values = NULL;
      new_comp->filename = file_name;
      new_comp->next_component = NULL;
      new_comp->level = level;
      new_comp->component_number = component_number;

      //Add new_comp to vector
      components.push_back(new_comp);

      //Increment Variables
      level_count += 1;
      component_number += 1;
    }

    //Decide when to push the component to disk
    if(count == DEFAULT_BUFFER_SIZE){
      count = 0;
      //cout << "Writing File" << count << "\n";
      fp = fopen(file_name.c_str(), "a");
      fwrite(data, sizeof(int), sizeof(data)/sizeof(int), fp);
      fclose(fp);

      if(intswritten == componentsize){
        new_comp->element_count = intswritten;
        intswritten == 0;
        // call sort on that file: sort(file_name)

        //Decide when to increment the level based on ratio T
        if(level_count == T){
          level += 1;
          level_count = 0;
        }
      }


    }



    // Extract each integer
    while(ss >> val){
        data[count] = val;
        count += 1;
        intswritten += 1;
        // If the next token is a comma, ignore it and move on
        if(ss.peek() == ',') ss.ignore();
    }
  }

  if(count != 0){
    new_comp->element_count = intswritten;
    fp = fopen(file_name.c_str(), "a");
    fwrite(data, sizeof(int), sizeof(data)/sizeof(int), fp);
    fclose(fp);
  }
}

int read(int key){
  for(int i = 0; i < components.size();i++){
    component* new_comp = components[i];
    int values_in = read_data_to_LSM(new_comp,0);

    while(values_in <= new_comp->element_count){
      vector<int> *data = new_comp->values;

      int value = binarySearch(data,0,data.size();key);

      if (value != -1){
        return value
      }

      values_in = read_data_to_LSM(new_comp,values_in);


    }

  }

  return -1;
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


int binarySearch(vector<int>*data, int l, int r, int x)
{
    if (r >= l) {
        int mid = l + (r - l) / 2;
        if(mid%2 != 0){
          mid = mid - 1;
        }

        // If the element is present at the middle
        // itself
        if (data[mid] == x)
            return data[mid+1];

        // If element is smaller than mid, then
        // it can only be present in left subarray
        if (data[mid] > x)
            return binarySearch(data, l, mid-1, x);

        // Else the element can only be present
        // in right subarray
        return binarySearch(data, mid+1, r, x);
    }

    // We reach here when element is not
    // present in array
    return -1;
}
