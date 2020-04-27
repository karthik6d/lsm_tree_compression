#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#define DEFAULT_BUFFER_SIZE \
  4096  // Defines number of ints in the Main Buffer aka Main Memory
// #define T 2 // Size ratio between components
#define COMPONENTS_PER_LEVEL 2

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
  vector<kv> get_kvs();
} component;

typedef struct level {
  vector<component> components;
  int level_capacity;

  pair<bool, int> read(int key);
  pair<bool, component> insert_component(component);
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
  void insert_component(component c);
} LSM_Tree;

// Declarations for server.cpp
void create(string db_name);
void load(string path);
int binary_search(vector<kv> data, int x);
component create_component(vector<kv> kvs);

vector<int> execute_workload();
