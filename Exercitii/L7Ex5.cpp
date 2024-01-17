#include <iostream>
#include <cstdlib>
#include <set>
#include <vector>
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);
	srand(time(NULL));

	int ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &ranks);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	std::vector<int> members[2];

	for (int i = 0; i < ranks; ++i)
		members[i & 1].push_back(i);

	MPI_Group groupWorld, group[2];
	MPI_Comm_group(MPI_COMM_WORLD, &groupWorld);	
 	
 	MPI_Group_incl(groupWorld, members[0].size(), members[0].data(), group);
 	MPI_Group_incl(groupWorld, members[1].size(), members[1].data(), group + 1);
 
 	int groupRank[2];
 	MPI_Group_rank(group[0], groupRank);
 	MPI_Group_rank(group[1], groupRank + 1);

 	MPI_Comm comm[2];
 	MPI_Comm_create(MPI_COMM_WORLD, group[0], comm);
 	MPI_Comm_create(MPI_COMM_WORLD, group[1], comm + 1);

 	if (groupRank[0] != MPI_UNDEFINED)
 	{
 		if (groupRank[0] == 0)
 			std::cout << "Grupul 1:" << std::endl;

 		MPI_Comm ring;
 		int dims = members[0].size(), period = 1, reord = 1, prev, next;

        MPI_Cart_create(comm[0], 1, &dims, &period, reord, &ring);
        MPI_Cart_shift(ring, 0, 1, &prev, &next);

        MPI_Sendrecv(NULL, 0, MPI_INT, next, 0, NULL, 0, MPI_INT, prev, 0, ring, MPI_STATUS_IGNORE);

        std::cout << "Rankul " << groupRank[0] << " a primit mesaj de la rankul " << prev << std::endl;
        MPI_Comm_free(comm);
 	}

 	MPI_Barrier(MPI_COMM_WORLD);

 	if (groupRank[1] != MPI_UNDEFINED)
 	{
 		if (groupRank[1] == 0)
 		{
 			std::cout << "Grupul 2:" << std::endl;

 			for (int i = 1; i < members[1].size(); ++i)
 			{
 				std::cout << "Rankul 0 expediaza rankului " << i << std::endl;
 				MPI_Sendrecv(NULL, 0, MPI_INT, i, 0, NULL, 0, MPI_INT, i, 0, comm[1], MPI_STATUS_IGNORE);
 			}
 		}
 		else
 		{
 			MPI_Recv(NULL, 0, MPI_INT, 0, 0, comm[1], MPI_STATUS_IGNORE);
 			std::cout << "Rankul " << groupRank[1] << " expediaza rankului 0" << std::endl;
 			MPI_Send(NULL, 0, MPI_INT, 0, 0, comm[1]);
 		}

 		MPI_Comm_free(comm + 1);
 	}

 	MPI_Group_free(group);
 	MPI_Group_free(group + 1);
 	MPI_Group_free(&groupWorld);
	MPI_Finalize();
}