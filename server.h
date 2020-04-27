#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdio>
#include <sstream>
#include <tuple>
#include <string>

#define DEFAULT_BUFFER_SIZE 4 // Defines number of ints in the Main Buffer aka Main Memory
#define T 2 // Size ratio between components


using namespace std;

enum query_type { read_query, write_query, update_query, delete_query };

typedef struct workload_entry {
  query_type type;
  int key;
  int value;
} workload_entry;

typedef struct kv {
  int key;
  int value;
} kv;

// Structure for a component in LSM Tree
typedef struct component {
  string filename;

  pair<bool, int> read(int key);
} component;

typedef struct level {
  vector<component> components;
  int level_capacity;

  pair<bool, int> read(int key);
} level;

// Structure for LSM Tree
typedef struct LSM_Tree {
  string name;
  vector<level> levels;
  vector<kv> buffer;

  void write(int key, int value);
  pair<bool, int> read(int key);
  void del(int key);
  void update(int key, int value);
  void insert_component(vector<kv> kvs);
  void insert_component(component c);
} LSM_Tree;

//Declarations for server.cpp
void create(string db_name);
void load(string path);
int read(int key);
void write(int key, int value);
void del(int key);
int read_data_to_LSM(component* curr_comp);
int binary_search(vector<kv> data, int x);

vector<int> execute_workload();
