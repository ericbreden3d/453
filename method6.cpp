
#include <mpi.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
using namespace std;

// object for summing data and collecting at source process (reduce/gather)
struct Reduce_Task {
    int* coord;
    char oper;

    Reduce_Task(int* coord, char oper) : coord(coord), oper(oper) {}
};

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
    int* arr;  // each process has dyn arr
    MPI_Status status;
    MPI_Request req;
    vector<Reduce_Task> reversal_stack;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // create cartesian topology with m dimensions
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

    // calc amount of elements that will be transfered at each dimension
    int dim_n[m] = {};
    for (int i = 0; i < m; i++) {
        int cur_n = i == 0 ? n : dim_n[i - 1];
        dim_n[i] = cur_n / dim_counts[i];
    }

    // root process creates and fills array with random values [-1,1]
    if (this_rank == 0)
    {
        srand(time(NULL));
        arr = new int[n];
        int correct_result = 0;
        for (int i = 0; i < n; i++)
        {
            arr[i] = 1 - rand() % 3;
            correct_result += arr[i];
        }

        cout << "Serial result: " << correct_result << endl;
    }

    // reverse loop through coord nums to find src using first non-zero num
    // e.g. [1, 3, 0], 3 > 0, so receiving from [1, 2, 0] in 2nd iter
    // also fill dim_send[] with 1 or 0 for whether proc sends to that dim 
    int send_dim[m] = {};
    for (int i = m - 1; i >= 0; i--) {
        if (this_coord[i] > 0) {
            // create array to hold size calculated previously for this dim
            int amount = dim_n[i] * (dim_counts[i] - this_coord[i]);
            arr = new int[amount];
            // create copy of coord with this dim - 1
            int src_coord[m];
            copy(this_coord, this_coord + m, src_coord);
            src_coord[i]--;
            // get rank of coord
            int src_rank;
            MPI_Cart_rank(new_comm, src_coord, &src_rank);
            // recv from src_rank
            // cout << "(" <<this_coord[0] << ", " << this_coord[1] << ") receiving " << amount << " to (" << src_coord[0] << ", " << src_coord[1] << ")" << endl;
            MPI_Recv(arr, amount, MPI_INT, src_rank, 0, new_comm, &status);
            // also note if the message needs to travel further
            send_dim[i] = (this_coord[i] < dim_counts[i]-1) ? 1 : 0;
            // add opposite operation to stack for reduction later
            reversal_stack.push_back(Reduce_Task(src_coord, "s"));
            break;    
        } else {
            // if trailing 0, then this proc sends to next dim during iter i
            send_dim[i] = 1;
        }
    }

    // send using send_dim bool data gathered above
    for (int i = 0; i < m; i++) {
        if (send_dim[i]) {
            // increment the coord of this dim to get dest
            int dest_coord[m];
            copy(this_coord, this_coord + m, dest_coord);
            dest_coord[i]++;
            // get rank from coors
            int dest_rank;
            MPI_Cart_rank(new_comm, dest_coord, &dest_rank);
            // calculate amount to send such that keeping cur_len/num_procs in dim
            int amount = dim_n[i] * (dim_counts[i] - dest_coord[i]);
            // cout << "(" <<this_coord[0] << ", " << this_coord[1] << ") sending " << amount << " to (" << dest_coord[0] << ", " << dest_coord[1] << ")" << endl;
            // send to src_rank
            MPI_Isend(arr + dim_n[i], amount, MPI_INT, dest_rank, 0, new_comm, &req);
        }
    }
    
    
    for (int i = 0; i < dim_n[m-1]; i++) {
        cout << arr[i] << " ";
    }




    delete[] arr;

    MPI_Finalize();
}