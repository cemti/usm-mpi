#include "allmaxloc.h"
#include "loadbalancing.h"
#include "masterslave.h"

int main(int argc, char* argv[])
{
	bool timed = false, statistical = false, silent = false;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (argc < 2)
	{
		if (rank == 0)
			std::cout << "Se necesita cel putin 2 argumenzi." << std::endl;
		
		MPI_Finalize();
		return 0;
	}

	for (int i = 2; i < argc; ++i)
	{
		if (std::string("print") == argv[i])
			verbose = true;
		else if (std::string("serial") == argv[i])
			serial = true;
		else if (std::string("time") == argv[i])
			timed = true;
		else if (std::string("independent") == argv[i])
			independent = true;
		else if (std::string("stat") == argv[i])
			statistical = true;
		else if (std::string("silent") == argv[i])
			silent = true;
		else if (std::string("lb") == argv[i])
			loadBalancing = true;
	}

	MPI_Comm_size(MPI_COMM_WORLD, &nRanks);

	double timestamp[2] = { MPI_Wtime() };
	PrepareMatrices(argv[1]);
	timestamp[0] = MPI_Wtime() - timestamp[0];

	MPI_Type_contiguous(rows, MPI_FLOAT_INT, floatIntArray);
	MPI_Type_contiguous(cols, MPI_FLOAT_INT, floatIntArray + 1);
	MPI_Type_commit(floatIntArray);
	MPI_Type_commit(floatIntArray + 1);

	MPI_Op_create((MPI_User_function*)FloatIntArrayAllMaxLoc, true, &ALLMAXLOC);

	{
		auto ptr = serial ? ProcessMatrixSerial : ProcessMatrix;

		if (rank == 0 && verbose)
			std::cout << "Pentru matricea A:" << std::endl;

		timestamp[1] = MPI_Wtime();
		ptr(false);

		if (rank == 0 && verbose)
			std::cout << "Pentru matricea B transpusa:" << std::endl;

		ptr(true);
		timestamp[1] = MPI_Wtime() - timestamp[1];
	}

	if (rank == 0 && !silent)
	{
		std::cout << "Situatiile Nash de echilibru:\n";

		if (ne.empty())
			std::cout << "Nu exista nici una.";
		else
		{
			for (auto& pair : ne)
				std::cout << "(" << pair.first << ", " << pair.second << "), ";

			std::cout << "\nTotal: " << ne.size();
		}

		std::cout << std::endl;
	}

	if (rank == 0 && (timed || statistical))
		std::cout << "Timpul de executie pentru paralelizare:\nrank,nivel de date,nivel de operatii,total" << std::endl;

	if (timed)
	{
		BEGIN_SYNC
		std::cout << rank << ',' << timestamp[0] << ',' << timestamp[1] << ',' << timestamp[0] + timestamp[1] << std::endl;
		END_SYNC
	}

	if (statistical)
	{
		if (rank == 0)
		{
			double avg[2], mx[2], mn[2];

			MPI_Reduce(timestamp, avg, 2, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			MPI_Reduce(timestamp, mx, 2, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
			MPI_Reduce(timestamp, mn, 2, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

			for (auto& x : avg)
				x /= nRanks;

			std::cout << "Media," << avg[0] << ',' << avg[1] << ',' << avg[0] + avg[1] << '\n'
				<< "Max," << mx[0] << ',' << mx[1] << ',' << mx[0] + mx[1] << '\n'
				<< "Min," << mn[0] << ',' << mn[1] << ',' << mn[0] + mn[1] << std::endl;
		}
		else
		{
			MPI_Reduce(timestamp, nullptr, 2, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			MPI_Reduce(timestamp, nullptr, 2, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
			MPI_Reduce(timestamp, nullptr, 2, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
		}
	}

	MPI_Op_free(&ALLMAXLOC);
	MPI_Type_free(floatIntArray);
	MPI_Type_free(floatIntArray + 1);
	MPI_Finalize();
}