#include <iostream>
#include <vector>
#include <mpi.h>

int Scatter(const void* source, int sendCount, MPI_Datatype sendType, void* destination, int recvCount, MPI_Datatype recvType, int root, MPI_Comm comm)
{
    int rank;
    auto statusIdx = MPI_Comm_rank(comm, &rank);

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
                return statusIdx;

            if (i == root)
                memcpy(destination, static_cast<const char*>(source) + i * typeSize, typeSize);
            else
            {
                statusIdx = MPI_Send(static_cast<const char*>(source) + i * typeSize, sendCount, sendType, i, 0, comm);

                if (statusIdx != MPI_SUCCESS)
                    return statusIdx;
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
    int rank, data;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        std::vector<int> vec(size);

        for (int i = 0; i < vec.size(); ++i)
            vec[i] = -i;

        Scatter(vec.data(), 1, MPI_INT, &data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
        Scatter(NULL, 0, MPI_INT, &data, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        std::vector<int> vec(size);
        MPI_Gather(&data, 1, MPI_INT, vec.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
        MPI_Gather(&data, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);

    std::cout << rank << " -> " << data << std::flush;

    MPI_Finalize();
}