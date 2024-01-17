#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int m, n;

	if (rank == 0)
	{
		std::cout << "m, n:\n";
		std::cin >> m >> n;
	}

	MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	std::vector<int> mtx(m * n);

	if (rank == 0)
	{
		std::vector<int> mtr(m * n);
		srand(time(NULL));

		std::cout << "Matricea:\n";

		for (int i = 0; i < m; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				mtr[i * n + j] = rand() & 65535;
				std::cout << mtr[i * n + j] << ' ';
			}

			std::cout << '\n';
		}

		std::cout << std::endl;

		MPI_Datatype vType, rType, type;

		MPI_Type_vector(m, 1, n, MPI_INT, &vType);
		MPI_Type_create_resized(vType, 0, sizeof(int), &rType);
		MPI_Type_contiguous(n, rType, &type);
		MPI_Type_commit(&type);

		MPI_Scatter(&mtr[0], 1, type, &mtx[0], m * n, MPI_INT, 0, MPI_COMM_SELF);

		MPI_Type_free(&vType);
		MPI_Type_free(&rType);
		MPI_Type_free(&type);
	}

	MPI_Bcast(&mtx[0], m * n, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < size; ++i)
	{
		if (i == rank)
		{
			std::cout << "Rankul " << i << ":\n";

			for (int j = 0; j < n; ++j)
			{
				for (int k = 0; k < m; ++k)
					std::cout << mtx[j * m + k] << ' ';

				std::cout << '\n';
			}

			std::cout << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Finalize();
}