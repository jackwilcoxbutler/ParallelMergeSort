#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>


using namespace std;

void mergesort(int * a, int first, int last);
void smerge(int * a, int * b, int lasta, int lastb, int * output = NULL);

// returns the number of items less than valToFind in array a
int myrank(int * a, int first, int last, int valToFind);


void pmerge(int * a, int * b, int lasta, int lastb, int * output = NULL);

int main () {
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

    mergesort(a, 0 , asize - 1);

    cout << "a = [ ";
    for (int x = 0; x<asize - 1; x++)
        cout << a[x] << ", ";
    cout << a[asize-1] << " ]" << endl;


    int valToFind = 0;
    cout << "Rank to find: ";
    cin >> valToFind;
    
    int temp = myrank(a, 0, asize -1, valToFind);

    cout << "The rank of " << valToFind << " is " << temp << endl;

    // Stage 1 (Partitioning)
    int n = 0;
    
    int x = floor(n/log(n));








    return 0;
}

void pmerge(int * a, int * b, int lasta, int lastb, int * output)
{
    
}

void mergesort(int * a, int first, int last){
    if (first<last){//if a[] is longer than 1
        int middle = first + (last)/2;
        int lastb = last - middle - 1;
        mergesort(a, first, middle); // recurse first half
        mergesort(&a[middle + 1], 0, lastb); // recurse second half
        smerge(a, &a[middle + 1], middle, lastb, a);// merge two sorted arrays
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
    // cout << "lenC is " << lenC << endl;
    // cout << "Length of output is " << sizeof(output) <<  endl;
    //copy merged array to output
    if (output != NULL){
        for (int i = 0; i < lenC; i++)
        output[i] = c[i];
    }
    else {

    }
    

    delete [] c;
}
// returns # of items less than value
int myrank(int * a, int first, int last, int valToFind){
    // cout << "a = [ ";
    // for (int i = first; i<last; i++)
    //     cout << a[i] << ", ";
    // cout << a[last] << " ]" << endl;
    int val = 0;
    double n = last - first + 1;
    int x = ceil(n/2);
    // cout << "n = " << n << endl;
    // cout << "x = " << x << endl;
    // cout << "first = " << first << endl;
    // cout << "last = " << last << endl;
    if (n == 1){
        if (valToFind < a[first]) val = 0;
        else val = 1;
    } else {
        if (valToFind<a[first + x -1]) val = myrank(a, first, first + x-1,valToFind);
        if (valToFind>=a[first + x -1]) val = x + myrank(a, first + x, last,valToFind);
    }
    // cout << "returning: " << val << endl;
    return val;
}

