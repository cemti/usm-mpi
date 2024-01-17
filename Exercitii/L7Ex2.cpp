#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);

	int ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &ranks);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	std::vector<int> row(ranks * 2);


	if (rank == 0)
	{
		std::vector<int> mtx(ranks * ranks);

		for (int i = 0; i < mtx.size(); ++i)
			mtx.push_back(rand() & 0xffffff);

		std::vector<int> counts(ranks), displs(ranks);

		for (int i = 0; i < ranks; ++i)
		{
			
		}

		MPI_Scatter(mtx.data(), ranks, MPI_INT, row.data(), ranks, MPI_INT, MPI_COMM_WORLD);
	}



	std::vector<int> members;

	for (int i = 0; i < ranks; i += 3)
		members.push_back(i);

	MPI_Group groupWorld, group;
	MPI_Comm_group(MPI_COMM_WORLD, &groupWorld);	
 	MPI_Group_incl(groupWorld, members.size(), members.data(), &group);

 	int grRank;
	MPI_Group_rank(group, &grRank);

	if (grRank != MPI_UNDEFINED)
	{
		char name[MPI_MAX_PROCESSOR_NAME];
		int temp;
		MPI_Get_processor_name(name, &temp);
		std::cout << "Rankul " << grRank << '(' << rank << ") de pe nodul " << name << std::endl;
	}

	MPI_Group_free(&group);
	MPI_Group_free(&groupWorld);
	MPI_Finalize();
}