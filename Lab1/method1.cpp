#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
using namespace std;

int main(int argc, char* argv[])
{
    int num_procs;
    int this_rank;
    int n = stoi(argv[1]);
    int max_subarr;
    int num_children;
    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &this_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    num_children = num_procs - 1;
    max_subarr = ceil((float) n / num_children);

    if (this_rank == 0) {
        // create and fill array with random values [-1,1]
        int arr[n] = {};
        int correct_result = 0;
        for (int i = 0; i < n; i++)
        {
            arr[i] = 1 - rand() % 3;
            correct_result += arr[i];
        }

        // split into 2d array of subarrays
        vector<vector<int>> subarrs(num_children, vector<int>());
        int cur_arr = 0;
        for (int i = 0; i < n; i++) {
            subarrs[cur_arr].push_back(arr[i]);
            cur_arr++;
            if (cur_arr == num_children) {
                cur_arr = 0;
            }
        }

        MPI_Request req;
        for (int dest = 1; dest < num_procs; dest++) {
            MPI_Isend(&subarrs[dest-1][0], subarrs[dest-1].size(), MPI_INT, dest, 0, MPI_COMM_WORLD, &req);
        }
        
        int sum = 0;
        MPI_Status status;
        for (int dest = 1; dest < num_procs; dest++) {
            int num;
            MPI_Recv(&num, 1, MPI_INT, dest, 0, MPI_COMM_WORLD, &status);
            sum += num;
        }
        cout << "Distributed result: " << sum << " Serial result: " << correct_result << endl;
    } else {
        vector<int> subarr(max_subarr, 0);
        MPI_Status status;
        MPI_Recv(&subarr[0], max_subarr, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        // calc sum
        int sum = 0;
        for (int num : subarr) {
            sum += num;
        }

        MPI_Request req;
        MPI_Isend(&sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &req);
    }

    MPI_Finalize();
}