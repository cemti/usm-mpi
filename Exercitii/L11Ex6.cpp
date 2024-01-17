#include <iostream>
#include <vector>
#include <ctime>
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int m, n, l;

	if (rank == 0)
	{
		std::cout << "m, n, l:\n"; // nr. de linii, nr. de coloane, nr. de linii a submatricei
		std::cin >> m >> n >> l;

		if (size * l != m)
		{
			std::cout << "nr. de rankuri * l != m" << std::endl;
			MPI_Abort(MPI_COMM_WORLD, -1);
		}
	}

	MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&l, 1, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Datatype linie;
	MPI_Type_contiguous(l * n, MPI_INT, &linie);
	MPI_Type_commit(&linie);

	std::vector<int> v(l * n);

	if (rank == 0)
	{
		std::vector<int> mtx(m * n);
		std::cout << "Matricea:\n";

		srand(time(NULL));

		for (int i = 0; i < m; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				mtx[i * n + j] = rand() & 65535;
				std::cout << mtx[i * n + j] << ' ';
			}

			std::cout << '\n';
		}

		std::cout << std::endl;
		MPI_Scatter(&mtx[0], 1, linie, &v[0], l * n, MPI_INT, 0, MPI_COMM_WORLD);
	}
	else
		MPI_Scatter(NULL, 0, linie, &v[0], l * n, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < size; ++i)
	{
		if (i == rank)
		{
			std::cout << "Rankul " << i << ":\n";

			for (int j = 0; j < l; ++j)
			{
				for (int k = 0; k < n; ++k)
					std::cout << v[j * n + k] << ' ';

				std::cout << '\n';
			}

			std::cout << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Type_free(&linie);
	MPI_Finalize();
}