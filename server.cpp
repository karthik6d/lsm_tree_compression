#include "quicksort.h"
#include "compression.h"

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
  ifstream data_file(path, ios::ate);

  int length = data_file.tellg();
  data_file.close();

  FILE* f = fopen(path.c_str(), "r");

  char* data =
	  (char*)mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fileno(f), 0);
  fclose(f);

  string data_str(data, data + length);

  assert(munmap(data, length) == 0);

  int i = 0;

  int lines = count(data_str.begin(), data_str.end(), '\n');

  cout << "reading csv" << endl;

  istringstream ss(data_str);

  string line;

  while (getline(ss, line)) {
  	cout << "\r" << ++i << "/" << lines << "\n" << flush;

  	istringstream ss(line);

  	string key_s, value_s;

  	getline(ss, key_s, ',');
  	getline(ss, value_s, ',');

    cout << "pre-write\n";

  	current_db -> write(stoi(key_s), stoi(value_s));

    cout << "iter end\n";
  }

  cout << endl;
}

void LSM_Tree::write(int key, int value) {
  // if it fits into the top-level buffer, just add it there
  cout << "pre top level buffer check\n";
  if (this->buffer.size() < DEFAULT_BUFFER_SIZE) {
    this->buffer.push_back({key, value});
    return;
  }

  // otherwise, we have to create a new component for it
  cout << "pre create component\n";
  component c = create_component(this->buffer);
  cout << "pre insert component\n";
  this->insert_component(c);
  cout << "pre buffer clear\n";
  this->buffer.clear();
  cout << "pre push back\n";
  this->buffer.push_back({key, value});
  cout << "!! write end !!\n";
}

void LSM_Tree::del(int key) { this->write(-key, 0); }

void LSM_Tree::update(int key, int value) { this->write(key, value); }

// comparison function between key value pairs
int compare_kvs(kv a, kv b) {
  int a_val = a.key < 0 ? -a.key : a.key;
  int b_val = b.key < 0 ? -b.key : b.key;
  return a_val < b_val;
}

subcomponent create_subcomponent(vector<kv>& kvs, int start, int end) {
  int min_value = INT32_MAX;
  int max_value = INT32_MIN;

  for (int j = start; j < end; ++j) {
	auto pair = kvs[j];
	int val = pair.key < 0 ? -pair.key : pair.key;

	if (val < min_value) {
	  min_value = val;
	}

	if (val > max_value) {
	  max_value = val;
	}
  }

  //Have to add the compressed as well
  // creating the data file
  string subcomponent_file_name("data/C");
  subcomponent_file_name.append(to_string(component_count++));
  subcomponent_file_name.append(".dat");

  ofstream data_file(subcomponent_file_name, ios::binary);

  // writing the key value pairs to the data file
  data_file.write((char*)(kvs.data() + start), (end - start) * sizeof(kv));
  data_file.close();

  //Add the compressed version of the file
  char *outfile_name;
  Status status = rle_delta_file_encode(subcomponent_file_name.c_str(), &outfile_name);
  (void)status;
  string compressed_file = string(outfile_name);

  return {.filename = subcomponent_file_name,
          .compressed_filename = compressed_file,
		  .min_value = min_value,
		  .max_value = max_value};
}

component create_component(vector<kv> kvs) {
  unordered_map<int, int> m;

  // iterate over the vector and collect the last updates
  for (kv k : kvs) {
	m.erase(-k.key);
	m[k.key] = k.value;
  }

  // create the final list of updates
  vector<kv> final_kvs;

  for (auto p : m) {
	final_kvs.push_back({p.first, p.second});
  }

  // sort the list of updates
  std::sort(final_kvs.begin(), final_kvs.end(), compare_kvs);

  // after the sorting and collecting, there should never be a negative key
  for (kv k : final_kvs) {
	assert(k.key >= 0);
  }

  vector<subcomponent> subs;

  for (int i = 0; i < final_kvs.size();) {
	int start = i;
	int end = min((size_t)i + DEFAULT_BUFFER_SIZE, final_kvs.size());

	auto sub = create_subcomponent(final_kvs, start, end);
	subs.push_back(sub);

	i = end;
  }

  return {.subcomponents = subs};
}

void LSM_Tree::insert_component(component c) {
  cout << "tree_insert_component check 1\n";
  int i = 0;

  cout << "tree_insert_component check 2\n";
  pair<bool, component> res;

  cout << "tree_insert_component check 3\n";
  do {
    cout << "tree_insert_component check 3.1\n";
  	if (i >= this->levels.size()) {
  	  this->levels.push_back(level());
  	}

    cout << "tree_insert_component check 3.2\n";
  	res = this->levels[i++].insert_component(c);

    cout << "tree_insert_component check 3.3\n";
  	c = res.second;
  } while (res.first);

  cout << "tree_insert_component check 4\n";
}

pair<read_result, int> LSM_Tree::read(int key) {
    cout << "READ\n";
  // look through buffer first (go in reverse)
  for (auto it = this->buffer.rbegin(); it != this->buffer.rend(); ++it) {
	auto k = *it;

	if (k.key == key) {
	  return pair<read_result, int>(found, k.value);
	} else if (k.key == -key) {
	  return pair<read_result, int>(found, 0);
	}
  }

  // go through each level and try to read
  for (level l : this->levels) {
	pair<read_result, int> res = l.read(key);

	if (res.first == found || res.first == deleted) {
	  return res;
	}
  }

  return pair<read_result, int>(not_found, 0);
}

pair<bool, component> level::insert_component(component c) {
  cout << "level_insert_component check 1\n";
  if (this->components.size() < COMPONENTS_PER_LEVEL) {
  	this->components.push_back(c);

  	return pair<bool, component>(false, component());
  }

  // Merge Step
//  vector<kv> all_kvs;
//
//  for (auto level_c : this->components) {
//    auto c_kvs = level_c.get_kvs();
//    all_kvs.insert(all_kvs.end(), c_kvs.begin(), c_kvs.end());
//
//    for (auto sub : level_c.subcomponents) {
//      remove(sub.filename.c_str());
//    }
//  }
//  cout << "\nBefore External Sort1 1 1\t";

  cout << "level_insert_component check 2\n";

  auto new_c = sort(this->components);
//    auto new_c = create_component(all_kvs);
  // End Merge Step

  cout << "level_insert_component check 3\n";

  this->components.clear();

  cout << "level_insert_component check 4\n";

  this->components.push_back(c);

  cout << "level_insert_component check 5\n";

  return pair<bool, component>(true, new_c);
}

pair<read_result, int> level::read(int key) {
  for (auto it = this->components.rbegin(); it != this->components.rend();
	   ++it) {
	auto c = *it;

	pair<read_result, int> res = c.read(key);

	if (res.first == found || res.first == deleted) {
	  return res;
	}
  }

  return pair<read_result, int>(not_found, 0);
}

vector<kv> component::get_kvs() {
  vector<kv> res;
  for (auto sub : this->subcomponents) {
	vector<kv> s = sub.get_kvs();

	res.insert(res.end(), s.begin(), s.end());
  }

  return res;
}

pair<read_result, int> component::read(int key) {
  for (auto sub : this->subcomponents) {
	if (key < sub.min_value || key > sub.max_value) {
	  continue;
	}

	pair<read_result, int> res = sub.read(key);

	if (res.first == found || res.first == deleted) {
	  return res;
	}
  }

  return pair<read_result, int>(not_found, 0);
}

vector<kv> subcomponent::get_kvs() {
    //Remove first four lines of code
    //Decode with DEFAULT_BUFFER_SIZE * 2
  ifstream f(this->filename, ios::ate | ios::binary);

  // get the length of the file
  int length = f.tellg();

  // the file must be length 8
  assert(length % sizeof(kv) == 0);

  // go back to the beginning
  f.seekg(0, f.beg);

  // prepare the array and read in the data
  //kv buf[length / sizeof(kv)];
//  f.read((char*)buf, length);
//
//  f.close();
    size_t number_read = 0;
    // kv* buf = (kv*) rlestreamdecode(this->compressed_filename.c_str(), length/4, &number_read);
    kv* buf = (kv*) rle_delta_stream_decode(this->compressed_filename.c_str(), length/4, &number_read);

  return vector<kv>(buf, buf + length / sizeof(kv));
}

pair<read_result, int> subcomponent::read(int key) {
  for (auto k : this->get_kvs()) {
	if (k.key == key) {
	  return pair<read_result, int>(found, k.value);
	}

	if (k.key == -key) {
	  return pair<read_result, int>(deleted, 0);
	}
  }

  return pair<read_result, int>(not_found, 0);
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
	    cout << "Write\n";
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

	cout << "\r" << i << "/" << workload.size() << "\n" << flush;

	switch (e.type) {
	  case read_query: {
		pair<read_result, int> r = current_db->read(e.key);

		if (r.first == found) {
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

	  default: { throw runtime_error("Unsupported workload command"); }
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
