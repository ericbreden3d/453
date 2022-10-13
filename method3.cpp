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
    max_subarr = ceil((float) n / num_procs);
    int subarr_sz = n / num_procs;

    if (this_rank == 0) {
        // create and fill array with random values [-1,1]
        int arr[n];
        int correct_result = 0;
        for (int i = 0; i < n; i++)
        {
            arr[i] = 1 - rand() % 3;
            correct_result += arr[i];
        }

        vector<int> subarr(n / num_procs, 0);
        MPI_Scatter(arr, subarr_sz, MPI_INT, &subarr[0], subarr_sz, MPI_INT, 0, MPI_COMM_WORLD);

        int sum = 0;
        for (int num : subarr) {
            sum += num;
        }

        int result;
        MPI_Reduce(&sum, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        cout << "Serial result: " << correct_result << " Distributed result: " << result << endl;
     } else {
        vector<int> subarr(subarr_sz, 0);
        MPI_Scatter(nullptr, 0, MPI_INT, &subarr[0], subarr_sz, MPI_INT, 0, MPI_COMM_WORLD);

        int sum = 0;
        for (int num : subarr) {
            sum += num;
        }

        MPI_Reduce(&sum, nullptr, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}