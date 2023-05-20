#include "../cabecalhos/util.hpp"


int main(int argc, char const *argv[])
{
	int arqDados = open("datafiles/datafile", O_RDWR);
	int arqOverflow = open("datafiles/overflow", O_RDWR);
	achaEntradaArqDados(stoi(argv[1]), arqDados, arqOverflow);
	return 0;
}