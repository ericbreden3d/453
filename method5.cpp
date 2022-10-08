#include <mpi.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
using namespace std;

int main(int argc, char *argv[])
{
    int num_procs;
    int this_rank;
    int n = stoi(argv[1]);
    int max_subarr;
    srand(time(NULL));

    // make sure n is power of 2
    double test = log2(n);
    cout << test << " " << floor(test) << endl;
    cout << test - floor(test) << endl;
    if (test - floor(test) != 0) {
        cout << "Please pass power-of-2 input" << endl;
        return 0;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    int sub_size = n / num_procs;
    int remainder = n % num_procs;

    if (this_rank == 0)
    {
        // create and fill array with random values [-1,1]
        int arr[n] = {};
        int correct_result = 0;
        for (int i = 0; i < n; i++)
        {
            arr[i] = 1 - rand() % 3;
            correct_result += arr[i];
        }

        // split into 2d array of subarrays
        // vector<vector<int>> subarrs(num_children, vector<int>());
        // int cur_arr = 0;
        // for (int i = 0; i < n; i++) {
        //     subarrs[cur_arr].push_back(arr[i]);
        //     cur_arr++;
        //     if (cur_arr == num_children) {
        //         cur_arr = 0;
        //     }
        // }

        // MPI_Request req;
        // MPI_Isend(arr, n - root_sub_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);

        // int sum = 0;
        // for (int num : subarr)
        // {
        //     sum += num;
        // }

        // wait 1st ring to complete before starting next
        // MPI_Barrier(MPI_COMM_WORLD);

        // MPI_Isend(&sum, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);

        // int result;
        // MPI_Status status;
        // MPI_Recv(&result, 1, MPI_INT, num_procs - 1, 0, MPI_COMM_WORLD, &status);

        // cout << "Serial result: " << correct_result << " Distributed result: " << result << endl;
    }

    int arr_size = n - remainder - (this_rank * sub_size);
    vector<int> arr(arr_size, 0);
    MPI_Status status;

    // one to all
    int d = log2(num_procs);
    int mask = pow(2, d) - 1;
    for (int i = d - 1; i >= 0; i--) {
        int op = pow(2, i);
        mask = (mask ^ op);
        if (this_rank & mask == 0) {
            if (this_rank & op == 0) {
                int dest = (this_rank ^ op);
                // MPI_Isend(&sum, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);
            } else {
                int msg_source = (this_rank ^ op);
                // MPI_Recv(&arr[0], arr_size, MPI_INT, this_rank - 1, 0, MPI_COMM_WORLD, &status);
            }
        }
    }

   

    MPI_Finalize();
}