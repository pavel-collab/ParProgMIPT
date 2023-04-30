#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cassert>
#include "mpi.h"

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: mpiexec -n 2 %s N\n", argv[0]);
        return 1;
    }
    // количество запусков
    long long N = atoll(argv[1]);

    int size = 0;
    int rank = 0;
    // значение, которое будем пересылать
    double x = 1.2234;
    double recv = 0;

    MPI_Status status;

    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        perror("[-] Error starting MPI program. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // нам нужно только 2 процесса
    if (size != 2) {
        perror("[-] Too many proceses. Programm was terminated.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    if (rank == 1) {
        double start = MPI_Wtime();
        /*
        MPI может оптимизировать работу Send. Если компилятор видит, что происходит несколько
        небольших отправок, он может сделать своеобразные delay и отправить все данные 
        одной посылкой. Оптимизация чем-то напоминает оптимизацию отправки в протоколе TCP.

        Чтобы избужать применения данной оптимизации, будем совершать 
        прием и отправку в обоих процессах. Однако, при этом стоит учесть, что полученное время
        включает в себя время работы Send на стороне master, время работы Recv на стороне
        slave, время работы Send на стороне slave и время работы Recv на стороне master.
        Для упрощения будем считать, что все эти операции занимают примерно одно и то же время, 
        а значит, полученный результат необходимо поделить на 4, чтобы получить время работы 
        одной операции Send.
        */
        for (long long i = 0; i < N; ++i) {
            MPI_Send(&x, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
            MPI_Recv(&recv, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            assert(recv == x);
        }
        double end = MPI_Wtime();

        double total_time = (end - start) * 1000; // ms
        std::cout << "total time is " << total_time << " ms" << std::endl;
        std::cout << "time of single session is " << total_time / N << " ms" << std::endl;
        std::cout << "time of single send is " << total_time / (N*4) << " ms" << std::endl;

        const char* time_file_name = "time.txt";
        FILE* time_out_file = fopen(time_file_name, "a");
        if (time_out_file) 
            fprintf(time_out_file, "%lf ", total_time / (N*4));
        fclose(time_out_file);
    }

    if (rank == 0) {
        for (long long i = 0; i < N; ++i) {
            MPI_Recv(&recv, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            assert(recv == x);
            MPI_Send(&recv, 1, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();

    return 0;
}