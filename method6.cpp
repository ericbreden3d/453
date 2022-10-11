
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

    // calculate number of elems to send/receive in each dimension
    // debug
    // for (int i = 0; i < m; i++) {
    //     cout << "Dim " << i+1 << ": " << dim_counts[i] << " processors\n";
    // }

    int dim_n[m] = {};
    for (int i = 0; i < m; i++) {
        int cur_n = i == 0 ? n : dim_n[i - 1];
        dim_n[i] = cur_n / dim_counts[i];
        // debug
        // cout << "Dim " << i+1 << " will send/receive " << dim_n[i] << " elems\n";
    }

    return 0;

    // ** move to top
    // each process has dyn arr and cur arr length
    int* arr;
    int ind = 0;
    int cur_len;
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

        cout << "Serial result: " << correct_result << endl;
    }

    // reverse loop through coords nums to find src.
    // e.g. [1, 3, 0], 3 > 0, so receiving from [1, 2, 0] in 2nd iter
    // also use dim_send to determine when this proc will send
    int send_dim[m] = {};
    for (int i = m - 1; i >= 0; i--) {
        if (this_coord[i] > 0) {
            // create array to hold size calculated previously for this dim
            int amount = dim_n[i];
            arr = new int[amount];
            // create copy of coord with this dim - 1 and recv from this src
            int src_coord[m];
            copy(this_coord, this_coord + m, src_coord);
            src_coord[i]--;
            int src_rank;
            MPI_Cart_rank(new_comm, src_coord, &src_rank);
            MPI_Recv(arr, amount, MPI_INT, src_rank, 0, new_comm, &status);
            cur_len = amount;
            break;    
        } else {
            // if trailing 0, then this proc sends to next dim during iter i
            send_dim[i] = 1;
        }
    }

    // calc sum

    for (int i = 0; i < m; i++) {
        if (send_dim[i] == 1) {
            int amount = dim_n[i];
            int dest_coord[m];
            copy(this_coord, this_coord + m, dest_coord);
            dest_coord[i]++;
            int dest_rank;
            MPI_Cart_rank(new_comm, dest_coord, &dest_rank);
            MPI_Isend(arr + amount, cur_len - amount, MPI_INT, dest_rank, 0, new_comm, &req);
            cur_len = amount;
        }
    }

    cout << "Rank: " << this_rank << " completed \n";

    delete[] arr;

    MPI_Finalize();
}