/*
Цели создания новых коммуникаторов более эффективноя работа с пересылками.
Нампример нам нужно, чтобы общались между собой только процессы с четными ранками.
Или нам нужно переупорядочить процессы (потому что коммуникатор это набор упорядоченных ранков).
*/
#include <stdio.h>
#include <math.h>
#include "mpi.h"

// #define COMPARE
#define DEBUG

double Sum(int start, int end) {
    double res = 0;
    for (double i = start; i <= end; ++i) {
        res += 1/i;
    }
    return res;
}

int main(int argc, char* argv[]) {

    // TODO: try to reduce the amount of variables
    int size = 0;
    int rank = 0;
    int new_size = 0;
    int new_rank = 0;
    int compare_res;
    double res = 0;
    uint64_t n_range = 0;
    uint64_t mod_size = 0;
    double rank_res = 0;

    if (argc != 2) {
        fprintf(stderr, "[-] Usage %s N\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // New comm variable
    MPI_Comm new_comm;
    // To create new comm we should to create firstly a new group with target processes
    // The group can't make a communication between processes
    MPI_Group base_group;
    MPI_Group new_group;

    // Get the base (MPI_COMM_WORLD group)
    MPI_Comm_group(MPI_COMM_WORLD, &base_group);

    // Here we can exclude from new group any rank that we want
    // int exclude_rank = size-1; // rank that we want to exclude from new group
    int exclude_rank = 0;
    MPI_Group_excl(base_group, 1, &exclude_rank, &new_group);
    if (MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm) != MPI_SUCCESS) {
        perror("[-] Error comm creating. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    // This code is called in EVERY process. 
    // So, we need to check if this runk in new_comm
    // If we won't do it, there will be an error MPI_ERR_COMM
    if (new_comm != MPI_COMM_NULL) {
        #ifdef COMPARE
        // Lets compare MPI_COMM_WORLD and new_comm
        MPI_Comm_compare(MPI_COMM_WORLD, new_comm, &compare_res);
        switch (compare_res)
        {
        case MPI_UNEQUAL:
            printf("MPI_COMM_WORLD and new_comm are absolutely unequal\n");
            break;

        case MPI_IDENT:
            printf("MPI_COMM_WORLD and new_comm is a single comm\n");
            break;
        
        case MPI_CONGRUENT:
            printf("MPI_COMM_WORLD and new_comm have the same group of processes and also the same numeration\n");
            break;
        
        case MPI_SIMILAR:
            printf("MPI_COMM_WORLD and new_comm have the same group of processes but different numeration\n");
            break;

        default:
            break;
        }
        #endif // COMPARE

        MPI_Comm_size(new_comm, &new_size);
        MPI_Comm_rank(new_comm, &new_rank);

        printf("WORLD RANK/SIZE : %d/%d \t NEW_COMM RANK/SIZE %d/%d\n", rank, size, new_rank, new_size);

        //-----------------------------Computation sum of 1/n by new group of processes-----------------------------

        if (int(floor(N/2)) < new_size) {
            perror("[-] Every process have to sum at least two numbers.\nSo, total number of processes must be at least 2 times less then N.\n");
            MPI_Abort(MPI_COMM_WORLD, rc);
        }

        n_range = N / new_size;
        mod_size = N % new_size;

        int rank_start, rank_end = 0;

        if (new_rank < mod_size) {
            rank_start = new_rank*(n_range + 1) + 1;
            rank_end = rank_start + n_range;
        }
        else {
            rank_start = new_rank*n_range + 1 + mod_size;
            rank_end = rank_start + n_range - 1;
        }

        rank_res = Sum(rank_start, rank_end);
        MPI_Reduce(&rank_res, &res, 1, MPI_DOUBLE, MPI_SUM, 0, new_comm);

        //----------------------------------------------------------------------------------------------------------

        MPI_Comm_free(&new_comm);
    }
    else {
        printf("WORLD RANK/SIZE : %d/%d is out of new_comm\n", rank, size);
    }

    // Note, that here we print the MPI_COMM_WORLD rank and size
    // This mean, one of this processes (with rank 0) will have rank_res=0.0 
    #ifdef DEBUG
        printf("Process %d, size %d, rank sum %lf\n", rank, size, rank_res);
    #endif // DEBUG

    MPI_Finalize();

    printf("The result is %lf\n", res);

    return 0;
}