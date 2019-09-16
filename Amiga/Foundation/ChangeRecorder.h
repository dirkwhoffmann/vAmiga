// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CHANGE_RECORDER_INC
#define _CHANGE_RECORDER_INC

struct Change
{
    int64_t trigger;
    uint32_t addr;
    uint16_t value;

    template <class T>
    void applyToItems(T& worker)
    {
        worker

        & trigger
        & addr
        & value;
    }

    // Constructors
    Change() : trigger(0), addr(0), value(0) { }
    Change(int64_t t, uint32_t a, uint16_t v) : trigger(t), addr(a), value(v) { }

    void print()
    {
        printf("trigger: %lld addr: %x value: %x\n", trigger, addr, value);
    }
};

template <uint16_t capacity> struct ChangeRecorder
{
    // Ringbuffer elements
    Change change[capacity];

    // Ringbuffer read pointer
    uint16_t r;

    // Ringbuffer write pointer
    uint16_t w;

    // Constructor
    ChangeRecorder() : r(0), w(0) { printf("Constructor\n"); dumpAll(); }

    // Moves a pointer back or forth
    static uint16_t advance(uint16_t p, int offset) { return (capacity + p + offset) % capacity; }
    static uint16_t next(uint16_t p) { return advance(p, 1); }
    static uint16_t prev(uint16_t p) { return advance(p, -1); }

    template <class T>
    void applyToItems(T& worker)
    {
        worker

        & change
        & r
        & w;
    }

    // Returns the index of the first element
    uint16_t begin() { return r; }

    // Returns the index of the last element advanced by 1
    uint16_t end() { return w; }

    // Returns the number of stored elements
    uint16_t count() { return (capacity + w - r) % capacity; } const

    // Indicates if the buffer is empty or full
    bool isEmpty() { return r == w; }
    bool isFull() { return count() == capacity - 1; }

    // Queries the next element to read
    Cycle trigger() { return isEmpty() ? NEVER : change[r].trigger; }
    uint32_t addr() { assert(!isEmpty()); return change[r].addr; }
    uint16_t value() { assert(!isEmpty()); return change[r].value; }

    // Adds an element
    void add(int64_t trigger, uint32_t addr, uint16_t value)
    {
        assert(!isFull());

        // Remember where the new element will be added
        uint16_t e = w;

        // Add the new element
        change[w] = Change(trigger, addr, value);
        w = next(w);

        // Keep the elements sorted
        while (e != r) {

            // Get the index of the preceeding element
            uint16_t p = prev(e);

            // Exit the loop once we've found the correct position
            if (trigger >= change[p].trigger) break;

            swap(change[e], change[p]);
            e = p;
        }
    }

    // Removes an element 
    void remove() { assert(!isEmpty()); r = next(r); }

    // Deletes all elements
    void clear() { r = w = 0; }

    // Prints some debug info
    void dump()
    {
        printf("Buffer with %d elements (r = %d w = %d):\n", count(), r, w);
        for (int i = r; i != w; i = next(i)) {
            assert(i < capacity);
            printf("%2i: ", i);
            change[i].print();
        }
        printf("\n");
    }
    void dumpAll()
    {
        printf("All elements (%d used)\n", count());

        for (int i = 0; i < capacity; i++) {
            printf("%2i: ", i);
            change[i].print();
        }
        printf("\n");
    }

    void test()
    {
        dump();

        add(100, 1, 2);
        add(200, 2, 3);
        add(300, 3, 4);

        dump();

        remove();

        dump();

        add(50, 5, 6);

        dump();

        add(150, 7, 8);

        dump(); 

        add(300, 9, 10);
        add(250, 11, 12);
        add(350, 11, 12);
        add(350, 11, 12);

        dump();

        printf("All elements up to 0:\n");
        for (uint16_t i = begin(); i != end() && change[i].trigger <= 0; i = next(i)) {
            printf("%2i: ", i); change[i].print();
        }
        printf("All elements up to 250:\n");
        for (uint16_t i = begin(); i != end() && change[i].trigger <= 250; i = next(i)) {
            printf("%2i: ", i); change[i].print();
        }
        printf("All elements up to 400:\n");
        for (uint16_t i = begin(); i != end() && change[i].trigger <= 400; i = next(i)) {
            printf("%2i: ", i); change[i].print();
        }
    }
};

#endif
