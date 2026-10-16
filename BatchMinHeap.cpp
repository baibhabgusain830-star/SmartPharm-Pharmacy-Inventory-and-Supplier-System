#include "BatchMinHeap.h"
#include <iostream>
#include <iomanip>

BatchMinHeap::BatchMinHeap(int maxCap) 
    : heapArray(nullptr), capacity(maxCap > 0 ? maxCap : 200), currentSize(0) {
    heapArray = new Batch[capacity];
}

BatchMinHeap::~BatchMinHeap() {
    delete[] heapArray;
    heapArray = nullptr;
}

BatchMinHeap::BatchMinHeap(const BatchMinHeap& other) 
    : heapArray(nullptr), capacity(other.capacity), currentSize(other.currentSize) {
    heapArray = new Batch[capacity];
    for (int i = 0; i < currentSize; ++i) {
        heapArray[i] = other.heapArray[i];
    }
}

BatchMinHeap& BatchMinHeap::operator=(const BatchMinHeap& other) {
    if (this != &other) {
        delete[] heapArray;
        capacity = other.capacity;
        currentSize = other.currentSize;
        heapArray = new Batch[capacity];
        for (int i = 0; i < currentSize; ++i) {
            heapArray[i] = other.heapArray[i];
        }
    }
    return *this;
}

void BatchMinHeap::swapBatches(int i, int j) {
    Batch temp = heapArray[i];
    heapArray[i] = heapArray[j];
    heapArray[j] = temp;
}

void BatchMinHeap::heapifyUp(int index) {
    // Parent must have earlier or equal expiry date than child
    while (index > 0) {
        int p = parent(index);
        if (DateUtils::compareDates(heapArray[index].getExpiryDate(), heapArray[p].getExpiryDate()) < 0) {
            swapBatches(index, p);
            index = p;
        } else {
            break;
        }
    }
}

void BatchMinHeap::heapifyDown(int index) {
    int smallest = index;
    int left = leftChild(index);
    int right = rightChild(index);

    if (left < currentSize && 
        DateUtils::compareDates(heapArray[left].getExpiryDate(), heapArray[smallest].getExpiryDate()) < 0) {
        smallest = left;
    }

    if (right < currentSize && 
        DateUtils::compareDates(heapArray[right].getExpiryDate(), heapArray[smallest].getExpiryDate()) < 0) {
        smallest = right;
    }

    if (smallest != index) {
        swapBatches(index, smallest);
        heapifyDown(smallest);
    }
}

bool BatchMinHeap::insert(const Batch& batch) {
    if (!batch.isValid()) {
        return false;
    }
    if (currentSize >= capacity) {
        std::cerr << "[Error] BatchMinHeap capacity reached. Cannot insert.\n";
        return false;
    }

    heapArray[currentSize] = batch;
    heapifyUp(currentSize);
    currentSize++;
    return true;
}

bool BatchMinHeap::extractMin(Batch& extracted) {
    if (currentSize <= 0) {
        return false;
    }

    extracted = heapArray[0];
    heapArray[0] = heapArray[currentSize - 1];
    currentSize--;

    if (currentSize > 0) {
        heapifyDown(0);
    }
    return true;
}

bool BatchMinHeap::peek(Batch& topBatch) const {
    if (currentSize <= 0) {
        return false;
    }
    topBatch = heapArray[0];
    return true;
}

bool BatchMinHeap::isEmpty() const {
    return currentSize == 0;
}

int BatchMinHeap::size() const {
    return currentSize;
}

int BatchMinHeap::getCapacity() const {
    return capacity;
}

void BatchMinHeap::clear() {
    currentSize = 0;
}

void BatchMinHeap::displayHeap() const {
    if (currentSize == 0) {
        std::cout << " (Min-Heap is currently empty)\n";
        return;
    }
    std::cout << " --- FEFO Batch Min-Heap Queue (Earliest Expiry First) ---\n";
    std::cout << std::left 
              << std::setw(6)  << "Idx" 
              << std::setw(12) << "Batch ID" 
              << std::setw(14) << "Medicine ID" 
              << std::setw(14) << "Expiry Date" 
              << std::setw(10) << "Quantity" 
              << "Cost\n";
    std::cout << "---------------------------------------------------------\n";
    for (int i = 0; i < currentSize; ++i) {
        std::cout << "[" << std::setw(3) << i << "] ";
        heapArray[i].display();
    }
}
