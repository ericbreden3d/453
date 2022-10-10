
#include <mpi.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
using namespace std;

int sum_arr(int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

void get_dim_counts(int m, MPI_Comm comm, int* dims_arr) {
    int dim_num;
    int periods[m];
    int coords[m];
    MPI_Cart_get(comm, m, dims_arr, periods, coords);
}

int main(int argc, char *argv[])
{
    int num_procs;
    int this_rank;
    int n = stoi(argv[1]);
    int m = stoi(argv[2]);
    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // make dims cl arg
    int dims[m] = {};
    MPI_Dims_create(num_procs, m, dims);
    int periods[m];
    fill_n(periods, m, true);
    int reorder = true;
    MPI_Comm new_comm;
    MPI_Cart_create(MPI_COMM_WORLD, m, dims, periods, reorder, &new_comm);

    // get proc info
    MPI_Comm_rank(new_comm, &this_rank);
    int coords[2];
    MPI_Cart_coords(new_comm, this_rank, m, coords);

    // cout << "Rank " << this_rank << " has coords";
    // for (int i = 0; i < m; i++) {
    //     cout << " " << coords[i];
    // }              
    // cout << endl;

    // each process has dyn arr and partition index
    int* arr;
    int ind = 0;
    int size;

    if (this_rank == 0)
    {
        int dim_counts[m];
        get_dim_counts(m, new_comm, dim_counts);
        for (int i = 0; i < m; i++) {
            cout << "Dim " << i+1 << " has " << dim_counts[i] << " processes\n";
        }

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





    // delete[] arr;

    MPI_Finalize();
}