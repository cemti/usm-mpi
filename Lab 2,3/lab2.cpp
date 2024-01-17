#include <iostream>
#include <vector>
#include <cmath>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (argc < 2)
	{
		if (rank == 0)
			std::cout << "Se necesita de indicat si fateta." << std::endl;

		MPI_Finalize();
		return 0;
	}

	bool swp = false;

	if (argc > 2)
		swp = argv[2][0] == 'i';

	int side = atoi(argv[1]) % 6;

	int nRanks;
	MPI_Comm_size(MPI_COMM_WORLD, &nRanks);

	int cubeRoot = int(cbrt(nRanks));

	MPI_Comm truncComm;

	{
		std::vector<int> members(cubeRoot * cubeRoot * cubeRoot);

		for (int i = 0; i < members.size(); ++i)
			members[i] = i;

		MPI_Group group1, group2;

		MPI_Comm_group(MPI_COMM_WORLD, &group1);
		MPI_Group_incl(group1, int(members.size()), members.data(), &group2);
		MPI_Comm_create(MPI_COMM_WORLD, group2, &truncComm);
		
		MPI_Group_free(&group1);
		MPI_Group_free(&group2);
	}

	if (truncComm == MPI_COMM_NULL)
	{
		MPI_Finalize();
		return 0;
	}

	MPI_Comm cubeComm;

	{
		int dims[] = { cubeRoot, cubeRoot, cubeRoot };
		int periods[3] = {};
		MPI_Cart_create(truncComm, 3, dims, periods, false, &cubeComm);
	}

	int sideAxisIdx = side >> 1;

	int pos[3];
	MPI_Cart_coords(cubeComm, rank, 3, pos);

	if ((side & 1 ? 0 : cubeRoot - 1) != pos[sideAxisIdx] ||
		!(
			(sideAxisIdx != 0 && (pos[0] == 0 || pos[0] == cubeRoot - 1)) ||
			(sideAxisIdx != 1 && (pos[1] == 0 || pos[1] == cubeRoot - 1)) ||
			(sideAxisIdx != 2 && (pos[2] == 0 || pos[2] == cubeRoot - 1))
			))
	{
		MPI_Comm_free(&cubeComm);
		MPI_Comm_free(&truncComm);
		MPI_Finalize();
		return 0;
	}

	int source, dest;

	switch (side)
	{
	case 0: case 1:
		if (pos[2] == 0)
		{
			MPI_Cart_shift(cubeComm, 1, 1, &source, &dest);

			int temp;

			if (source == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 2, 1, &temp, &source);
			else if (dest == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 2, 1, &temp, &dest);
		}
		else if (pos[2] == cubeRoot - 1)
		{
			MPI_Cart_shift(cubeComm, 1, 1, &dest, &source);

			int temp;

			if (source == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 2, 1, &source, &temp);
			else if (dest == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 2, 1, &dest, &temp);
		}
		else
		{
			MPI_Cart_shift(cubeComm, 2, 1, &source, &dest);

			if (pos[1] == 0)
				std::swap(source, dest);
		}

		if (side == 1)
			std::swap(source, dest);

		break;

	case 2: case 3:
		if (pos[2] == 0)
		{
			MPI_Cart_shift(cubeComm, 0, 1, &source, &dest);

			int temp;

			if (source == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 2, 1, &temp, &source);
			else if (dest == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 2, 1, &temp, &dest);
		}
		else if (pos[2] == cubeRoot - 1)
		{
			MPI_Cart_shift(cubeComm, 0, 1, &dest, &source);

			int temp;

			if (source == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 2, 1, &source, &temp);
			else if (dest == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 2, 1, &dest, &temp);
		}
		else
		{
			MPI_Cart_shift(cubeComm, 2, 1, &source, &dest);

			if (pos[0] == 0)
				std::swap(source, dest);
		}

		if (side == 2)
			std::swap(source, dest);

		break;

	case 4: case 5:
		if (pos[1] == 0)
		{
			MPI_Cart_shift(cubeComm, 0, 1, &source, &dest);

			int temp;

			if (source == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 1, 1, &temp, &source);
			else if (dest == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 1, 1, &temp, &dest);
		}
		else if (pos[1] == cubeRoot - 1)
		{
			MPI_Cart_shift(cubeComm, 0, 1, &dest, &source);

			int temp;

			if (source == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 1, 1, &source, &temp);
			else if (dest == MPI_PROC_NULL)
				MPI_Cart_shift(cubeComm, 1, 1, &dest, &temp);
		}
		else
		{
			MPI_Cart_shift(cubeComm, 1, 1, &source, &dest);

			if (pos[0] == 0)
				std::swap(source, dest);
		}

		if (side == 5)
			std::swap(source, dest);

		break;
	}

	if (swp)
		std::swap(source, dest);

	if (source == MPI_PROC_NULL || dest == MPI_PROC_NULL)
	{
		MPI_Comm_free(&cubeComm);
		MPI_Comm_free(&truncComm);
		MPI_Finalize();
		return 0;
	}

	int tPos[3];

	MPI_Cart_coords(cubeComm, dest, 3, tPos);

	std::cout << "Rankul " << rank << " (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ") expediaza rankului "
		<< dest << " (" << tPos[0] << ", " << tPos[1] << ", " << tPos[2] << ")\n";

	MPI_Sendrecv(NULL, 0, MPI_INT, dest, 0, NULL, 0, MPI_INT, source, 0, cubeComm, MPI_STATUS_IGNORE);

	MPI_Cart_coords(cubeComm, source, 3, tPos);

	std::cout << "Rankul " << rank << " (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ") a receptionat de la rankul "
		<< source << " (" << tPos[0] << ", " << tPos[1] << ", " << tPos[2] << ')' << std::endl;

	MPI_Comm_free(&cubeComm);
	MPI_Comm_free(&truncComm);
	MPI_Finalize();
}