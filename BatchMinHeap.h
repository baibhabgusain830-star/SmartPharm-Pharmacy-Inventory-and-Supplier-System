#ifndef BATCH_MIN_HEAP_H
#define BATCH_MIN_HEAP_H

#include "Batch.h"

// Custom Min-Heap prioritizing Batches by earliest expiry date (FEFO)
// Follows CLRS Binary Min-Heap specifications without any STL dynamic array container dependency.
class BatchMinHeap {
private:
    Batch* heapArray;
    int capacity;
    int currentSize;

    int parent(int i) const { return (i - 1) / 2; }
    int leftChild(int i) const { return 2 * i + 1; }
    int rightChild(int i) const { return 2 * i + 2; }

    void heapifyUp(int index);
    void heapifyDown(int index);
    void swapBatches(int i, int j);

public:
    explicit BatchMinHeap(int maxCap = 200);
    ~BatchMinHeap();

    BatchMinHeap(const BatchMinHeap& other);
    BatchMinHeap& operator=(const BatchMinHeap& other);

    bool insert(const Batch& batch);
    bool extractMin(Batch& extracted);
    bool peek(Batch& topBatch) const;

    bool isEmpty() const;
    int size() const;
    int getCapacity() const;
    void clear();

    void displayHeap() const;
};

#endif // BATCH_MIN_HEAP_H
