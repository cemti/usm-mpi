#include <iostream>
#include <vector>
#include <cassert>
#include <mpi.h>

int Scatter(const void* source, int sendCount, MPI_Datatype sendType, void* destination, int recvCount, MPI_Datatype recvType, int root, MPI_Comm comm)
{
    int rank, statusIdx = MPI_Comm_rank(comm, &rank);

    if (statusIdx != MPI_SUCCESS)
        return statusIdx;

    if (rank == root)
    {
        int size;
        statusIdx = MPI_Comm_size(comm, &size);

        if (statusIdx != MPI_SUCCESS)
            return statusIdx;

        for (int i = 0; i < size; ++i)
        {
            int typeSize;
            statusIdx = MPI_Type_size(sendType, &typeSize);

            if (statusIdx != MPI_SUCCESS)
                break;

            if (i == root)
            {
                assert(sendCount >= recvCount);
                memcpy(destination, static_cast<const char*>(source) + i * typeSize, typeSize);
            }
            else
            {
                statusIdx = MPI_Send(static_cast<const char*>(source) + i * typeSize, sendCount, sendType, i, 0, comm);

                if (statusIdx != MPI_SUCCESS)
                    break;
            }
        }
    }
    else
        statusIdx = MPI_Recv(destination, recvCount, recvType, root, 0, comm, MPI_STATUS_IGNORE);

    return statusIdx;
}

int main()
{
    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double data;

    if (rank == 0)
    {
        std::vector<double> vec(size);

        for (int i = 0; i < vec.size(); ++i)
            vec[i] = sqrt(i);

        Scatter(vec.data(), 1, MPI_DOUBLE, &data, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    else
        Scatter(NULL, 0, MPI_DOUBLE, &data, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank > 0)
        MPI_Recv(NULL, 0, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    else
        std::cout << "Radacinele patrate pentru fiecare rank:\n";

    std::cout << rank << " -> " << data << std::endl;

    if (rank + 1 < size)
        MPI_Send(NULL, 0, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);

    MPI_Finalize();
}