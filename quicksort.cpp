#include "quicksort.h"

#include <sys/stat.h>

#include <cstdio>
#include <iostream>
#include <vector>

#include "minheap.h"

#define MEM_SIZE 10  // Size of main memory

using namespace std;

// Merges k sorted files.  Names of files are assumed
// to be 1, 2, 3, ... k
void mergeFiles(FILE *out, int n, int k) {
  FILE *in[k];
  for (int i = 0; i < k; i++) {
    char fileName[2];

    // convert i to string
    snprintf(fileName, sizeof(fileName), "%d", i);

    // Open output files in read mode.
    in[i] = fopen(fileName, "r");
  }

  // FINAL OUTPUT FILE
  // FILE *out = openFile(output_file, "w");

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
    } else {
      harr[i].element = read_data[0];
      harr[i].value = read_data[1];
      harr[i].i = i;  // Index of scrath output file
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
    } else {
      root.element = key_value[0];
      root.value = key_value[1];
    }

    // Replace root with next element of input file
    hp.replaceMin(root);
  }

  // close input and output files
  for (int i = 0; i < k; i++) {
    fclose(in[i]);
    char fileName[2];
    // convert i to string
    snprintf(fileName, sizeof(fileName), "%d", i);
    remove(fileName);
  }

  fclose(out);
}

// Using a merge-sort algorithm, create the initial runs
// and divide them evenly among the output files
void createInitialRuns(FILE *fp, int run_size, int num_ways, size_t size) {
  // output scratch files
  FILE *out[num_ways];
  char fileName[2];
  for (int i = 0; i < num_ways; i++) {
    // convert i to string
    snprintf(fileName, sizeof(fileName), "%d", i);

    // Open output files in write mode.
    out[i] = fopen(fileName, "w");
  }

  // int data[40];
  // fread(data, sizeof(int), sizeof(data)/sizeof(int), fp);
  // for(int i = 0; i < 40; i++){
  //   cout << data[i] << "\t";
  // }
  // cout << "\n";
  // cout << "After\n";
  // int data;
  // int val = fscanf(fp, "%d", &data);
  // cout << "Data: " << data << " FSCANF Val:" << val << "\n";

  // allocate a dynamic array large enough
  // to accommodate runs of size run_size
  int arr[run_size];

  bool more_input = true;
  int next_output_file = 0;

  int i;
  int num_integers = size / sizeof(int);
  int write_amount;
  while (num_integers > 0) {
    // Determine how much to read
    if (num_integers >= run_size) {
      fread(arr, sizeof(int), run_size, fp);
      num_integers -= run_size;
      write_amount = run_size;
    } else {
      fread(arr, sizeof(int), num_integers, fp);
      num_integers = 0;
      write_amount = num_integers;
    }

    // cout << "PRE-SORT RUN: " << next_output_file << "\n";
    // for(int i = 0; i < 10; i++){
    //   cout << arr[i] << "\t";
    // }
    // cout << "\n";

    // Break up the vector into two sub vectors
    vector<int> *keys = new vector<int>;
    vector<int> *values = new vector<int>;

    for (int i = 0; i < run_size; i++) {
      if (i % 2 == 0) {
        keys->push_back(arr[i]);
      } else {
        values->push_back(arr[i]);
      }
    }

    vector<int> *sort_arr[2] = {keys, values};
    // for(int i = 0; i < keys->size(); i++){
    //   cout << "Pre-Keys: " << keys->at(i) << "\t";
    // }
    // cout << "\n";
    //
    // sort array using merge sort
    mergeSort(sort_arr, 2, 0, 0, (run_size / 2) - 1);

    // for(int i = 0; i < keys->size(); i++){
    //   cout << "Post-Keys: " << keys->at(i) << "\t";
    // }
    // cout << "\n";

    // Put the vectors pack into arr
    for (int i = 0; i < run_size; i++) {
      if (i % 2 == 0) {
        arr[i] = keys->at(i / 2);
      } else {
        arr[i] = values->at(i / 2);
      }
    }

    // //Print
    // cout << "POST-SORT RUN: " << next_output_file << "\n";
    // for(int i = 0; i < 10; i++){
    //   cout << arr[i] << "\t";
    // }
    // cout << "\n";

    // Write the data to the file
    fwrite(arr, sizeof(int), write_amount, out[next_output_file]);

    // Create run for next file
    next_output_file++;
  }

  // close input and output files
  for (int i = 0; i < num_ways; i++) fclose(out[i]);

  fclose(fp);
}

void in_memory_sort(FILE *fp, string filename, size_t size) {
  int number_members = (size / sizeof(int));
  int data_ptr[number_members];
  size_t nmemb = fread(data_ptr, 4, number_members, fp);
  fclose(fp);

  vector<int> *keys = new vector<int>;
  vector<int> *values = new vector<int>;

  // Allocate two vectors both the keys and values
  for (int i = 0; i < number_members; i++) {
    if (i % 2 == 0) {
      keys->push_back(data_ptr[i]);
    } else {
      values->push_back(data_ptr[i]);
    }
  }

  vector<int> *arr[2] = {keys, values};
  mergeSort(arr, 2, 0, 0, (number_members / 2) - 1);

  // Write everything to the disk file
  for (int i = 0; i < number_members; i++) {
    if (i % 2 == 0) {
      data_ptr[i] = keys->at(i / 2);
    } else {
      data_ptr[i] = values->at(i / 2);
    }
  }

  FILE *output_fp = fopen(filename.c_str(), "w");
  fwrite(data_ptr, sizeof(int), sizeof(data_ptr) / sizeof(int), output_fp);
  fclose(output_fp);
}

void external_sort(FILE *fp, string filename, size_t size) {
  int num_ways = (size / ((int)MEM_SIZE * (int)sizeof(int)));
  int run_size = (int)MEM_SIZE;

  createInitialRuns(fp, run_size, num_ways, size);
  fp = fopen(filename.c_str(), "w");
  mergeFiles(fp, run_size, num_ways);
}

void sort(string filename) {
  FILE *fp;
  fp = fopen(filename.c_str(), "r");
  struct stat st;
  size_t col_size = 0;
  // Get file size of the col_data
  if (stat(filename.c_str(), &st) == 0) {
    col_size = st.st_size;
  }

  cout << "COL SIZE: " << col_size << "\n";
  string output_file;
  if (col_size / sizeof(int) <= MEM_SIZE) {
    // TODO: call in memory sort
    cout << "IN MEMORY SORT\n";
    in_memory_sort(fp, filename, col_size);
  } else {
    // TODO: call external sort
    cout << "EXTERNAL SORT\n";
    external_sort(fp, filename, col_size);
  }
}

string create_data() {
  // Use this to test the above functions
  // Create simple key value as follows:
  // 10:0, 9:1, 8:2, 7:3, 6:4, 5:5, 4:6, 3:7, 2:8, 1:9
  vector<int> *keys = new vector<int>;
  vector<int> *values = new vector<int>;

  for (int i = 0; i < 20; i++) {
    keys->push_back(20 - i);
    values->push_back(i);
  }

  int final_data[40];

  for (int i = 0; i < 40; i++) {
    if (i % 2 == 0) {
      final_data[i] = keys->at(i / 2);
    } else {
      final_data[i] = values->at(i / 2);
    }
  }

  string output_file = "output_file.txt";
  FILE *output_fp = fopen(output_file.c_str(), "w");
  fwrite(final_data, sizeof(int), sizeof(final_data) / sizeof(int), output_fp);
  fclose(output_fp);

  return output_file;
}

int main() {
  string output_file = create_data();

  sort(output_file);
  FILE *fp;
  int sorted_data[10];

  // for(int i = 0; i < 4; i++){
  //   char fileName[2];
  //   snprintf(fileName, sizeof(fileName), "%d", i);
  //   fp = fopen(fileName, "r");
  //   fread(sorted_data, sizeof(int), sizeof(sorted_data)/sizeof(int), fp);
  //   fclose(fp);
  //
  //   cout << "MERGE FILE: " << i << "\n";
  //   for(int j = 0; j < 10; j++){
  //     cout << sorted_data[j] << "\t";
  //   }
  //   cout << "\n";
  // }
  // fclose(fp);

  int data[40];
  fp = fopen(output_file.c_str(), "r");
  fread(data, sizeof(int), sizeof(data) / sizeof(int), fp);
  fclose(fp);

  // See if it is correct
  cout << "After Merging: "
       << "\n";
  for (int i = 0; i < 40; i++) {
    cout << data[i] << "\n";
  }

  return 0;
}
