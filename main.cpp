#include <iostream>
#include <stdio.h>
#include <string.h>
#include "mpi.h" // message passing interface
using namespace std;

// New compile and run commands for MPI!
// mpicxx -o blah file.cpp
// mpirun -q -np 32 blah

void mergesort(int * a, int first, int last);
void smerge(int * a, int * b, int lasta, int lastb, int * output = NULL);

int main (int argc, char * argv[]) {

	int my_rank;			// my CPU number for this process
	int p;					// number of CPUs that we have
	int source;				// rank of the sender
	int dest;				// rank of destination
	int tag = 0;			// message number
	char message[100];		// message itself
	MPI_Status status;		// return status for receive
	
	// Start MPI
	MPI_Init(&argc, &argv);
	
	// Find out my rank!
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	// Find out the number of processes!
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	
	// THE REAL PROGRAM IS HERE
	//if (my_rank == 0){
        int seed =  71911;
        srand(seed);
        int asize = 0;
        while (asize <= 0){
            cout << "Select the size of array: ";
            cin >> asize;
        }
        int * a = new int[asize];

        // fill in array
        for (int x = 0; x<asize; x++)
            a[x] = rand() % 100;
        
        cout << "a = [ ";
        for (int x = 0; x<asize - 1; x++)
            cout << a[x] << ", ";
        cout << a[asize-1] << " ]" << endl;
    //}
    MPI_Bcast(a ,asize ,MPI_INT, 0, MPI_COMM_WORLD);

	// Shut down MPI
	MPI_Finalize();

	return 0;
}

void mergesort(int * a, int first, int last){
    if (first<last){//if a[] is longer than 1
        int middle = first + (last)/2;
        int lastb = last - middle - 1;
        mergesort(a, first, middle); // recurse first half
        mergesort(&a[middle + 1], 0, lastb); // recurse second half
        smerge(a, &a[middle + 1], middle, lastb);// merge two sorted arrays
    }
    return;
}
void smerge(int * a, int * b, int lasta, int lastb, int * output){
    int indexA = 0;
    int indexB = 0;
    int lenA = lasta + 1;
    int lenB = lastb + 1;

    int indexC = 0;
    int lenC = lenA + lenB;
    int * c = new int[lenC];

    //fill array c with next lowest of a[] and b[]
    while (indexA < lenA && indexB < lenB){
        if (a[indexA] <= b[indexB]){
            c[indexC] = a[indexA];
            indexA++;
        } else {
            c[indexC] = b[indexB];
            indexB++;
        }
        indexC++;
    }

    //fill array c with remaining values
    //  will enter exactly one of the loops
    while(indexA < lenA){
        c[indexC] = a[indexA];
        indexA++;
        indexC++;
    }
    while(indexB < lenB){
        c[indexC] = b[indexB];
        indexB++;
        indexC++;
    }

    //copy merged array back to a
    for (int i = 0; i < lenC; i++)
        a[i] = c[i];

    delete [] c;
}






