#ifndef HW4_STANDARD_COMMUNICATION_H
#define HW4_STANDARD_COMMUNICATION_H

class StandardCommunication {
public:
    // Must be called by every process after MPI_Init.
    static void add();
    static void add(bool optimized);
};

#endif
