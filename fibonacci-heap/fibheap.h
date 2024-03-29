#pragma once

#ifndef HOJDAR_FIBHEAP_H
#define HOJDAR_FIBHEAP_H

#include <cassert>
#include <limits>

// \todo Remember to uncheck me!
// #define NDEBUG

class Logger {
   public:
    long long numberOfLogs = 0;
    long long cumulativeLog = 0;
    long long N = 0;

    void logStep(long long num) {
        cumulativeLog += num;
    }

    void logExtMin() {
        numberOfLogs++;
    }

    void rememberN(long long rn) {
        N = rn;
    }

    double getAverage() const {
        assert(numberOfLogs > 0);
        return double(cumulativeLog) / double(numberOfLogs);
    }

    void reset() {
        numberOfLogs = 0;
        cumulativeLog = 0;
    }

    bool init() const {
        return numberOfLogs > 0;
    }
};

struct FibNode {
    int id;
    int key;
    bool mark = false;
    int sonCount = 0;

    FibNode* parent = nullptr;
    FibNode* prevBro = nullptr;
    FibNode* nextBro = nullptr;
    FibNode* firstSon = nullptr;

    FibNode(int id_, int key_) : id(id_), key(key_) {
        nextBro = this;
        prevBro = this;
    };
};

class FibonacciHeap {
   public:
#define MAP_SIZE 2000000
    FibNode** mapa = new FibNode*[MAP_SIZE]();
    static_assert(MAP_SIZE >= 2000000, "Check if the map is big enough.");

    Logger log;

    bool naive = false;

    FibNode* cachedMin = nullptr;

    FibNode* firstTree = nullptr;

    FibNode* maxVal = new FibNode(std::numeric_limits<std::int32_t>::max(),
                                  std::numeric_limits<std::int32_t>::max());

    int numberOfTrees = 0;

    /// Append one 'item' to a cyclic list of nodes 'list'
    static FibNode* appendToList(FibNode* list, FibNode* item) {
        FibNode* start = list;
        if (start) {
            FibNode* end = start->prevBro;
            item->prevBro = end;
            item->nextBro = start;
            end->nextBro = item;
            start->prevBro = item;
        } else {
            start = item;
            start->prevBro = item;
            start->nextBro = item;
        }
        return start;
    }

    /// Append a tree into the forest
    void appendTree(FibNode* tree) {
        firstTree = appendToList(firstTree, tree);
        numberOfTrees++;
    }

    /// Delete the node 'x' from x->parent's sons cyclic list
    static void deleteMyselfFromSons(FibNode* x) {
        // \todo nullptr?
        FibNode* prevPtr = x->prevBro;
        FibNode* nextPtr = x->nextBro;
        prevPtr->nextBro = nextPtr;
        nextPtr->prevBro = prevPtr;
        if (x->parent && x->parent->firstSon == x) {
            x->parent->firstSon = nextPtr;
            if (nextPtr == x) {
                x->parent->firstSon = nullptr;
            }
        }
        x->nextBro = x;
        x->prevBro = x;
    }

    void cut(FibNode* x) {
        // 2. remember the parent
        FibNode* parent = x->parent;

        // 1. if x is a root, return
        if (!parent) {
            return;
        }

        // 3. delete the edge parent-x
        deleteMyselfFromSons(x);
        parent->sonCount -= 1;

        // 5. take x and make it a new tree by inserting into trees
        appendTree(x);

        // 4. reset the mark on x
        x->mark = false;
        x->parent = nullptr;  // x is a root

        // 6. consider parent's mark
        if (!parent->mark) {
            parent->mark = true;
        } else if (parent->mark && naive == false) {
            cut(parent);
        }
    }

    /// Delete one particular node by deleting it from its parent's sons list, then deleting the
    /// node, reducing parent's sons count and returning a CyclicList of newly formed trees without
    /// a root
    FibNode* deleteNode(FibNode* itemToDelete) {
        FibNode* newTrees = itemToDelete->firstSon;
        log.logStep(itemToDelete->sonCount);
        // \todo reset 'parent' atribute for newTrees?
        deleteMyselfFromSons(itemToDelete);
        if (itemToDelete->parent) {
            itemToDelete->parent->sonCount--;
        }
        mapa[itemToDelete->id] = nullptr;
        delete itemToDelete;
        return newTrees;
    }

    /// Merge two cyclic lists into one
    static FibNode* mergeLists(FibNode* mergeInto, FibNode* mergeFrom) {
        if (mergeFrom == nullptr) {
            return mergeInto;
        }
        if (mergeInto == nullptr) {
            return mergeFrom;
        }

        FibNode* intoEnd = mergeInto->prevBro;
        FibNode* fromEnd = mergeFrom->prevBro;
        intoEnd->nextBro = mergeFrom;
        fromEnd->nextBro = mergeInto;
        mergeInto->prevBro = fromEnd;
        mergeFrom->prevBro = intoEnd;
        return mergeInto;
    }

    static FibNode* addSon(FibNode* heap, FibNode* son) {
        heap->firstSon = appendToList(heap->firstSon, son);
        son->parent = heap;
        heap->sonCount++;
        return heap;
    }

    static FibNode* heapMerge(FibNode* heap1, FibNode* heap2) {
        // dvojku privesime pod jednicku
        if (heap1->key <= heap2->key) {
            heap1 = addSon(heap1, heap2);
            return heap1;
        } else if (heap1->key > heap2->key) {
            heap2 = addSon(heap2, heap1);
            return heap2;
        } else {
            assert(false);
            return nullptr;
        }
    }

    FibNode* consolidate() {
        numberOfTrees = 0;
        FibNode* returnVal = nullptr;
        if (firstTree == nullptr) {
            return returnVal;
        }

        FibNode* boxes[50];
        for (int i = 0; i < 50; ++i) {
            boxes[i] = nullptr;
        }

        // Fill all into boxes
        FibNode* current = firstTree;
        while (true) {
            // Work
            FibNode* nextVal = current->nextBro;
            deleteMyselfFromSons(current);
            boxes[current->sonCount] = appendToList(boxes[current->sonCount], current);

            // Next
            if (current == nextVal) {
                break;
            }
            current = nextVal;
        }

        // Go through all, merge
        for (int currentBox = 0; currentBox < 50; ++currentBox) {
            FibNode* node = boxes[currentBox];
            // Merge
            while (node != nullptr && node->nextBro != node) {
                FibNode* one = node;
                FibNode* two = node->nextBro;
                if (two->nextBro == one) {
                    boxes[currentBox] = nullptr;
                } else {
                    boxes[currentBox] = two->nextBro;
                }
                deleteMyselfFromSons(one);
                deleteMyselfFromSons(two);
                boxes[currentBox + 1] = appendToList(boxes[currentBox + 1], heapMerge(one, two));
                log.logStep(1);
                node = boxes[currentBox];
            }

            if (node != nullptr) {
                assert(node == node->nextBro);
                node->parent = nullptr;
                returnVal = appendToList(returnVal, node);
                numberOfTrees++;
                cachedMin = cachedMin->key > node->key ? node : cachedMin;
                boxes[currentBox] = nullptr;
            }
        }

        return returnVal;
    }

    FibNode* findById(int id) {
        return mapa[id];
    }

   public:
    /// Insert ID and KEY
    /// \param newNode first is ID, second is KEY
    FibNode* insert(std::pair<int, int> newNode) {
        if (mapa[newNode.first] != nullptr) {
            assert(false);
        }

        FibNode* newTree = new FibNode(newNode.first, newNode.second);
        mapa[newNode.first] = newTree;
        if (cachedMin == nullptr || newTree->key < cachedMin->key) {
            cachedMin = newTree;
        }
        appendTree(newTree);
        return newTree;
    }

    int extractMin() {
        if (firstTree == nullptr) {
            assert(false);
        }
        FibNode* minElement = cachedMin;
        const int retValue = minElement->key;
        cachedMin = maxVal;

        if (minElement == firstTree) {
            if (firstTree->nextBro == firstTree) {
                firstTree = nullptr;
            } else {
                firstTree = firstTree->nextBro;
            }
        }

        FibNode* newHeap = deleteNode(minElement);

        if (newHeap) {
            FibNode* cur = newHeap;
            while (true) {
                cur->parent = nullptr;
                if (cur != newHeap->prevBro) {
                    cur = cur->nextBro;
                } else {
                    break;
                }
            }
            firstTree = mergeLists(firstTree, newHeap);
        }

        firstTree = consolidate();

        log.logExtMin();
        return retValue;
    }

    void decrease(int idToDecrease, int newValue) {
        FibNode* nodeToDec = findById(idToDecrease);
        if (nodeToDec == nullptr) {
            return;
        }
        assert(nodeToDec->id == idToDecrease);
        assert(nodeToDec->key > newValue);
        nodeToDec->key = newValue;

        // Update the mininum
        if (nodeToDec->key < cachedMin->key) {
            cachedMin = nodeToDec;
        }

        // Cut if we break the heap invariant
        FibNode* parent = nodeToDec->parent;
        if (parent && parent->key > newValue) {
            cut(nodeToDec);
        }
    }

    void deleteItem(int idToDelete) {
        decrease(idToDelete, std::numeric_limits<std::int32_t>::min());
        int value = extractMin();
        assert(value == std::numeric_limits<std::int32_t>::min());
    }

    void reset() {
        while (firstTree != nullptr) {
            extractMin();
        }
        log.reset();
    }
};

#endif
