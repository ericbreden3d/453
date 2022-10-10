
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

    // get new rank, cart coords, and amount of procs in each dim
    MPI_Comm_rank(new_comm, &this_rank);
    int this_coord[2];
    MPI_Cart_coords(new_comm, this_rank, m, this_coord);
    int dim_counts[m];
    get_dim_counts(m, new_comm, dim_counts);

    for (int = 0; i < m; i++) {
        cout << "Dim " << i+1 << ": " << dim_counts[i] << " processors\n";
    }

    // calculate each dimensions assignment num
    if (this_rank == 0) {
        int dim_n[m] = {};
        for (int i = 0; i < m; i++) {
            int cur_n = i == 0 ? n : dim_n[i - 1];
            dim_n[i] = cur_n / dim_counts[i];
            cout << "Dim " << i+1 << " will send/receive " << dim_n[i] << " elems\n";
        }
    }

    return 0;

    // ** move to top
    // each process has dyn arr and partition index
    int* arr;
    int ind = 0;
    int size;
    MPI_Status status;
    MPI_Request req;

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

        // MPI_Isend(n / )

        cout << "Serial result: " << correct_result << endl;
    }

    // get dest coord by checking for first dim that hasn't been fully sent to
    int dest_coord[m];
    copy(this_coord, this_coord + m, dest_coord);
    for (int i = 0; i < m; i++) {
        if (this_coord[i] + 1 < dim_counts[i]) {
            dest_coord[i] = this_coord[i] + 1;
            break;   
        } 
     }





    // delete[] arr;

    MPI_Finalize();
}