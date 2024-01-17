#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>

typedef std::pair<int, char[MPI_MAX_PROCESSOR_NAME]> Structura;

int main()
{
	MPI_Init(NULL, NULL);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Datatype type;

	{
		int lens[] = { 1, MPI_MAX_PROCESSOR_NAME };
		MPI_Aint displs[] = { 0, sizeof(int) };
		MPI_Datatype types[] = { MPI_INT, MPI_CHAR };

		MPI_Type_create_struct(2, lens, displs, types, &type);
		MPI_Type_commit(&type);
	}

	Structura structura;
	structura.first = rank;

	{
		int len;
		MPI_Get_processor_name(structura.second, &len);
	}

	if (rank == 0)
	{
		std::vector<Structura> v(size);
		MPI_Gather(&structura, 1, type, &v[0], 1, type, 0, MPI_COMM_WORLD);

		for (int i = 0; i < size; ++i)
			std::cout << "Rankul " << v[i].first << " de pe nodul " << v[i].second << std::endl;
	}
	else
		MPI_Gather(&structura, 1, type, NULL, 0, type, 0, MPI_COMM_WORLD);
	
	MPI_Type_free(&type);
	MPI_Finalize();
}