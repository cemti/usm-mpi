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

	std::set<int> members;

	{
		int nMembers = 1 + rand() % ranks;

		while (members.size() < nMembers)
			members.insert(rand() % ranks);
	}

	MPI_Group groupWorld, group;
	MPI_Comm_group(MPI_COMM_WORLD, &groupWorld);	
 	MPI_Group_incl(groupWorld, members.size(), std::vector<int>(members.begin(), members.end()).data(), &group);
 
 	int groupRank;
 	MPI_Group_rank(group, &groupRank);

 	MPI_Comm comm;
 	MPI_Comm_create(MPI_COMM_WORLD, group, &comm);

 	if (groupRank != MPI_UNDEFINED)
 	{
 		MPI_Comm ring;
 		int dims = members.size(), period = 1, reord = 1, prev, next;

        MPI_Cart_create(comm, 1, &dims, &period, reord, &ring);
        MPI_Cart_shift(ring, 0, 1, &prev, &next);

        int msg;
        MPI_Sendrecv(&groupRank, 1, MPI_INT, next, 0, &msg, 1, MPI_INT, prev, 0, ring, MPI_STATUS_IGNORE);

        std::cout << "Rankul " << groupRank << " (original " << rank << ") a primit mesaj de la rankul " << msg << std::endl;
        MPI_Comm_free(&comm);
 	}

 	MPI_Group_free(&group);
 	MPI_Group_free(&groupWorld);
	MPI_Finalize();
}