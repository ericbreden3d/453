#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
using namespace std;

int main(int argc, char* argv[])
{
    int num_procs;
    int this_rank;
    int n = stoi(argv[1]);
    int max_subarr;
    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    int sub_size = n / num_procs;
    int remainder = n % num_procs;

    if (this_rank == 0) {
        // create and fill array with random values [-1,1]
        vector<int> arr(n, 0);
        int correct_result = 0;
        for (int i = 0; i < n; i++)
        {
            arr[i] = 1 - rand() % 3;
            correct_result += arr[i];
        }

        // root will handle remainder if n not divisible by num procs
        int root_sub_size = sub_size + remainder;
        vector<int> subarr(root_sub_size, 0);
        for (int i = 0; i < root_sub_size; i++) {
            subarr[i] = arr.back();
            arr.pop_back();
        }

        MPI_Request req;
        MPI_Isend(&arr[0], n - root_sub_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);

        int sum = 0;
        for (int num : subarr) {
            sum += num;
        }

        // wait 1st ring to complete before starting next
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Isend(&sum, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);

        int result;
        MPI_Status status;
        MPI_Recv(&result, 1, MPI_INT, num_procs-1, 0, MPI_COMM_WORLD, &status);

        cout << "Serial result: " << correct_result << " Distributed result: " << result << endl;
     } else {
        int arr_size = n - remainder - (this_rank * sub_size);
        vector<int> arr(arr_size, 0);
        MPI_Status status;
        MPI_Recv(&arr[0], arr_size, MPI_INT, this_rank-1, 0, MPI_COMM_WORLD, &status);

        vector<int> subarr(sub_size, 0);
        for (int i = 0; i < sub_size; i++) {
            subarr[i] = arr.back();
            arr.pop_back();
        }

        MPI_Request req;
        if (this_rank != num_procs-1) {
            MPI_Isend(&arr[0], arr_size - sub_size, MPI_INT, this_rank + 1, 0, MPI_COMM_WORLD, &req);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        int sum = 0;
        for (int num : subarr) {
            sum += num;
        }

        int cur_sum;
        MPI_Recv(&cur_sum, 1, MPI_INT, this_rank - 1, 0, MPI_COMM_WORLD, &status);

        cur_sum += sum;
        int dest = (this_rank == num_procs-1) ? 0 : this_rank + 1;
        MPI_Isend(&cur_sum, 1, MPI_INT, dest, 0, MPI_COMM_WORLD, &req);
    }

    MPI_Finalize();
}