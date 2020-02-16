// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* The emulator uses buffers at various places. Most of them are derived from
 * one of the following two classes:
 *
 *           RingBuffer : A standard ringbuffer data structure
 *     SortedRingBuffer : A ringbuffer that keeps the entries sorted
 */

#ifndef _BUFFERS_INC
#define _BUFFERS_INC

template <class T, int capacity> struct RingBuffer
{
    // Element storage
    T elements[capacity];

    // Read and write pointers
    int r, w;


    //
    // Constructing and initializing
    //

    RingBuffer() { clear(); }

    void clear() { r = w = 0; }


    //
    // Serializing
    //

    template <class W>
    void applyToItems(W& worker)
    {
        worker & elements & r & w;
    }


    //
    // Querying the fill status
    //

    int count() const { return (capacity + w - r) % capacity; }
    bool isEmpty() const { return r == w; }
    bool isFull() const { return count() == capacity - 1; }


    //
    // Working with indices
    //

    int begin() const { return r; }
    int end() const { return w; }
    static int next(int i) { return (capacity + i + 1) % capacity; }
    static int prev(int i) { return (capacity + i - 1) % capacity; }


    //
    // Reading and writing elements
    //

    T& read()
    {
        assert(!isEmpty());

        int oldr = r;
        r = next(r);
        return elements[oldr];
    }

    void write(T element)
    {
        assert(!isFull());

        int oldw = w;
        w = next(w);
        elements[oldw] = element;
    }


    //
    // Debugging
    //

    void dump()
     {
         printf("%d elements (r = %d w = %d):\n", count(), r, w);
         for (int i = r; i != w; i = next(i)) {
             assert(i < capacity);
             printf("%2i: ", i);
             elements[i].print();
         }
         printf("\n");
     }
};

template <class T, int capacity>
struct SortedRingBuffer : public RingBuffer<T, capacity>
{
    // Key storage
    int64_t keys[capacity];

    // Inserts an element at the right position
    void insert(int64_t key, T element)
    {
        assert(!this->isFull());

        // Add the new element
        int oldw = this->w;
        this->write(element);
        keys[oldw] = key;

        // Keep the elements sorted
        while (oldw != this->r) {

            // Get the index of the preceeding element
            int p = this->prev(oldw);

            // Exit the loop once we've found the correct position
            if (key >= keys[p]) break;

            // Otherwise, swap elements
            swap(this->elements[oldw], this->elements[p]);
            swap(keys[oldw], keys[p]);
            oldw = p;
        }
    }

    void dump()
    {
        printf("%d elements (r = %d w = %d):\n", this->count(), this->r, this->w);
        for (int i = this->r; i != this->w; i = this->next(i)) {
            assert(i < capacity);
            printf("%2i: [%lld] ", i, this->keys[i]);
            printf("%d\n", this->elements[i]);
            // this->elements[i].print();
        }
        printf("\n");
    }
};


/*
 *
 */

struct RegChange
{
    uint32_t addr;
    uint16_t value;

    template <class T>
    void applyToItems(T& worker)
    {
        worker & addr & value;
    }

    // Constructors
    RegChange() : addr(0), value(0) { }
    RegChange(uint32_t a, uint16_t v) : addr(a), value(v) { }

    void print()
    {
        printf("addr: %x value: %x\n", addr, value);
    }
};

/*
inline void test()
{
    SortedRingBuffer<RegChange, 128> buf;

    buf.dump();

    RegChange r1 = RegChange(1, 2);
    RegChange r2 = RegChange(2, 3);
    RegChange r3 = RegChange(3, 4);

    buf.insert(100, r1);
    buf.insert(200, r2);
    buf.insert(300, r3);

    buf.dump();

    (void)buf.read();

    buf.dump();

    RegChange r4 = RegChange(5, 6);
    buf.insert(50, r4);

    buf.dump();

    RegChange r5 = RegChange(7, 8);
    buf.insert(150, r5);

    buf.dump();

    RegChange r6 = RegChange(9, 10);
    RegChange r7 = RegChange(11, 12);
    RegChange r8 = RegChange(13, 14);
    RegChange r9 = RegChange(15, 16);

    buf.insert(300, r6);
    buf.insert(250, r7);
    buf.insert(350, r8);
    buf.insert(350, r9);

    buf.dump();

    printf("All elements up to 0:\n");
    for (int i = buf.begin(); i != buf.end() && buf.keys[i] <= 0; i = buf.next(i)) {
        printf("%2i: ", i); buf.elements[i].print();
    }
    printf("All elements up to 250:\n");
    for (int i = buf.begin(); i != buf.end() && buf.keys[i] <= 250; i = buf.next(i)) {
        printf("%2i: ", i); buf.elements[i].print();
    }
    printf("All elements up to 400:\n");
    for (int i = buf.begin(); i != buf.end() && buf.keys[i] <= 400; i = buf.next(i)) {
        printf("%2i: ", i); buf.elements[i].print();
    }
}
*/

#endif
