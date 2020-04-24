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
int buffer[DEFAULT_BUFFER_SIZE];
int buffer_count;
int component_number;


int read_data_to_LSM(component* curr_comp, int valuesin){
  //Read data from the file
  FILE *fp;

  // valuesin reports how many values of component already read
  // check if the whole component been read
  if(valuesin >= curr_comp->element_count){
    int breakval = curr_comp-> element_count + 1;
    return breakval;
  }

  // open file and set start ptr to values in times 4 bytes per val
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

  // read as many values fit in main memory
  size_t nmemb = fread(data_ptr, 4, DEFAULT_BUFFER_SIZE, fp);
  fclose(fp);
  vector<int> *data = new vector<int>;

  // add to data vector
  for(int i = 0; i < (int)nmemb; i++){
    data->push_back(data_ptr[i]);
  }
  curr_comp->values = data;
  int total_elements_read = valuesin + (int)nmemb;

  // return how many elements of component have been read
  return total_elements_read;
}

void create(string db_name){
  LSM_Tree* db = (LSM_Tree*)malloc(sizeof(LSM_Tree));
  component* first_comp = (component*)malloc(sizeof(component));

  db->name = db_name;
  db->C0 = first_comp;

  current_db = db;
  buffer_count = 0;
  component_number = 0;

}

void load(string path){
  //Read data from CSV File
  ifstream myFile(path);
  //Check to see if the file exists
  if(!myFile.is_open()) throw std::runtime_error("Could not open file");

  //Loop through the data
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


    // intswritten keeps track of how many ints written to component
    // create new component every times its reset to 0
    if (intswritten == 0){
      //size of component depends on level, main mem size, and level ratio
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

    //Decide when to push the read vector to disk, once main mem full
    if(count == DEFAULT_BUFFER_SIZE){
      count = 0;
      //cout << "Writing File" << count << "\n";
      fp = fopen(file_name.c_str(), "a");
      fwrite(data, sizeof(int), sizeof(data)/sizeof(int), fp);
      fclose(fp);

      // check if component has been filled and then sort it
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
  int neg_found = 0;
  // iterate through components
  for(int i = 0; i < components.size();i++){
    component* new_comp = components[i];
    int values_in = read_data_to_LSM(new_comp,0);
    int new_key = key * -1;

    // check if all values in the component have been read, if not, keep
    // reading component
    while(values_in <= new_comp->element_count){
      vector<int> *data = new_comp->values;

      // binary search through mainmemory to find key, if not keep searching

      int value = binarySearch(data,0,data.size();key);
      int neg_value = binarySearch(data,0,data.size();new_key);

      // if value found (something not -1 returned), return it

      // if ((value != -1)&&(neg_value == -1)){
      //   return value;
      // } else if ((value == -1)&&(neg_value != -1)){
      //   neg_found = 1;
      //   break;
      // } else if ((value != -1)&&(neg_value != -1)){
      //
      //
      // }

      if (value != -1){
        return value;
      }

      delete new_comp->values;
      values_in = read_data_to_LSM(new_comp,values_in);

    }

    if(neg_found == 1){
      break;
    }

  }

  return -1;
}

void write(int key, int value){
  if (buffer_count != DEFAULT_BUFFER_SIZE){
    buffer[buffer_count] = key;
    buffer[buffer_count+1] = value;
    buffer_count = buffer_count + 2;
  } else {
    buffer_count = 0;

    string file_name("C");
    file_name.append(to_string(component_number));
    file_name.append(".dat");

    component* new_comp = (component*)malloc(sizeof(component));
    new_comp->values = NULL;
    new_comp->filename = file_name;
    new_comp->next_component = NULL;
    new_comp->level = 0;
    new_comp->component_number = component_number;
    component_number = component_number + 1;

    //Add new_comp to vector
    components.insert(0,new_comp);

    fp = fopen(file_name.c_str(), "a");
    fwrite(buffer, sizeof(int), sizeof(data)/sizeof(int), fp);
    fclose(fp);

    vector<string> filename;
    filename.push_back(file_name);
    sort(filename);

    int level;
    int level_counter = 0;
    for(int i = 0; i < components.size();i++){
      if(i > 0){
        if(components[i]->level != components[i-1]->level){
          level_counter = 1;
        }
       else {
          level_counter = level_counter + 1;
        }
      }

      if(level_counter > T) {
        level_counter = 0;
        vector<string> merge_filenames;

        string merge_file_name("C");
        merge_file_name.append(to_string(component_number));
        merge_file_name.append(".dat");

        component* new_comp2 = (component*)malloc(sizeof(component));
        new_comp2->values = NULL;
        new_comp2->filename = merge_file_name;
        new_comp2->next_component = NULL;
        new_comp2->level = components[i]->level + 1;
        new_comp2->component_number = component_number;
        component_number = component_number + 1;
        //Add new_comp to vector
        components.insert(i+1,new_comp2);

        for(int j = 0; j < T; j++){
          delete components[i - j]->values;
          string curr_filename = components[i - j]->filename;
          free(components[i - j]);
          merge_filenames.push_back(curr_filename);
        }

        merge_filenames.push_back(merge_file_name);
        sort(merge_filenames);

        components.erase(i-T+1,i+1);

        i = i - T;

      }

      }
    }

  }


void del(int key){
  new_key = key * -1;
  write(new_key, 0);
}

void update(int key, int update_value){
  del(key);
  write(key,update_value);
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
