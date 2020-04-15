#include <iostream> 

using namespace std; 

struct MinHeapNode 
{ 
    // The element to be stored 
    int element; 
    //Value of the key
    int value;
    // index of the array from which the element is taken 
    int i; 
}; 

//Function declarations
void swap(MinHeapNode* x, MinHeapNode* y); 

// A class for Min Heap 
class MinHeap 
{ 
    MinHeapNode* harr; // pointer to array of elements in heap 
    int heap_size;     // size of min heap 
  
    public: 
      // Constructor: creates a min heap of given size 
      MinHeap(MinHeapNode a[], int size){
        harr = a;
        heap_size = size;
      } 
      // to heapify a subtree with root at given index 
      void MinHeapify(int); 
      // to get index of left child of node at index i 
      int left(int i) { return (2 * i + 1); } 
      // to get index of right child of node at index i 
      int right(int i) { return (2 * i + 2); } 
      // to get the root 
      MinHeapNode getMin() {  return harr[0]; } 
      // to replace root with new node x and heapify() 
      // new root 
      void replaceMin(MinHeapNode x) 
      { 
          harr[0] = x; 
          MinHeapify(0); 
      } 
}; 

// A recursive method to heapify a subtree with root 
// at given index. This method assumes that the 
// subtrees are already heapified 
void MinHeap::MinHeapify(int i) 
{ 
    int l = left(i); 
    int r = right(i); 
    int smallest = i; 
    if (l < heap_size && harr[l].element < harr[i].element) 
        smallest = l; 
    if (r < heap_size && harr[r].element < harr[smallest].element) 
        smallest = r; 
    if (smallest != i) 
    { 
        swap(&harr[i], &harr[smallest]); 
        MinHeapify(smallest); 
    } 
} 
  
// A utility function to swap two elements 
void swap(MinHeapNode* x, MinHeapNode* y) 
{ 
    MinHeapNode temp = *x; 
    *x = *y; 
    *y = temp; 
}