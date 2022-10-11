#include <mpi.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
using namespace std;

int sum_arr(int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[])
{
    int num_procs;
    int this_rank;
    int n = stoi(argv[1]);
    int max_subarr;
    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // make sure num_procs is power of 2
    double test = log2(num_procs);
    if (test - floor(test) != 0) {
        cout << "Please pass power-of-2 input" << endl;
        return 0;
    }

    // each process has dyn arr and partition index
    int* arr;
    int ind = 0;
    int size;

    if (this_rank == 0)
    {
        // create and fill array with random values [-1,1]
        arr = new int[n];
        int correct_result = 0;
        for (int i = 0; i < n; i++)
        {
            arr[i] = 1 - rand() % 3;
            correct_result += arr[i];
        }
        cout << "Serial result: " << correct_result << endl;
    }

    // MPI structs for send and receive calls
    MPI_Request req; 
    MPI_Status status;

    // one to all broadcast
    int d = log2(num_procs);
    int mask = pow(2, d) - 1;
    for (int i = d - 1; i >= 0; i--) {
        int op = pow(2, i);
        mask = (mask ^ op);
        int iter = d - i;
        int load_size = n / pow(2, iter);
        if ((this_rank & mask) == 0) {
            if ((this_rank & op) == 0) {
                int dest = (this_rank ^ op);
                MPI_Isend(arr + ind, load_size, MPI_INT, dest, 0, MPI_COMM_WORLD, &req);
                ind += load_size;
                size = load_size;
            } else {
                int src = (this_rank ^ op);
                arr = new int[load_size];
                MPI_Recv(arr, load_size, MPI_INT, src, 0, MPI_COMM_WORLD, &status);
                size = load_size;
            }
        }

    // compute sums
    int sum = sum_arr(arr + ind, size);

    // all to one reduce
    mask = 0;
    for (int i = 0; i < d; i++) {
        int op = pow(2, i);
        if ((this_rank & mask) == 0) {
            if ((this_rank & op) != 0) {
                int dest = (this_rank ^ op);
                MPI_Isend(&sum, 1, MPI_INT, dest, 0, MPI_COMM_WORLD, &req);
            } else {
                int src = (this_rank ^ op);
                int recv;
                MPI_Recv(&recv, 1, MPI_INT, src, 0, MPI_COMM_WORLD, &status);
                sum += recv;
            }
        }
        mask = (mask ^ op);
    }
   
   if (this_rank == 0) {
    cout << "Distributed result: " << sum << endl;
   }

    delete[] arr;

    MPI_Finalize();
}