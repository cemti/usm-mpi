#include <iostream>
#include <cstdlib>
#include <MPI.h>

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    if (argc != 3)
    {
        MPI_Finalize();
        return 0;
    }

    const int incep = atoi(argv[1]);
    const bool callSendRecv = argv[2][0] == '1'; // 0 (Send + Recv) sau 1 (Sendrecv)

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (incep >= size)
    {
        MPI_Finalize();
        return 0;
    }

    int target = rank + 1, source = rank - 1;

    if (target >= size)
        target = 0;

    if (source < 0)
        source = size - 1;

    if (rank == incep)
    {
        double startTime = MPI_Wtime();

        if (callSendRecv)
            MPI_Sendrecv(NULL, 0, MPI_INT, target, 0, NULL, 0, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        else
        {
            MPI_Send(NULL, 0, MPI_INT, target, 0, MPI_COMM_WORLD);
            MPI_Recv(NULL, 0, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        std::cout << "Timpul de executie: " << MPI_Wtime() - startTime << std::endl;
    }
    else
    {
        MPI_Recv(NULL, 0, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(NULL, 0, MPI_INT, target, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}