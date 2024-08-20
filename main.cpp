#include "Server.hpp"

int main(int ac, char **av) 
{
	try {
		Server::arg_control(ac, av);
	} catch (std::string &e) {
		std::cerr << "error: " << e << std::endl;
		return(-1);
	}

	Server	svObj;
	return( Server::runner(svObj, av[1], av[2]) );
}
