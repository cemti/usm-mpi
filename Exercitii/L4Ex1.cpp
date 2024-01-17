#include <iostream>
#include <vector>
#include <cassert>
#include <mpi.h>

int Gather(const void* source, int sendCount, MPI_Datatype sendType, void* destination, int recvCount, MPI_Datatype recvType, int root, MPI_Comm comm)
{
    int rank, statusIdx = MPI_Comm_rank(comm, &rank);

    if (statusIdx != MPI_SUCCESS)
        return statusIdx;

    if (rank == root)
    {
        int size;
        statusIdx = MPI_Comm_size(comm, &size);

        if (statusIdx == MPI_SUCCESS)
            for (int i = 0; i < size; ++i)
            {
                int typeSize;
                statusIdx = MPI_Type_size(recvType, &typeSize);

                if (statusIdx != MPI_SUCCESS)
                    break;

                if (i == root)
                {
                    assert(recvCount <= sendCount);
                    memcpy(static_cast<char*>(destination) + i * typeSize, source, typeSize);
                }
                else
                {
                    statusIdx = MPI_Recv(static_cast<char*>(destination) + i * typeSize, recvCount, recvType, i, 0, comm, MPI_STATUS_IGNORE);

                    if (statusIdx != MPI_SUCCESS)
                        break;
                }
            }
    }
    else
        statusIdx = MPI_Send(source, sendCount, sendType, root, 0, comm);

    return statusIdx;
}

int main()
{
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	rank = -rank;

    if (rank == 0)
    {
        std::vector<int> vec(size);
        Gather(&rank, 1, MPI_INT, vec.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

        std::cout << "Lista rancurilor negate:\n";

        for (int i = 0; i < size; ++i)
            std::cout << vec[i] << ' ';

        std::cout << std::endl;
    }
    else
        Gather(&rank, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Finalize();
}