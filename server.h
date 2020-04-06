#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdio>
#include <sstream>
#include <tuple>
#include <string>

#define DEFAULT_BUFFER_SIZE 4 //Defines number of ints in the Main Buffer
#define T 2 // Size ratio between components

using namespace std;

//Structure for a component in LSM Tree
typedef struct component {
  vector<int>* values;
  string filename;
  component* next_component;
  int level;
  int component_number;
} component;


// Structure for LSM Tree
typedef struct LSM_Tree {
  string name;
  component* C0;
} LSM_Tree;

//Declarations for server.cpp
void create(string db_name);
void load(string path);
void read(int key);
void write(int key, int value);
void del(int key);
void read_data_to_LSM(component* curr_comp);
