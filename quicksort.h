#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#define DEFAULT_BUFFER_SIZE 4  // Defines number of ints in the Main Buffer
#define T 2                    // Size ratio between components

using namespace std;

void merge(vector<int>* arr[], int N, int primary_index, int l, int m, int r);
void mergeSort(vector<int>* arr[], int N, int primary_key, int l, int r);
void in_memory_sort(FILE* fp, string filename, size_t size);
void external_sort(FILE* fp, string filename, size_t size);
void sort(string filename);
void createInitialRuns(FILE* fp, int run_size, int num_ways, size_t size);
void mergeFiles(FILE* out, int n, int k);
string create_data();

void merge(vector<int>* arr[], int N, int primary_index, int l, int m, int r) {
  int i, j, k;
  int n1 = m - l + 1;
  int n2 = r - m;

  /* create temp arrays */
  vector<int>* L[N];
  vector<int>* R[N];
  for (int z = 0; z < N; z++) {
    L[z] = new vector<int>;
    R[z] = new vector<int>;
  }

  /* Copy data to temp arrays L[] and R[] */
  for (int z = 0; z < N; z++) {
    vector<int>* real = arr[z];
    vector<int>* temp_l = L[z];
    vector<int>* temp_r = R[z];
    for (i = 0; i < n1; i++) {
      temp_l->push_back(real->at(l + i));
    }
    for (j = 0; j < n2; j++) {
      temp_r->push_back(real->at(m + 1 + j));
    }
  }

  /* Merge the temp arrays back into arr[l..r]*/
  i = 0;  // Initial index of first subarray
  j = 0;  // Initial index of second subarray
  k = l;  // Initial index of merged subarray

  vector<int>* main_l = L[primary_index];
  vector<int>* main_r = R[primary_index];

  while (i < n1 && j < n2) {
    if (main_l->at(i) <= main_r->at(j)) {
      for (int z = 0; z < N; z++) {
        vector<int>* temp = arr[z];
        vector<int>* temp_left = L[z];
        temp->at(k) = temp_left->at(i);
      }
      i++;
    } else {
      for (int z = 0; z < N; z++) {
        vector<int>* temp = arr[z];
        vector<int>* temp_right = R[z];
        temp->at(k) = temp_right->at(j);
      }
      j++;
    }
    k++;
  }

  /* Copy the remaining elements of L[], if there
     are any */
  while (i < n1) {
    for (int z = 0; z < N; z++) {
      vector<int>* temp = arr[z];
      vector<int>* temp_left = L[z];
      temp->at(k) = temp_left->at(i);
    }
    i++;
    k++;
  }

  /* Copy the remaining elements of R[], if there
     are any */
  while (j < n2) {
    for (int z = 0; z < N; z++) {
      vector<int>* temp = arr[z];
      vector<int>* temp_right = R[z];
      temp->at(k) = temp_right->at(j);
    }
    j++;
    k++;
  }

  // for(int i = 0; i < N; i++){
  //   vector_free(L[i]);
  //   free(L[i]);
  //   vector_free(R[i]);
  //   free(R[i]);
  // }
}

/* l is for left index and r is right index of the
   sub-array of arr to be sorted */
void mergeSort(vector<int>* arr[], int N, int primary_key, int l, int r) {
  if (l < r) {
    // Same as (l+r)/2, but avoids overflow for
    // large l and h
    int m = l + (r - l) / 2;

    // Sort first and second halves
    mergeSort(arr, N, primary_key, l, m);
    mergeSort(arr, N, primary_key, m + 1, r);

    merge(arr, N, primary_key, l, m, r);
  }
}