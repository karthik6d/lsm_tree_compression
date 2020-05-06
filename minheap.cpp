#include <iostream>
#include "minheap.h"

using namespace std;

// A recursive method to heapify a subtree with root
// at given index. This method assumes that the
// subtrees are already heapified
void MinHeap::MinHeapify(int i) {
  int l = left(i);
  int r = right(i);
  int smallest = i;
  if (l < heap_size && harr[l].element < harr[i].element) smallest = l;
  if (r < heap_size && harr[r].element < harr[smallest].element) smallest = r;
  if (smallest != i) {
    swap(&harr[i], &harr[smallest]);
    MinHeapify(smallest);
  }
}

// A utility function to swap two elements
void swap(MinHeapNode* x, MinHeapNode* y) {
  MinHeapNode temp = *x;
  *x = *y;
  *y = temp;
}

int main() { return 0; }