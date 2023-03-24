#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mpi.h> // message passing interface
using namespace std;

// New compile and run commands for MPI!
// mpicxx -o blah file.cpp
// mpirun -q -np 32 blah

// void mergesort(int * a, int first, int last);

void pmergesort(int * a, int first, int last);

void smerge(int * a, int * b, int lasta, int lastb, int * output = NULL);

// returns the number of items less than valToFind in array a
int myrank(int * a, int first, int last, int valToFind);

void pmerge(int * a, int * b, int lasta, int lastb, int * output = NULL);

int main (int argc, char * argv[]){

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
    int size = 0;

    if(my_rank == 0){
        cout << "Enter a size for your array : ";
        cin >> size;
    }

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);              
    

    int * a = new int[size];
	if (my_rank == 0){
        int seed =  71911;
        srand(seed);
        
        // fill in array
        for (int x = 0; x<size; x++)
            a[x] = rand() % 500;

        cout << "Before Parallel-MergeSort:\n";
        cout << " a = [ \t";
        for (int x = 0; x<size - 1; x++){
            cout << a[x] << ", ";
            if (x % 32 == 31)
                cout << "\n\t";
        }
            
        cout << a[size-1] << " ]" << endl << endl;
    
    }

    MPI_Bcast(a, size, MPI_INT, 0, MPI_COMM_WORLD);

    pmergesort(a, 0, size-1);
    
    if(my_rank == p-1){
        cout << "Final Output brought to you by process " << p-1 << ":" << endl;
        cout << " a = [ \t";
        for (int x = 0; x<size - 1; x++){
            cout << a[x] << ", ";
            if (x % 32 == 31)
                cout << "\n\t";
        }
        cout << a[size-1] << " ]" << endl;
    }

    // Delete
    delete [] a;

	// Shut down MPI
	MPI_Finalize();

	return 0;
}

void pmerge(int * a, int * b, int lasta, int lastb, int * output)
{
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    int p;
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // # of elements of a
    int n = lasta + 1;
    // # of elements of b
    int m = lastb + 1;

    if (output == NULL)
        output = new int[n+m];
    // Temperary array as to not overright a and b before neccessary
    int * output_temp = new int[n+m];
    // Local versions so that MPI_Allreduce works
    int output_local[n+m]= {};
    int ogIndex_local[n+m] = {}; 
    bool originating_local[n+m] = {};
    //Number of indexes between ranks to find for a[]
    int aGap = max(1, (int)log2(n));
    //Number of indexes between ranks to find for b[]
    int bGap = max(1, (int)log2(m));

    int aRanksToFind = n/aGap;
    int bRanksToFind = m/bGap;

    // int  aIndexesToFind[aRanksToFind] = {};
    // for (int i= 0;i<aRanksToFind;i++)
    //     aIndexesToFind[i]= i*aGap;
    
    // int  bIndexesToFind[bRanksToFind] = {};
    // for (int i= 0;i<bRanksToFind;i++)
    //     bIndexesToFind[i]= i*bGap;

    //A elements ranks in B
    int SRANKA_local[aRanksToFind] = {};

    //A elements ranks in B
    int SRANKB_local[bRanksToFind] = {};

    // A process will find all the ranks where my_rank = 
    for (int i = my_rank; i<aRanksToFind; i+=p){
        int valToFind = a[i * aGap];
        int temp = myrank(b, 0, lastb, valToFind);
        SRANKA_local[i] = temp;
        originating_local[i * aGap + temp] = 0;
        ogIndex_local[i * aGap + temp] = i * aGap;
        output_local[i * aGap + temp] = valToFind;
    }
    
    for (int i = my_rank; i<bRanksToFind; i+=p){
        int valToFind = b[i * bGap];
        int temp = myrank(a, 0, lasta, valToFind);
        SRANKB_local[i] = temp;
        originating_local[i * bGap + temp] = 1;
        ogIndex_local[i * bGap + temp] = i * bGap;
        output_local[i * bGap + temp] = valToFind;
    }

    bool originating[n+m] = {};
    int ogIndex[n+m] = {};
    MPI_Allreduce(&output_local, output_temp, n+m, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&originating_local, &originating, n+m, MPI_C_BOOL, MPI_LXOR, MPI_COMM_WORLD);
    MPI_Allreduce(&ogIndex_local, &ogIndex, n+m, MPI_INT, MPI_SUM, MPI_COMM_WORLD);


    // positions in output that we have already filled
    int positions[aRanksToFind + bRanksToFind] = {};    
    if(my_rank == 0){
        int y = 0;
        for (int x = 0; x<n+m; x++)
            if (output_temp[x] != 0){
                positions[y] = x;
                y++;
            }
    }
    MPI_Bcast(&positions, aRanksToFind + bRanksToFind, MPI_INT, 0, MPI_COMM_WORLD);

    //fill in shapes by finding the 4 indexes that bound them and smerging
    for (int i = my_rank; i<aRanksToFind + bRanksToFind; i+=p){
        //shape "corners"
        int a1index, a2index, b1index, b2index;
        //lines
        int top = positions[i];
        int bottom;
        if (i == aRanksToFind + bRanksToFind - 1)
            bottom = n+m-1;
        else bottom = positions[i+1];
        
        if (bottom - top > 0){
            if (originating[top] == 1){
                b1index = ogIndex[top] + 1;
                a1index = top - ogIndex[top];
            } else {
                a1index = ogIndex[top] + 1;
                b1index = top - ogIndex[top];
            }
            if (originating[bottom] == 1){
                b2index = ogIndex[bottom] - 1;
                a2index = bottom - ogIndex[bottom] - 1;
            } else {
                a2index = ogIndex[bottom] - 1;
                b2index = bottom - ogIndex[bottom] - 1;
            }
            if (i == aRanksToFind + bRanksToFind - 1){
                a2index = n - 1;
                b2index = m - 1;
            }
    
            smerge(&a[a1index], &b[b1index], a2index - a1index, b2index - b1index, &output_temp[top + 1]);
    
        }
    }
    MPI_Allreduce(output_temp, output, n+m, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    /*
    Information for creating shapes

    originating array (a/b) a = 0, b = 1
    original index 
    corispoding rank
    output rank/index
    */

    delete [] output_temp;
}

// //first and last are index #s
// void mergesort(int * a, int first, int last){
//     if (first<last){//if a[] is longer than 1
//         int middle = (first + last)/2;
//         int lastA = middle - first;
//         int firstB = middle + 1;
//         int lastB = last - firstB;
//         mergesort(&a[first], 0, lastA); // recurse first half
//         mergesort(&a[firstB], 0, lastB); // recurse second half
//         smerge(&a[first], &a[firstB], lastA, lastB, &a[first]);// merge two sorted arrays
//     }
//     return;
// }

void pmergesort(int * a, int first, int last){
    if (first<last){//if a[] is longer than 1
        int middle = (first + last)/2;
        int lastA = middle - first;
        int firstB = middle + 1;
        int lastB = last - firstB;

        pmergesort(&a[first], 0, lastA); // recurse first half
        pmergesort(&a[firstB], 0, lastB); // recurse second half
        pmerge(&a[first], &a[firstB], lastA, lastB, &a[first]);// merge two sorted arrays
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
    int c[lenC] = {};

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

    for (int i = 0; i < lenC; i++)
        output[i] = c[i];

}

int myrank(int * a, int first, int last, int valToFind){
    int val = 0;
    double n = last - first + 1;
    int x = ceil(n/2);
    
    if (n == 1){
        if (valToFind < a[first]) val = 0;
        else val = 1;
    } else {
        if (valToFind<a[first + x -1]) val = myrank(a, first, first + x-1,valToFind);
        if (valToFind>=a[first + x -1]) val = x + myrank(a, first + x, last,valToFind);
    }

    return val;
}


/*
run with seed 71909
thomas% mpirun -q -np 16 blah
Enter a size for your array : 128
Before Parallel-MergeSort:
 a = [  254, 397, 169, 133, 389, 419, 192, 36, 338, 484, 115, 399, 376, 20, 196, 99, 110, 424, 244, 171, 91, 303, 88, 209, 239, 490, 262, 436, 441, 281, 168, 195, 
        178, 337, 328, 420, 108, 372, 456, 447, 357, 423, 346, 85, 443, 42, 36, 53, 466, 280, 76, 409, 435, 165, 470, 174, 7, 84, 111, 448, 366, 279, 143, 396, 
        469, 323, 316, 429, 47, 124, 376, 256, 47, 74, 342, 491, 468, 230, 396, 434, 363, 473, 195, 298, 138, 165, 473, 145, 250, 436, 93, 468, 67, 88, 216, 388, 
        263, 385, 318, 162, 9, 194, 419, 409, 269, 113, 252, 89, 343, 148, 24, 58, 121, 71, 357, 111, 237, 182, 108, 339, 470, 53, 159, 37, 493, 375, 426, 256 ]

Final Output brought to you by process 15:
 a = [  0, 7, 9, 20, 24, 36, 36, 37, 42, 47, 53, 53, 58, 67, 71, 74, 76, 84, 85, 88, 88, 89, 91, 93, 99, 108, 108, 110, 111, 111, 113, 115, 
        121, 124, 133, 138, 143, 145, 148, 159, 162, 165, 165, 168, 169, 171, 174, 178, 182, 192, 194, 195, 195, 196, 209, 216, 230, 237, 239, 244, 250, 252, 254, 256, 
        256, 262, 263, 269, 279, 280, 281, 298, 303, 316, 318, 323, 328, 337, 338, 339, 342, 343, 346, 357, 357, 363, 366, 372, 375, 376, 376, 385, 388, 389, 396, 396, 
        397, 399, 409, 409, 419, 419, 420, 423, 424, 426, 429, 434, 435, 436, 436, 441, 443, 447, 448, 456, 466, 468, 468, 469, 470, 470, 473, 473, 484, 490, 491, 493 ]


run with seed 71911
thomas% mpirun -q -np 16 blah
Enter a size for your array : 128
Before Parallel-MergeSort:
 a = [  117, 250, 136, 29, 31, 199, 391, 271, 140, 326, 217, 467, 165, 197, 9, 256, 442, 136, 277, 251, 365, 121, 432, 375, 167, 325, 211, 385, 149, 56, 17, 118, 
        307, 153, 147, 190, 353, 390, 313, 493, 69, 383, 312, 234, 80, 321, 342, 22, 458, 120, 273, 175, 93, 58, 50, 113, 235, 113, 350, 384, 170, 367, 2, 329, 
        21, 2, 19, 226, 244, 184, 71, 313, 67, 236, 400, 148, 57, 94, 170, 367, 66, 296, 42, 160, 206, 444, 273, 441, 58, 123, 325, 80, 343, 179, 409, 364, 
        181, 280, 442, 278, 464, 365, 443, 32, 101, 343, 180, 11, 438, 202, 378, 4, 350, 273, 16, 56, 217, 141, 497, 127, 117, 174, 59, 460, 206, 468, 176, 239 ]

Final Output brought to you by process 15:
 a = [  0, 2, 4, 9, 11, 16, 17, 19, 21, 22, 29, 31, 32, 42, 50, 56, 56, 57, 58, 58, 59, 66, 67, 69, 71, 80, 80, 93, 94, 101, 113, 113, 
        117, 117, 118, 120, 121, 123, 127, 136, 136, 140, 141, 147, 148, 149, 153, 160, 165, 167, 170, 170, 174, 175, 176, 179, 180, 181, 184, 190, 197, 199, 202, 206, 
        206, 211, 217, 217, 226, 234, 235, 236, 239, 244, 250, 251, 256, 271, 273, 273, 273, 277, 278, 280, 296, 307, 312, 313, 313, 321, 325, 325, 326, 329, 342, 343, 
        343, 350, 350, 353, 364, 365, 365, 367, 367, 375, 378, 383, 384, 385, 390, 391, 400, 409, 432, 438, 441, 442, 442, 443, 444, 458, 460, 464, 467, 468, 493, 497 ]

*/





