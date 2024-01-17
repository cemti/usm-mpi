#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm parent;
	MPI_Comm_get_parent(&parent);

	if (parent == MPI_COMM_NULL)
		argc = 1;
	
	if (argc <= 2)
	{
		char* args[] = { argv[0], NULL, NULL };

		if (parent != MPI_COMM_NULL)
		{
			args[1] = argv[0];
			std::cout << "Mesaj afisat de un proces fiu, care va genera 2 procese." << std::endl;
			MPI_Comm_disconnect(&parent);
		}

		MPI_Comm_spawn(argv[0], args, 2, MPI_INFO_NULL, 0, MPI_COMM_SELF, &parent, MPI_ERRCODES_IGNORE);
	}
	else
		std::cout << "Da, s-a demonstrat ca un proces fiu poate sa genereze procese MPI." << std::endl;

	MPI_Comm_disconnect(&parent);
	MPI_Finalize();
}