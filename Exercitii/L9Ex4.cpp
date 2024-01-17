/* =====
Transmiterea mesajelor pe cerc incepand cu procesul "incep ".
Valoarea variabilei "incep" este initializata de toate procesele.
=====*/
#include <stdio.h>
#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[])
{
    int size, rank, t, namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);

    if (rank == 0)
    {
        printf("\n=====REZULTATUL PROGRAMULUI '%s' \n", argv[0]);
        std::cout << std::flush;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Win win;
    MPI_Win_create(&t, sizeof t, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    MPI_Win_fence(0, win);
    MPI_Put(&rank, 1, MPI_INT, (rank + 1) % size, 0, 1, MPI_INT, win);
    MPI_Win_fence(0, win);

    printf("Procesu cu rancul %d al nodului '%s' a primit valoarea %d de la procesul cu rancul %d\n", rank, processor_name, t, t);
    std::cout << std::flush;

    MPI_Win_free(&win);
    MPI_Finalize();
    return 0;
}