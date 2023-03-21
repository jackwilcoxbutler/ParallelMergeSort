#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mpi.h> // message passing interface
using namespace std;

// New compile and run commands for MPI!
// mpicxx -o blah file.cpp
// mpirun -q -np 32 blah

void mergesort(int * a, int first, int last);
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
    int size = 64;
    int half = size/2;
    int * a = new int[size];
	if (my_rank == 0){
        int seed =  71911;
        srand(seed);
        // int asize = 0;
        // while (asize <= 0){
        //     cout << "Select the size of array: ";
        //     cin >> asize;
        // }
        

        // fill in array
        for (int x = 0; x<size; x++)
            a[x] = rand() % 100;


        cout << "a = [ ";
        for (int x = 0; x<size - 1; x++)
            cout << a[x] << ", ";
        cout << a[size-1] << " ]" << endl << endl;

        // mergesort(a, 0, half-1);
        // mergesort(a, half, size-1);
    }
    MPI_Bcast(a, size, MPI_INT, 0, MPI_COMM_WORLD);

    // int * b = &a[half];

    // if (my_rank == 0){
    //     cout << "a = [ ";
    //     for (int x = 0; x<half-1; x++)
    //         cout << a[x] << ", ";
    //     cout << a[half-1] << " ]" << endl;

    //     cout << "b = [ ";
    //     for (int x = 0; x<half-1; x++)
    //         cout << b[x] << ", ";
    //     cout << b[half-1] << " ]" << endl;
    // }
    

    int * output = new int[size];

    pmergesort(a, 0, size-1);
    
    // for (int i = 0; i < size; i++){
    //     output[i] = 0;
    // }

    if(my_rank == p-1){
        cout << "From Process " << p-1 << " : " << endl;
        cout << "\ta = [ ";
        for (int x = 0; x<size - 1; x++)
            cout << a[x] << ", ";
        cout << a[size-1] << " ]" << endl;
    }

    //pmerge(a, b, half-1, half-1, output);

    // // cout << "hello" << endl;

    // int output2[size] = {};
    // MPI_Allreduce( output, &output2, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    // //MPI_Allreduce( const void* sendbuf , void* recvbuf , MPI_Count count , MPI_Datatype datatype , MPI_Op op , MPI_Comm comm);
    //cout << "hello" << endl;

    // if(my_rank == 0){
    //     cout << "From Process " << my_rank << " : " << endl;
    //     cout << "** Final Output ** = \t[ ";
    //     for (int x = 0; x<size - 1; x++)
    //         cout << output[x] << ", ";
    //     cout << output[size-1] << " ]" << endl;
    // }

    //Delete
    delete [] a;
    delete [] output;
    //delete [] output2;


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


    if (my_rank == 0){
        cout << "a at begining = [ ";
        for (int x = 0; x<lasta; x++)
            cout << a[x] << ", ";
        cout << a[lasta] << " ]" << endl;

        cout << "b at begining = [ ";
        for (int x = 0; x<lastb; x++)
            cout << b[x] << ", ";
        cout << b[lastb] << " ]" << endl;
    }
    
    // n -> a 
    int n = lasta + 1;
    // m -> b
    int m = lastb + 1;

    if (output == NULL)
        output = new int[n+m];
    int * output_temp = new int[n+m];
    int output_local[n+m] = {}; 
    bool originating_local[n+m] = {};
    int ogIndex_local[n+m] = {};

    int aGap = max(1, (int)log2(n));
    int bGap = max(1, (int)log2(m));

    int aRanksToFind = n/aGap;
    int bRanksToFind = m/bGap;
    // if(my_rank == 0){
    //     cout << "log2(n) = " << log2(n) << endl;
    //     cout << "n = " << n << endl;
    //     cout << "aRanksToFind = " << aRanksToFind << endl;
    // }

    int  aIndexesToFind[aRanksToFind] = {};
    for (int i= 0;i<aRanksToFind;i++)
        aIndexesToFind[i]= i*aGap;
    
    int  bIndexesToFind[bRanksToFind] = {};
    for (int i= 0;i<bRanksToFind;i++)
        bIndexesToFind[i]= i*bGap;
    

    if(my_rank == 0){
        cout << "aIndexesToFind = [ ";
        for (int i = 0; i<aRanksToFind-1; i++)
            cout << aIndexesToFind[i] << ", ";
        cout << aIndexesToFind[aRanksToFind-1] << " ]" << endl;
    }

    // if (my_rank < aRanksToFind){
    //     int valToFind = a[my_rank * aGap];
    //     int temp = myrank(b, 0, lastb, valToFind);
    //     output[my_rank * aGap + temp] = valToFind;
    // }

    //A elements ranks in B
    int SRANKA_local[aRanksToFind] = {};

    //A elements ranks in B
    int SRANKB_local[bRanksToFind] = {};


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

    // cout << "SRANKA_local = [ ";
    // for (int i = 0; i<aRanksToFind-1; i++)
    //     cout << SRANKA_local[i] << ", ";
    // cout << SRANKA_local[aRanksToFind-1] << " ]" << endl;

    // int SRANKA[aRanksToFind] = {};
    // MPI_Allreduce(SRANKA_local, SRANKA, aRanksToFind, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    // int SRANKB[bRanksToFind] = {};
    // MPI_Allreduce(SRANKB_local, SRANKB, bRanksToFind, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // if(my_rank == 0){
    //     cout << "SRANKA = [ ";
    //     for (int i = 0; i<aRanksToFind-1; i++)
    //         cout << SRANKA[i] << ", ";
    //     cout << SRANKA[aRanksToFind-1] << " ]" << endl;
    // }
    // if(my_rank == 0){
    //     cout << "SRANKB = [ ";
    //     for (int i = 0; i<bRanksToFind-1; i++)
    //         cout << SRANKB[i] << ", ";
    //     cout << SRANKB[bRanksToFind-1] << " ]" << endl;
    // }

    bool originating[n+m] = {};
    int ogIndex[n+m] = {};
    MPI_Allreduce(&output_local, output_temp, n+m, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&originating_local, &originating, n+m, MPI_C_BOOL, MPI_LXOR, MPI_COMM_WORLD);
    MPI_Allreduce(&ogIndex_local, &ogIndex, n+m, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // if(my_rank == 0){
    //     cout << "From Process " << my_rank << " : " << endl;
    //     cout << "\toriginating = \t[ ";
    //     for (int x = 0; x<n+m - 1; x++)
    //         cout << originating[x] << ", ";
    //     cout << originating[n+m-1] << " ]" << endl;
    // }
    // if(my_rank == 0){
    //     cout << "From Process " << my_rank << " : " << endl;
    //     cout << "\togIndex = \t[ ";
    //     for (int x = 0; x<n+m - 1; x++)
    //         cout << ogIndex[x] << ", ";
    //     cout << ogIndex[n+m-1] << " ]" << endl;
    // }
    if(my_rank == 0){
        cout << "From Process " << my_rank << " : " << endl;
        cout << "\toutput before shapes = \t[ ";
        for (int x = 0; x<n+m - 1; x++)
            cout << output_temp[x] << ", ";
        cout << output_temp[n+m-1] << " ]" << endl;
    }

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

    if(my_rank == 0){
        cout << "From Process " << my_rank << " : " << endl;
        cout << "\tpositions = \t[ ";
        for (int x = 0; x<aRanksToFind + bRanksToFind - 1; x++)
            cout << positions[x] << ", ";
        cout << positions[aRanksToFind + bRanksToFind-1] << " ]" << endl;
    }
    for (int i = my_rank; i<aRanksToFind + bRanksToFind; i+=p){
        //shape "corners"
        int a1index, a2index, b1index, b2index;
        //lines

        int top = positions[i];
        int bottom;
        if (i == aRanksToFind + bRanksToFind - 1)
            bottom = n+m-1;
        else bottom = positions[i+1];
        cout << "i = " << i << endl;
        cout << "top = " << top << endl;
        cout << "bottom = " << bottom << endl;
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
            if (my_rank == 0){
                cout << "a before smerge = [ ";
                for (int x = 0; x<lasta; x++)
                    cout << a[x] << ", ";
                cout << a[lasta] << " ]" << endl;

                cout << "b before smerge = [ ";
                for (int x = 0; x<lastb; x++)
                    cout << b[x] << ", ";
                cout << b[lastb] << " ]" << endl;
            }
            
            cout << "b1index = " << b1index << endl;
            cout << "a1index = " << a1index << endl;
            cout << "b2index = " << b2index << endl;
            cout << "a2index = " << a2index << endl;
            smerge(&a[a1index], &b[b1index], a2index - a1index, b2index - b1index, &output_temp[top + 1]);
            // if(my_rank == 0){
            //     cout << "From Process " << my_rank << " : " << endl;
            //     cout << "\tshape = \t[ ";
            //     for (int x = top + 1; x<top + 1 + (a2index - a1index ) + (b2index - b1index) + 1; x++)
            //         cout << output[x] << ", ";
            //     if (top + 1 + (a2index - a1index ) + (b2index - b1index) + 1 > 0) cout << output[top + 1 + (a2index - a1index ) + (b2index - b1index) + 1];
            //     cout << " ]" << endl;
            // }
        }
    }

    int temp[n+m] = {};
    //MPI_Allreduce(output_temp, &temp, n+m, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(output_temp, output, n+m, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    /*
    Information for creating shapes

    originating array (a/b) a = 0, b = 1
    original index 
    corispoding rank
    output rank/index

    
    
    */
   if(my_rank == 0){
        cout << "From Process " << my_rank << " : " << endl;
        cout << "\toutput after shapes = \t[ ";
        for (int x = 0; x<n+m - 1; x++)
            cout << output[x] << ", ";
        cout << output[n+m-1] << " ]" << endl;
    }


    delete [] output_temp;
}

//first and last are index #s
void mergesort(int * a, int first, int last){
    if (first<last){//if a[] is longer than 1
        int middle = (first + last)/2;
        int lastA = middle - first;
        int firstB = middle + 1;
        int lastB = last - firstB;
        mergesort(&a[first], 0, lastA); // recurse first half
        mergesort(&a[firstB], 0, lastB); // recurse second half
        smerge(&a[first], &a[firstB], lastA, lastB, &a[first]);// merge two sorted arrays
    }
    return;
}
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

    cout << "\tA in smerge = \t[ ";
    for (int x = 0; x<lenA - 2; x++)
        cout << a[x] << ", ";
    if (lenA>0) cout << a[lenA-1];
    cout << " ]" << endl;
    cout << "\tB in smerge = \t[ ";
    for (int x = 0; x<lenB - 2; x++)
        cout << b[x] << ", ";
    if (lenB>0) cout << b[lenB-1];
    cout << " ]" << endl;


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

    //copy merged array back to a
    // if (output == NULL){
    //     int * output = new int[lenC];
    // }
    for (int i = 0; i < lenC; i++)
        output[i] = c[i];

    cout << "\t output in smerge = \t[ ";
    for (int x = 0; x<lenC - 1; x++)
        cout << output[x] << ", ";
    if (lenC > 0) cout << output[lenC-1];
    cout << " ]" << endl;
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





