#include "../cabecalhos/btree.hpp"

int totalNos = 0;

// Abre o arquivo de indice usando BTree
int abreArqIndice(){
	return open("indexes/primary_tree", O_RDWR);
}

// Cria o indice da BTree em arquivo
int criaIndiceBTree(){
	int arq = open("indexes/primary_tree", O_WRONLY | O_CREAT | O_TRUNC, 00700);
	if(arq < 0){
		std::cout << "ERROR: Nao foi possivel abrir o arquivo " << "indexes/primary_tree" << std::endl;
		return -1;
	}

	frame block;
	btree* headerBTree = (btree*) block;

	headerBTree->height = 1;
	headerBTree->ptr_root.addr = 4096;

	// Escrevendo o header da BTree para o arquivo
	if(write(arq, block, PAGE_SIZE) < 0){
		std::cout << "ERROR: Nao foi possivel criar o arquivo de indice" << std::endl;
		return -1;
	}

	primary_tree_node* raiz = (primary_tree_node*) block;

	raiz->nr_regstr = 0;
	raiz->last_ptr.addr = 0;
	raiz->last_ptr.typeaddr = ADDR_LEAF;

	// Escreve o no raiz no arquivo
	if(write(arq, block, PAGE_SIZE) < 0){
		std::cout << "ERROR: Nao foi possivel criar o arquivo de indice" << std::endl;
		return -1;
	}

	// Fechando o arquivo
	if(close(arq) < 0){
		std::cout << "ERROR: Nao foi possivel fechar " << "indexes/primary_tree" << std::endl;
		return -1;
	}
	
	return 0;
}

// Acha a posicao de um elemento no indice da BTree
int achaPos(int id, pair_id* indexes, int nr_regstr){
	int i = 0, j = nr_regstr - 1, mid = 0;

	// Busca binaria para achar a posicao
	while(i <= j){
		mid = (i + j) / 2;
		if(id < indexes[mid].key){
			j = mid - 1;
		}else if(id > indexes[mid].key){
			i = mid + 1;
		}else{
			return mid + 1;
		}
	}

	return (id > indexes[mid].key) ? mid + 1 : mid;
}

// Ordena o indice da BTree
void ordena(pair_id* indexes, int nr_regstr) {
	int i;
	pair_id pivot;
	i = nr_regstr - 2;
	pivot = indexes[nr_regstr - 1];

	// Bubble sort para ordenar os indices
	while(i >= 0 && pivot.key < indexes[i].key){
		indexes[i + 1] = indexes[i];
		i--;
	}

	indexes[i + 1] = pivot;
}

// Splita um nó folha quando está cheio
pair_id splitNoFolha(pair_id novaEntrada, int fd_btree, long endNo, primary_tree_node* noBTree){
	pair_id novaReferencia;
	int m = BTREE_ID_ORDER * 2;
	pair_id conteudo[m+1];

	// Copia os registros do nó no array
	memcpy(conteudo, noBTree->regstr, m*sizeof(pair_id));

	// Adiciona um novo registro no final do array
	conteudo[m] = novaEntrada;
	ordena(conteudo, m+1);

	frame novoBlock;
	primary_tree_node* novoNo = (primary_tree_node*) novoBlock;

	// Divide entradas igualmente com os dois nos resultantes
	novoNo->nr_regstr = BTREE_ID_ORDER + 1;
	novoNo->last_ptr = noBTree->last_ptr;
	noBTree->nr_regstr = BTREE_ID_ORDER;
	memcpy(noBTree->regstr, conteudo, BTREE_ID_ORDER*sizeof(pair_id));
	memcpy(novoNo->regstr, &conteudo[BTREE_ID_ORDER], (BTREE_ID_ORDER + 1)*sizeof(pair_id));

	long endNovo = lseek(fd_btree, 0, SEEK_END);
	if(endNovo < 0 || write(fd_btree, novoBlock, PAGE_SIZE) < 0){
		std::cout << "ERROR: Nao pode criar folha"  << std::endl;
		novaReferencia.ptr.typeaddr = -1;
		return novaReferencia;
	}

	// Update node_tree's last pointer to point to new_node
	noBTree->last_ptr.addr = endNovo;
	noBTree->last_ptr.typeaddr = ADDR_LEAF;
	novaReferencia.ptr = noBTree->last_ptr;
	novaReferencia.key = conteudo[BTREE_ID_ORDER].key;

	if(lseek(fd_btree, endNo, SEEK_SET) < 0 || write(fd_btree, noBTree, PAGE_SIZE) != PAGE_SIZE){
		std::cout << "ERROR: Could not update leaf" << std::endl;
		novaReferencia.ptr.typeaddr = -1;
	}
	
	return novaReferencia;
}

// Splita os nós no caso da inserção na BTree
pair_id splitNo(pair_id novaEntrada, int fd_btree, long endNo, primary_tree_node* noBTree, pair_id novaRef){
	int m = BTREE_ID_ORDER * 2;

	// Cria um array para poder ordenar todo o conteúdo
	pair_id conteudo[m+1];
	memcpy(conteudo, noBTree->regstr, m*sizeof(pair_id));
	conteudo[m] = novaRef;

	// Ordena o conteudo
	ordena(conteudo, m+1);

	// Prepara um novo nó
	frame novoBloco;
	primary_tree_node* novoNo = (primary_tree_node*) novoBloco;
	novoNo->nr_regstr = BTREE_ID_ORDER;
	novoNo->last_ptr = noBTree->last_ptr;

	noBTree->nr_regstr = BTREE_ID_ORDER;

	// Copia as entraas originais para o novo nó e original
	memcpy(noBTree->regstr, conteudo, BTREE_ID_ORDER*sizeof(pair_id));
	memcpy(novoNo->regstr, &conteudo[BTREE_ID_ORDER+1], BTREE_ID_ORDER*sizeof(pair_id));

	// Cria um novo nó no final do arquivo
	long novoEnd = lseek(fd_btree, 0, SEEK_END);
	if(novoEnd < 0 || write(fd_btree, novoBloco, PAGE_SIZE) < 0){
		std::cout << "ERROR: Falha ao escrever no offset " << endNo << std::endl;
		novaRef.ptr.typeaddr = -1;
		return novaRef;
	}

	// Update the original node
	if(lseek(fd_btree, endNo, SEEK_SET) < 0 || write(fd_btree, noBTree, PAGE_SIZE) < 0){
		std::cout << "ERROR: Falha ao escrever no offset " << endNo << std::endl;
		novaRef.ptr.typeaddr = -1;
		return novaRef;
	}

	novaRef.ptr.addr = novoEnd;
	novaRef.key = conteudo[BTREE_ID_ORDER].key;

	return novaRef;
}

// Insere uma entrda em um nó específico
pair_id inserirNo(pair_id novaEntrada, int fd_btree, long endNo, int m){
	frame block;
	pair_id novaRef;
	novaRef.ptr.typeaddr = -2;

	if(lseek(fd_btree, endNo, SEEK_SET) < 0 || read(fd_btree, block, PAGE_SIZE) < 0){
		std::cout << "ERROR: Failed to seek or read at address " << endNo << " ." << std::endl;
		novaRef.ptr.typeaddr = -1;
		return novaRef;
	}

	primary_tree_node* noBTree = (primary_tree_node*) block;

	if(m == 0){
		if(noBTree->nr_regstr > BTREE_ID_ORDER * 2){
			return splitNoFolha(novaEntrada, fd_btree, endNo, noBTree);
		}else{
			noBTree->regstr[noBTree->nr_regstr] = novaEntrada;
			noBTree->nr_regstr++;
			ordena(noBTree->regstr, noBTree->nr_regstr);

			if(lseek(fd_btree, endNo, SEEK_SET) < 0 || write(fd_btree, block, PAGE_SIZE) < 0 ){
				std::cout << "ERROR: Failed to update the leaf" << std::endl;
				novaRef.ptr.typeaddr = -1;
				return novaRef;
			}

			return novaRef;
		}
	}
	
	int position = achaPos(novaEntrada.key, noBTree->regstr, noBTree->nr_regstr);
	novaRef = inserirNo(novaEntrada, fd_btree, noBTree->regstr[position].ptr.addr, m - 1);

	if(novaRef.ptr.typeaddr < 0){
		return novaRef;
	}

	int tmp = noBTree->regstr[position].key;
	noBTree->regstr[position].key = novaRef.key;
	novaRef.key = tmp;

	if(noBTree->nr_regstr > BTREE_ID_ORDER * 2){
		return splitNo(novaEntrada, fd_btree, endNo, noBTree, novaRef);
	}else{
		memcpy(&noBTree->regstr[position+1], &noBTree->regstr[position], sizeof(pair_id)*(position - noBTree->nr_regstr) + sizeof(block_addr));
		noBTree->regstr[position] = novaRef;
		noBTree->nr_regstr++;

		if(lseek(fd_btree, endNo, SEEK_SET) < 0 || write(fd_btree, noBTree, PAGE_SIZE) < 0){
			std::cout << "ERROR: Failed to seek and write at offset " << endNo << std::endl;
			novaRef.ptr.typeaddr = -1;
			return novaRef;
		}

		novaRef.ptr.typeaddr = -2;
	}

	return novaRef;
}

// Insere uma nova chave na BTree
int insereChaveBTree(pair_id chave, int fd_btree){
	frame headerBloco;
	btree* BTree = (btree*) headerBloco;

	if(lseek(fd_btree, 0, SEEK_SET) < 0 || read(fd_btree, headerBloco, PAGE_SIZE) < 0){
		std::cout << "ERROR: Não foi possível carregar o header da arvore." << std::endl;
		return -1;
	}

	pair_id noRetorno = inserirNo(chave, fd_btree, BTree->ptr_root.addr, BTree->height - 1);

	if(noRetorno.ptr.typeaddr == -1)
		return -1;

	if(noRetorno.ptr.typeaddr == -2)
		return 0;

	frame blocoNovaRaiz;
	primary_tree_node* novaRaiz = (primary_tree_node*) blocoNovaRaiz;

	long novoEnd = lseek(fd_btree, 0, SEEK_END);

	novaRaiz->nr_regstr = 1;
	novaRaiz->last_ptr.addr = 1;
	novaRaiz->regstr[0].ptr = BTree->ptr_root;
	novaRaiz->regstr[0].key = noRetorno.key;
	novaRaiz->regstr[1] = noRetorno;

	if( novoEnd < 0 || write(fd_btree, blocoNovaRaiz, PAGE_SIZE) < 0){
		std::cout << "ERROR: Nao foi possivel escrever a nova raiz na arvore." << std::endl;
		return -1;
	}
	
	BTree->height++;
	BTree->ptr_root.addr = novoEnd;

	if(lseek(fd_btree, 0, SEEK_SET) < 0 || write(fd_btree, headerBloco, PAGE_SIZE) < 0){
		std::cout << "ERROR: Nao foi possivel escrever o header." << std::endl;
		return -1;
	}

	return 0;
}

