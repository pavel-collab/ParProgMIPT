#include <iostream>
#include <cassert>
#include "mpi.h"

// #define DEBUG

//! следим за тем, чтобы функция копирования не выходила за границу массива 
void CopyArray(int* src, int* dst, unsigned a, unsigned b) {
    unsigned dst_itr = 0;
    for (unsigned i = a; i <= b; ++i) {
        dst[dst_itr] = src[i];
        dst_itr++;
    }
}

void Bond(int* arr, unsigned n1, unsigned m, unsigned n2) {
    unsigned N1 = m - n1 + 1;
    unsigned N2 = n2 - m;

    int* sub_arr1 = (int*) malloc(N1*sizeof(int));
    int* sub_arr2 = (int*) malloc(N2*sizeof(int));
    CopyArray(arr, sub_arr1, n1, m);
    CopyArray(arr, sub_arr2, m+1, n2);

    unsigned itr1 = 0;
    unsigned itr2 = 0;
    unsigned main_itr = n1;

    while ((itr1 < N1) && (itr2 < N2)) {
        if (sub_arr1[itr1] < sub_arr2[itr2]) {
            arr[main_itr] = sub_arr1[itr1];
            itr1++;
        }
        else {
            arr[main_itr] = sub_arr2[itr2];
            itr2++;
        }
        main_itr++;
    }

    if (itr1 == N1) {
        while(itr2 < N2) {
            arr[main_itr] = sub_arr2[itr2];
            itr2++;
            main_itr++;
        }
    }
    else {
        while(itr1 < N1) {
            arr[main_itr] = sub_arr1[itr1];
            itr1++;
            main_itr++;
        }
    }
    free(sub_arr1);
    free(sub_arr2);
}

// Функция сортирует массив arr от элемента с индексом n1 до элемента с индексом n2
void Merge(int* arr, unsigned n1, unsigned n2) {
    
    unsigned m = (n1 + n2) / 2;
    if (n1 == n2)
        return;
    else {
        Merge(arr, n1, m);
        Merge(arr, m+1, n2);
        Bond(arr, n1, m, n2);
    }
}

int main(int argc, char* argv[]) {

    int size = 0;
    int rank = 0;

    int arr[] = {3, 13, 8, 1, 15, 2, 3, 7};
    int N = 8;

    if (argc != 2) {
        fprintf(stderr, "expected more arguments\n");
        return 1;
    }
    int N_proc = atoi(argv[1]);
    int* raz = (int*) calloc(N_proc, sizeof(int));
    int* dist = (int*) calloc(N_proc, sizeof(int));
    int dist_val = 0;
    for (int proc = 0; proc < N_proc; ++proc) {
        int k = 0;
        if (rank < N%N_proc)
            k = N/N_proc + 1;
        else
            k = N/N_proc;

        dist[proc] = dist_val;
        dist_val += k;
        raz[proc] = k;
    }

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    assert(N_proc == size);
    int k = 0;
    if (rank < N%size)
        k = N/size + 1;
    else
        k = N/size;

    MPI_Bcast (&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int* x_ptr = nullptr;
    int* x = (int*) calloc(k, sizeof(int));
    x_ptr = x;
    MPI_Scatterv(arr, raz, dist, MPI_INT, x_ptr, k, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    Merge(x_ptr, 0, k-1);

    int s = size, m = 1;
    while (s > 1) {
        s = s/2 + s%2;
        // правые процессы отравляют длину массива и сам массив
        if((rank-m)%(2*m) == 0) {            
            MPI_Send(&k, 1, MPI_INT, rank-m, 0, MPI_COMM_WORLD);
            MPI_Send(x_ptr, k, MPI_INT, rank-m, 0, MPI_COMM_WORLD);
        }

        // левые процессы принимают массивы
        if((rank%(2*m) == 0) && (size - rank > m)) {
            MPI_Status status;
            int k1 = 0;

            MPI_Recv(&k1, 1, MPI_INT, rank+m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int* y = (int*) calloc(k+k1, sizeof(int));
            MPI_Recv(y, k1, MPI_INT, rank+m, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            // соединяем 2 массива
            for (int i = 0; i < k; i++)
                y[i+k1] = x_ptr[i];

            // делаем перераспределение внутри массива
            Bond(y, 0, k1-1, k+k1-1);

            int* x_new = (int*) calloc(k+k1, sizeof(int));
            x_ptr = x_new;

            for (int i = 0; i < k+k1; i++)
                x_ptr[i] = y[i];
            free(y);

            k = k + k1;
            
            #ifdef DEBUG
            std::cout << "rank " << rank << std::endl;
            for (int i = 0; i < k; ++i){
                std::cout << x_ptr[i] << std::endl;
            }
            std::cout << "s = " << s << std::endl;
            #endif //DEBUG
        }
        m = 2*m;
    }

    if (rank == 0) {
        for (int i = 0; i < N; ++i) {
            std::cout << x_ptr[i] << std::endl;
        }
    }

    MPI_Finalize();

    return 0;
}

/*
0        1      2       3
[3, 13] [1, 8] [2, 15] [3, 7]


0              2
[1, 3, 8, 13] [2, 3, 7, 15]
*/