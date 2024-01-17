#include <iostream>
#include <climits>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	MPI_Comm comm;

	if (argc <= 1)
	{
		int l;

		std::cout << "Nr. procese copii: ";
		std::cin >> l;

		char str[] = "", * args[] = { str, NULL };

		MPI_Comm icomm;
		MPI_Comm_spawn(argv[0], args, l, MPI_INFO_NULL, 0, MPI_COMM_SELF, &icomm, MPI_ERRCODES_IGNORE);
		MPI_Intercomm_merge(icomm, 0, &comm);
		MPI_Comm_free(&icomm);

		int norm = INT_MIN;

		MPI_Reduce(MPI_IN_PLACE, &norm, 1, MPI_INT, MPI_MAX, 0, comm);
		std::cout << "Valoarea maxima: " << norm << std::endl;
		MPI_Comm_free(&comm);
	}
	else
	{
		MPI_Comm icomm;
		MPI_Comm_get_parent(&icomm);
		MPI_Intercomm_merge(icomm, 1, &comm);
		MPI_Comm_free(&icomm);

		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		srand(time(NULL) / (rank + 1));
		int norm = rand() & 65535;

		std::cout << "Rankul " << rank << " are " << norm << std::endl;

		MPI_Reduce(&norm, NULL, 1, MPI_INT, MPI_MAX, 0, comm);
		MPI_Comm_free(&comm);
	}

	MPI_Finalize();
}