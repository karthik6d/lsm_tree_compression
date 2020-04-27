#include "server.h"

#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace std;
LSM_Tree* current_db;
int component_count = 0;

vector<workload_entry> workload;

void create(string db_name) {
  LSM_Tree* db = new LSM_Tree();
  db->name = db_name;
  current_db = db;

  mkdir("data", 0755);
}

void load(string path) {
  // Read data from CSV File
  ifstream myFile(path);

  string line;
  while (getline(myFile, line)) {
    istringstream ss(line);

    string key_s, value_s;

    getline(ss, key_s, ',');
    getline(ss, value_s, ',');

    current_db->write(stoi(key_s), stoi(value_s));
  }
}

void LSM_Tree::write(int key, int value) {
  // if it fits into the top-level buffer, just add it there
  if (this->buffer.size() < DEFAULT_BUFFER_SIZE) {
    this->buffer.push_back({key, value});
    return;
  }

  // otherwise, we have to create a new component for it
  component c = create_component(this->buffer);
  this->insert_component(c);
  this->buffer.clear();
  this->buffer.push_back({key, value});
}

void LSM_Tree::del(int key) { this->write(-key, 0); }

void LSM_Tree::update(int key, int value) { this->write(key, value); }

// comparison function between key value pairs
int compare_kvs(kv a, kv b) { return a.key < b.key; }

component create_component(vector<kv> kvs) {
  unordered_map<int, int> m;

  // iterate over the vector and collect the last updates
  for (kv k : kvs) {
    m.erase(-k.key);
    m[k.key] = k.value;
  }

  // create the final list of updates
  vector<kv> final_kvs;

  for (auto pair : m) {
    final_kvs.push_back((kv){pair.first, pair.second});
  }

  // sort the list of updates
  std::sort(final_kvs.begin(), final_kvs.end(), compare_kvs);

  // after the sorting and collecting, there should never be a negative key
  for (kv k : final_kvs) {
    assert(k.key >= 0);
  }

  // creating the data file
  string component_file_name("data/C");
  component_file_name.append(to_string(component_count++));
  component_file_name.append(".dat");

  ofstream data_file(component_file_name, ios::binary);

  // writing the key value pairs to the data file
  for (kv k : final_kvs) {
    data_file.write((char*)&k.key, sizeof(k.key));
    data_file.write((char*)&k.value, sizeof(k.value));
  }

  data_file.close();

  return {component_file_name};
}

void LSM_Tree::insert_component(component c) {
  int i = 0;

  pair<bool, component> res;

  do {
    if (i >= this->levels.size()) {
      this->levels.push_back(level());
    }

    res = this->levels[i++].insert_component(c);

    c = res.second;
  } while (res.first);
}

pair<bool, int> LSM_Tree::read(int key) {
  // look through buffer first (go in reverse)
  for (auto it = this->buffer.rbegin(); it != this->buffer.rend(); ++it) {
    auto k = *it;

    if (k.key == key) {
      return pair<bool, int>(true, k.value);
    } else if (k.key == -key) {
      return pair<bool, int>(false, 0);
    }
  }

  // go through each level and try to read
  for (level l : this->levels) {
    pair<bool, int> res = l.read(key);

    if (res.first) {
      return res;
    }
  }

  return pair<bool, int>(false, 0);
}

pair<bool, component> level::insert_component(component c) {
  if (this->components.size() < COMPONENTS_PER_LEVEL) {
    this->components.push_back(c);

    return pair<bool, component>(false, component());
  }

  vector<kv> all_kvs;

  for (auto level_c : this->components) {
    auto c_kvs = level_c.get_kvs();
    all_kvs.insert(all_kvs.end(), c_kvs.begin(), c_kvs.end());

    remove(level_c.filename.c_str());
  }

  auto new_c = create_component(all_kvs);

  this->components.clear();
  this->components.push_back(c);

  return pair<bool, component>(true, new_c);
}

pair<bool, int> level::read(int key) {
  for (auto it = this->components.rbegin(); it != this->components.rend();
       ++it) {
    auto c = *it;

    pair<bool, int> res = c.read(key);

    if (res.first) {
      return res;
    }
  }

  return pair<bool, int>(false, 0);
}

vector<kv> component::get_kvs() {
  vector<kv> kvs;
  ifstream f(this->filename, ios::binary);

  kv k;

  do {
    f.read((char*)&k, sizeof(k));

    if (f.eof()) {
      break;
    }

    kvs.push_back(k);
  } while (true);

  return kvs;
}

pair<bool, int> component::read(int key) {
  vector<kv> kvs = this->get_kvs();

  int res = binary_search(kvs, key);

  if (res == -1) {
    return pair<bool, int>(false, 0);
  }

  return pair<bool, int>(true, kvs[res].value);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "Need to pass in query file" << endl;
    return 1;
  }

  string output_str;
  ifstream query_file(argv[1]);

  // Parse through workload file
  while (getline(query_file, output_str)) {
    istringstream ss(output_str);
    vector<string> elements;

    do {
      string word;
      ss >> word;
      elements.push_back(word);
    } while (ss);

    if (elements[0].compare("create") == 0) {
      create(elements.at(1));

    } else if (elements[0].compare("load") == 0) {
      load(elements.at(1));

    } else if (elements[0].compare("read") == 0) {
      workload.push_back({read_query, stoi(elements[1]), 0});

    } else if (elements[0].compare("write") == 0) {
      workload.push_back({write_query, stoi(elements[1]), stoi(elements[2])});

    } else if (elements[0].compare("delete") == 0) {
      workload.push_back({delete_query, stoi(elements[1])});

    } else if (elements[0].compare("update") == 0) {
      workload.push_back({update_query, stoi(elements[1]), stoi(elements[2])});

    } else {
      throw runtime_error("Unknown command");
    }
  }

  cout << "starting workload" << endl;

  auto result = execute_workload();

  cout << "finished workload, writing results to file" << endl;

  ofstream f("hello.res");

  for (int r : result) {
    f << r << endl;
  }

  f.close();

  return 0;
}

vector<int> execute_workload() {
  vector<int> res;

  int i = 0;
  for (auto e : workload) {
    i++;

    cout << "\r" << i << "/" << workload.size() << flush;

    switch (e.type) {
      case read_query: {
        pair<bool, int> r = current_db->read(e.key);

        if (r.first) {
          res.push_back(r.second);
        }

        break;
      }

      case write_query: {
        current_db->write(e.key, e.value);
        break;
      }

      case delete_query: {
        current_db->del(e.key);
        break;
      }

      case update_query: {
        current_db->update(e.key, e.value);
        break;
      }

      default: {
        throw runtime_error("Unsupported workload command");
      }
    }
  }

  cout << endl;

  return res;
}

int binary_search_helper(vector<kv> data, int l, int r, int x) {
  if (r <= l) {
    return -1;
  }

  int m = (r + l) / 2;

  if (data[m].key > x) {
    return binary_search_helper(data, l, m, x);
  } else if (data[m].key < x) {
    return binary_search_helper(data, m + 1, r, x);
  } else {
    return m;
  }
}

int binary_search(vector<kv> data, int x) {
  return binary_search_helper(data, 0, data.size(), x);
}
