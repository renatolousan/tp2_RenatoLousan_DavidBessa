#include "../cabecalhos/util.hpp"

int main(int argc, char const *argv[]){
	if(argc != 2){
		return -1;
	}	

	if(uploadArquivo(argv[1])){
		return -1;
	}

	

	return 0;
}
