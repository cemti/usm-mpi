#include <iostream>
#include <mpi.h>

int Allreduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
	int ret = MPI_Reduce(sendbuf, recvbuf, count, datatype, op, 0, comm);

	if (ret != MPI_SUCCESS)
		return ret;

	return MPI_Bcast(recvbuf, 1, datatype, 0, comm);
}

int main()
{
	MPI_Init(NULL, NULL);

	int rank, sum, ranks;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &ranks);

	if (rank == 0)
		std::cout << "Suma tuturor rakurilor (afisata de fiecare proces):" << std::endl;

	Allreduce(&rank, &sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	for (int i = 0; i < ranks; ++i)
	{
		if (rank == i)
			std::cout << "Rankul " << rank << " are valoarea " << sum << std::endl;

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Finalize();
}
