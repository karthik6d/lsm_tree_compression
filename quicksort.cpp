#include "quicksort.h"
#include <sys/stat.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include "minheap.h"

int global_counter = 0;  // Size of main memory

using namespace std;

// Merges k sorted files.  Names of files are assumed
// to be 1, 2, 3, ... k
void mergeFiles(FILE *out, vector<string> input_files, int k) {
  FILE *in[k];

  for (int i = 0; i < input_files.size(); i++) {
    in[i] = fopen(input_files.at(i).c_str(), "r");
  }

  // Create a min heap with k heap nodes.  Every heap node
  // has first element of scratch output file
  MinHeapNode *harr = new MinHeapNode[k];
  int i;
  for (i = 0; i < k; i++) {
    // break if no output file is empty and
    // index i will be no. of input files
    int read_data[2];
    if (fread(read_data, sizeof(int), 2, in[i]) < 2) {
      break;
    }
    else {
      harr[i].element = read_data[0];
      harr[i].value = read_data[1];
      harr[i].i = i;  // Index of scratch output file
    }
  }

  MinHeap hp(harr, k);  // Create the heap

  int count = 0;

  // Now one by one get the minimum element from min
  // heap and replace it with next element.
  // run till all filled input files reach EOF
  while (count != i) {
    // Get the minimum element and store it in output file
    MinHeapNode root = hp.getMin();
    int key_value[2] = {root.element, root.value};
    fwrite(key_value, sizeof(int), 2, out);

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

    // Replace root with next element of input file
    hp.replaceMin(root);
  }

  // close input and output files
  for (int i = 0; i < input_files.size(); i++) {
    fclose(in[i]);
    remove(input_files.at(i).c_str());
  }

  fclose(out);
}

component sort(vector<component> components){
    vector<string> names;

    //Add all subcomponents into a filename list
    for(int i = 0; i < components.size(); i++){
        component temp = components.at(i);
        //cout << "Problem Here\n";
        for(int j = 0; j < temp.subcomponents.size(); j++){
            names.push_back(temp.subcomponents.at(j).filename);
            cout << temp.subcomponents.at(j).filename << "\n";
        }
    }

    //Merge the subcomponents using external sort
    string output_file_name = "generic_output.dat";
    FILE* output = fopen(output_file_name.c_str(), "w");
    mergeFiles(output, names, names.size());
    fclose(output);

    output = fopen(output_file_name.c_str(), "r");

    //Create the new component
    component c;
    vector<subcomponent> subs;
    struct stat st;
    size_t size = 0;
    // Get file size
    if (stat(output_file_name.c_str(), &st) == 0) {
        size = st.st_size;
    }


    size_t counter = 0;
    int name_counter = 0;
    //Number of ints in the output file
    size /= 4;

    while(counter < size){
        int length = size - counter > (DEFAULT_BUFFER_SIZE*2) ? (DEFAULT_BUFFER_SIZE*2) : (size-counter);
        int buffer[length];

        fread(buffer, sizeof(int), length, output);
        int min = buffer[0];
        int max = buffer[length-2];

        FILE* new_sub = fopen(names.at(name_counter).c_str(), "w");
        fwrite(buffer, sizeof(int), length, new_sub);
        fclose(new_sub);


        subcomponent temp;
        temp.filename = names.at(name_counter);
        temp.min_value = min;
        temp.max_value = max;
        subs.push_back(temp);

        counter += (DEFAULT_BUFFER_SIZE*2);
        name_counter++;
    }

    c.subcomponents = subs;

    return c;
}
