/* =====
Transmiterea mesajelor pe cerc incepand cu procesul "incep ".
Valoarea variabilei "incep" este initializata de toate procesele.
=====*/
#include <stdio.h>
#include <iostream>
#include <mpi.h>

void Sendrecv(const void* source, int sendCount, MPI_Datatype sendType, int targetRank, int, void* dest, int recvCount, MPI_Datatype sourceType, int sourceRank, int, MPI_Comm comm, MPI_Status*)
{
    MPI_Win win;
    int size;
    MPI_Type_size(sourceType, &size);
    MPI_Win_create(dest, sendCount * size, 1, MPI_INFO_NULL, comm, &win);

    MPI_Win_fence(0, win);
    MPI_Put(source, sendCount, sendType, targetRank, 0, sendCount, sendType, win);
    MPI_Win_fence(0, win);

    /*MPI_Win_fence(0, win);
    MPI_Get(dest, recvCount, sourceType, sourceRank, 0, recvCount, sourceType, win);
    MPI_Win_fence(0, win);*/

    MPI_Win_free(&win);
}

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

    Sendrecv(&rank, 1, MPI_INT, (rank + 1) % size, 0, &t, 1, MPI_INT, (rank + size - 1) % size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Procesu cu rancul %d al nodului '%s' a primit valoarea %d de la procesul cu rancul %d\n", rank, processor_name, t, t);
    std::cout << std::flush;

    MPI_Finalize();
    return 0;
}