#include <iostream>
#include <vector>
#include <sys/stat.h>
#include "quicksort.h"
#include "minheap.h"

#define MEM_SIZE 1024 //Size of main memory

using namespace std;

// Merges k sorted files.  Names of files are assumed
// to be 1, 2, 3, ... k
void mergeFiles(FILE* out, int n, int k)
{
    FILE* in[k];
    for (int i = 0; i < k; i++)
    {
        char fileName[2];

        // convert i to string 
        snprintf(fileName, sizeof(fileName), "%d", i);

        // Open output files in read mode.
        in[i] = fopen(fileName, "r");
    }

    // FINAL OUTPUT FILE
    //FILE *out = openFile(output_file, "w");

    // Create a min heap with k heap nodes.  Every heap node
    // has first element of scratch output file
    MinHeapNode* harr = new MinHeapNode[k];
    int i;
    for (i = 0; i < k; i++)
    {
        // break if no output file is empty and
        // index i will be no. of input files
        fscanf(in[i], "%d", &harr[i].element);
        if(fscanf(in[i], "%d", &harr[i].value) != 1){
          break;
        }

        harr[i].i = i; // Index of scratch output file
    }
    MinHeap hp(harr, i); // Create the heap

    int count = 0;

    // Now one by one get the minimum element from min
    // heap and replace it with next element.
    // run till all filled input files reach EOF
    while (count != i)
    {
        // Get the minimum element and store it in output file
        MinHeapNode root = hp.getMin();
        fprintf(out, "%d ", root.element);
        fprintf(out, "%d", root.value);

        // Find the next element that will replace current
        // root of heap. The next element belongs to same
        // input file as the current min element.
        fscanf(in[root.i], "%d", &root.element);
        if (fscanf(in[root.i], "%d ", &root.value) != 1)
        {
            root.element = INT_MAX;
            root.value = INT_MAX;
            count++;
        }

        // Replace root with next element of input file
        hp.replaceMin(root);
    }

    // close input and output files
    for (int i = 0; i < k; i++)
        fclose(in[i]);

    fclose(out);
}

// Using a merge-sort algorithm, create the initial runs
// and divide them evenly among the output files
void createInitialRuns(FILE* fp, int run_size, int num_ways) {
    // output scratch files
    FILE* out[num_ways];
    char fileName[2];
    for (int i = 0; i < num_ways; i++)
    {
        // convert i to string
        snprintf(fileName, sizeof(fileName), "%d", i);

        // Open output files in write mode.
        out[i] = fopen(fileName, "w");
    }

    // allocate a dynamic array large enough
    // to accommodate runs of size run_size
    int* arr = (int*)malloc(run_size * sizeof(int));

    bool more_input = true;
    int next_output_file = 0;

    int i;
    while (more_input)
    {
        // write run_size elements into arr from input file
        for (i = 0; i < run_size; i++)
        {
            if (fscanf(fp, "%d", &arr[i]) != 1)
            {
                more_input = false;
                break;
            }
        }

        //Break up the vector into two sub vectors
        vector<int> *keys = new vector<int>;
        vector<int> *values = new vector<int>;

        for(int i = 0; i < run_size; i++){
          if(i % 2 == 0){
            keys->push_back(arr[i]);
          }
          else{
            values->push_back(arr[i]);
          }
        }

        vector<int>* sort_arr[2] = {keys, values};

        // sort array using merge sort
        mergeSort(sort_arr, 2, 0, 0, i/2 - 1);

        //Put the vectors pack into arr
        for(int i = 0; i < run_size; i++){
          if(i % 2 == 0){
           arr[i] = keys->at(i/2);
          }
          else{
            arr[i] = values->at(i/2);
          }
        }

        // write the records to the appropriate scratch output file
        // can't assume that the loop runs to run_size
        // since the last run's length may be less than run_size
        for (int j = 0; j < i; j++)
            fprintf(out[next_output_file], "%d ", arr[j]);

        next_output_file++;
    }

    // close input and output files
    for (int i = 0; i < num_ways; i++)
        fclose(out[i]);

    fclose(fp);
}

void in_memory_sort(FILE* fp, size_t size){
  int number_members = (size / sizeof(int));
  int data_ptr[number_members];
  size_t nmemb = fread(data_ptr, 4, number_members, fp);

  vector<int> *keys = new vector<int>;
  vector<int> *values = new vector<int>;

  //Allocate two vectors both the keys and values
  for(int i = 0; i < number_members; i++){
    if(i % 2 == 0){
      keys->push_back(data_ptr[i]);
    }
    else{
      values->push_back(data_ptr[i]);
    }
  }

  vector<int>* arr[2] = {keys, values};
  mergeSort(arr, 2, 0, 0, (number_members/2)-1);

  //Write everything to the disk file
  for(int i = 0; i < number_members; i++){
    if(i % 2 == 0){
      data_ptr[i] = keys->at(i/2);
    }
    else{
      data_ptr[i] = values->at(i/2);
    }
  }

  //FILE output_fp* = fopen(output_file.c_str(), "w");
  fwrite(data_ptr, sizeof(int), sizeof(data_ptr)/sizeof(int), fp);
  fclose(fp);
}


void external_sort(FILE *fp, size_t size){
  int num_ways = (size / ((int)MEM_SIZE * (int)sizeof(int))) + 1;
  int run_size = (int)MEM_SIZE;

  createInitialRuns(fp, run_size, num_ways);
  mergeFiles(fp, run_size, num_ways);
}


string sort(string filename){
  FILE *fp;
  fp = fopen(filename.c_str(), "r");
  struct stat st;
  size_t col_size=0;
  //Get file size of the col_data
  if (stat(filename.c_str(), &st) == 0){
    col_size = st.st_size;
  }

  string output_file;
  if(col_size <= MEM_SIZE){
    //TODO: call in memory sort
    in_memory_sort(fp, col_size);
  }
  else{
    //TODO: call external sort
    external_sort(fp, col_size);
  }

  return output_file;
}


int main(){
  vector<int> *keys = new vector<int>;
  vector<int> *values = new vector<int>;

  for(int i = 0; i < 10; i++){
    keys->push_back(10-i);
    values->push_back(i);
  }

  vector<int>* arr[2] = {keys, values};
  mergeSort(arr, 2, 0, 0, 9);

  for(int i = 0; i < keys->size(); i++){
    cout << "Key: "<<keys->at(i) << "\t Value: " << values->at(i) << "\n";
  }

  return 0;
}
