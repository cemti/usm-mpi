#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm parent;
	MPI_Comm_get_parent(&parent);

	if (parent == MPI_COMM_NULL)
	{
		std::cout << "Mesajul afisat de un proces fara parinte." << std::endl;
		MPI_Comm_spawn(argv[0], argv, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &parent, MPI_ERRCODES_IGNORE);
	}
	else
		std::cout << "Mesajul afisat de un proces copil." << std::endl;

	MPI_Comm_free(&parent);
	MPI_Finalize();
}