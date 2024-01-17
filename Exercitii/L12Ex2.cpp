#include <iostream>
#include <ctime>
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);

	int rank, size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		MPI_File f;
		MPI_File_open(MPI_COMM_SELF, "arr.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &f);
		
		std::cout << "Matricea:\n";
		srand(time(NULL));

		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < size; ++j)
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

	MPI_File f;
	MPI_File_open(MPI_COMM_WORLD, "arr.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	MPI_File_seek(f, rank * size * sizeof(int), MPI_SEEK_SET);

	for (int r = 0; r < size; ++r)
	{
		if (r == rank)
		{
			std::cout << "Rankul " << r << ":\n";
			
			for (int j = 0; j < size; ++j)
			{
				int temp;
				MPI_Status status;

				MPI_File_read(f, &temp, 1, MPI_INT, &status);

				std::cout << temp << ' ';
			}

			std::cout << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_File_close(&f);
	MPI_Finalize();
}