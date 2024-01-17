#include <iostream>
#include <ctime>
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);

	int rank, size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int n, l;

	if (rank == 0)
	{
		int m;
		std::cout << "m, n si l:\n"; // nr. de linii, nr. de coloane, nr. de linii a submatricei
		std::cin >> m >> n >> l;

		if (size * l > m)
		{
			std::cout << "nr. de rankuri * l > m" << std::endl;
			MPI_Abort(MPI_COMM_WORLD, -1);
		}

		MPI_File f;
		MPI_File_open(MPI_COMM_SELF, "arr.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &f);
		
		std::cout << "Matricea:\n";
		srand(time(NULL));

		for (int i = 0; i < m; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				int temp = rand() & 65535;
				std::cout << temp << ' ';

				MPI_File_write(f, &temp, 1, MPI_INT, MPI_STATUS_IGNORE);
			}

			std::cout << '\n';
		}

		std::cout << std::endl;
		MPI_File_close(&f);
	}

	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&l, 1, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_File f;
	MPI_File_open(MPI_COMM_WORLD, "arr.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	MPI_File_seek(f, rank * l * n * sizeof(int), MPI_SEEK_SET);

	for (int r = 0; r < size; ++r)
	{
		if (r == rank)
		{
			std::cout << "Rankul " << r << ":\n";
			
			for (int i = 0; i < l; ++i)
			{
				for (int j = 0; j < n; ++j)
				{
					int temp;
					MPI_Status status;

					MPI_File_read(f, &temp, 1, MPI_INT, &status);

					std::cout << temp << ' ';
				}

				std::cout << '\n';
			}

			std::cout << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_File_close(&f);
	MPI_Finalize();
}