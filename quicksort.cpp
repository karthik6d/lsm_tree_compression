#include "quicksort.h"
#include "compression.h"
#include "minheap.h"

#include <sys/stat.h>
#include <cstdio>
#include <iostream>
#include <vector>

int global_counter = 0;
string prefix = "data/new_C";
using namespace std;

// Merges k sorted files.  Names of files are assumed
// to be 1, 2, 3, ... k
vector<subcomponent> mergeFiles(vector<string> input_files, vector<string> compressed_files, int k) {
  cout << "mergeFile check 1\n";

  FILE *in[k];
  FILE *output_files[k];
  vector<string> output_file_names;
  vector<string> compressed_file_names;
  vector<kv> min_max;

  cout << "mergeFile check 2\n";

  for (int i = 0; i < input_files.size(); i++) {
    in[i] = fopen(input_files.at(i).c_str(), "r");
    string new_file = prefix + to_string(global_counter) + ".dat";
    output_files[i] = fopen(new_file.c_str(), "w");
    output_file_names.push_back(new_file);
    global_counter++;
  }

  // Create a min heap with k heap nodes.  Every heap node
  // has first element of scratch output file

  cout << "mergeFile check 3\n";

  MinHeapNode *harr = new MinHeapNode[k];
  int i;
  cout << "mergeFile check 4\n";
  for (i = 0; i < k; i++) {
    // break if no output file is empty and
    // index i will be no. of input files
    cout << "mergeFile check 4.1\n";
    int read_data[2];

    cout << "mergeFile check 4.2 - in[i] = " << in[i] << "; k = " << k << "; input_files.size() = "
      << input_files.size() << "; i = " << i << "\n";

    size_t elts_read = fread(read_data, sizeof(int), 2, in[i]);

    cout << "mergeFile check 4.3\n";

    if (elts_read < 2) {
      break;
    }
    else {
      cout << "mergeFile check 4.3.1\n";
      harr[i].element = read_data[0];
      cout << "mergeFile check 4.3.2\n";
      harr[i].value = read_data[1];
      cout << "mergeFile check 4.3.2\n";
      harr[i].i = i;  // Index of scratch output file
    }
  }

  cout << "mergeFile check 5\n";

  MinHeap hp(harr, k);  // Create the heap

  int count = 0;
  int index = 0;
  int output_counter = 0;
  int arr[DEFAULT_BUFFER_SIZE*2];

  cout << "mergeFile check 6\n";

  // Now one by one get the minimum element from min
  // heap and replace it with next element.
  // run till all filled input files reach EOF
  while (count != i) {
    // Get the minimum element and store it in output file
    MinHeapNode root = hp.getMin();

    arr[index] = root.element;
    arr[index+1] = root.value;
    index += 2;

    int key_value[2] = {0, 0};

    if(index == DEFAULT_BUFFER_SIZE*2){
      fwrite(arr, sizeof(int), index, output_files[output_counter]);
      fclose(output_files[output_counter]);
      cout << "mergeFile check 6.1\n";
      char *outfile_name;
      Status status = rle_delta_file_encode(output_file_names.at(output_counter).c_str(), &outfile_name);
      (void)status;
      string compressed_file = string(outfile_name);
      cout << "mergeFile check 6.2\n";
      compressed_file_names.push_back(compressed_file);
      kv pair;
      pair.key = arr[0];
      pair.value = arr[(DEFAULT_BUFFER_SIZE*2) - 2];
      min_max.push_back(pair);
      index = 0;
      output_counter++;
    }

    cout << "mergeFile check 7\n";

    // Find the next element that will replace current
    // root of heap. The next element belongs to same
    // input file as the current min element.
    if (fread(key_value, sizeof(int), 2, in[root.i]) < 2) {
      root.element = INT_MAX;
      root.value = INT_MAX;
      count++;
    }
    else {
      root.element = key_value[0];
      root.value = key_value[1];
    }

    cout << "mergeFile check 8\n";

    // Replace root with next element of input file
    hp.replaceMin(root);
  }

  cout << "mergeFile check 9\n";

  // close input and output files
  for (int i = 0; i < input_files.size(); i++) {
    fclose(in[i]);
    //fclose(output_files[i]);
    remove(input_files.at(i).c_str());
    remove(compressed_files.at(i).c_str());
  }

  //Create the new subcomponents
  vector<subcomponent> final_subcomponents;
  for(int i = 0; i < output_file_names.size(); i++){
      subcomponent temp;
      temp.filename = output_file_names.at(i);
      temp.compressed_filename = compressed_file_names.at(i);
      temp.min_value = min_max.at(i).key;
      temp.max_value = min_max.at(i).value;
      final_subcomponents.push_back(temp);
  }

  return final_subcomponents;
}

component sort(vector<component> components){
  cout << "sort check 1\n";

  vector<string> names;
  vector<string> compressed;

  cout << "sort check 2\n";

  //Add all subcomponents into a filename list
  for(int i = 0; i < components.size(); i++){
    cout << "sort check 2.1\n";
    component temp = components.at(i);
    cout << "sort check 2.2\n";
    for(int j = 0; j < temp.subcomponents.size(); j++){
      cout << "sort check 2.2.1\n";
      names.push_back(temp.subcomponents.at(j).filename);
      cout << "sort check 2.2.2\n";
      compressed.push_back(temp.subcomponents.at(j).compressed_filename);
    }
  }

  //Merge the subcomponents using external sort
  cout << "sort check 3\n";
  vector<subcomponent> output_files = mergeFiles(names, compressed, names.size());

  //Create the new component
  cout << "sort check 4\n";
  component c;
  cout << "sort check 5\n";
  c.subcomponents = output_files;
  cout << "sort check 6\n";

  return c;
}
