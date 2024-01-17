#include <iostream>
#include <mpi.h>

int main()
{
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        for (int i = 0; i < size; ++i)
        {
            MPI_Sendrecv(NULL, 0, MPI_INT, i, 0, NULL, 0, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "0 a primit raspuns de la " << i << std::endl;
        }
    }
    else
    {
        MPI_Recv(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << rank << " a primit raspuns de la 0" << std::endl;
        MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
