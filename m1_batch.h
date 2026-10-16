#ifndef BATCH_H
#define BATCH_H

#include <iostream>
using namespace std;

class Batch {
public:
    int batchId;
    int medId;
    int quantity;
    int expiryDate; // YYYYMMDD

    Batch(int bId = 0, int mId = 0, int q = 0, int exp = 0) {
        batchId = bId;
        medId = mId;
        quantity = q;
        expiryDate = exp;
    }
};

class MinHeap {
private:
    Batch heap[50];
    int size;

    void swap(Batch &a, Batch &b) {
        Batch temp = a;
        a = b;
        b = temp;
    }

public:
    MinHeap() { size = 0; }

    void insert(Batch b) {
        if (size >= 50) return;
        int i = size;
        heap[size++] = b;

        while (i != 0 && heap[(i - 1) / 2].expiryDate > heap[i].expiryDate) {
            swap(heap[i], heap[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }

    Batch getNearestExpiryBatch() {
        if (size <= 0) return Batch();
        return heap[0];
    }

    Batch extractMin() {
        if (size <= 0) return Batch();
        if (size == 1) return heap[--size];

        Batch root = heap[0];
        heap[0] = heap[--size];

        int i = 0;
        while (2 * i + 1 < size) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;

            if (left < size && heap[left].expiryDate < heap[smallest].expiryDate)
                smallest = left;
            if (right < size && heap[right].expiryDate < heap[smallest].expiryDate)
                smallest = right;

            if (smallest != i) {
                swap(heap[i], heap[smallest]);
                i = smallest;
            } else {
                break;
            }
        }
        return root;
    }
};

#endif