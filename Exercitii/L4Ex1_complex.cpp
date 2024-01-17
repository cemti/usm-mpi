#include <iostream>
#include <vector>
#include <mpi.h>

int Gather(const void* source, int sendCount, MPI_Datatype sendType, void* destination, int recvCount, MPI_Datatype recvType, int root, MPI_Comm comm)
{
    int rank;
    auto statusIdx = MPI_Comm_rank(comm, &rank);

    if (statusIdx == MPI_SUCCESS)
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
            statusIdx = MPI_Type_size(recvType, &typeSize);

            if (statusIdx != MPI_SUCCESS)
                return statusIdx;

            if (i == root)
                memcpy(static_cast<char*>(destination) + i * typeSize, source, typeSize);
            else
            {
                statusIdx = MPI_Recv(static_cast<char*>(destination) + i * typeSize, recvCount, recvType, i, 0, comm, MPI_STATUS_IGNORE);

                if (statusIdx != MPI_SUCCESS)
                    return statusIdx;
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
    int rank, size, data;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0)
    {
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        std::vector<int> vec(size);

        for (int i = 0; i < vec.size(); ++i)
		{
            vec[i] = -i;
			std::cout << -i << ' ';
		}
		
		std::cout << std::endl;
        MPI_Scatter(vec.data(), 1, MPI_INT, &data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
        MPI_Scatter(NULL, 0, MPI_INT, &data, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::vector<int> vec(size);
        Gather(&data, 1, MPI_INT, vec.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
        Gather(&data, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
	
	if (rank > 0)
		MPI_Recv(NULL, 0, MPI_INT, rank - 1, 0, MPI_STATUS_IGNORE);

    std::cout << rank << " -> " << data << std::flush;
	
	if (rank + 1 < size)
		MPI_Send(NULL, 0, MPI_INT, rank + 1, 0);

    MPI_Finalize();
}