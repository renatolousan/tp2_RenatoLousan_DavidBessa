#include "../cabecalhos/btree.hpp"

// Insere um registro no arquivo de dados
block_addr insereRegistro(record* novaEntrada, int arqSaida, int arqOverflow){

	long localChave = (novaEntrada->id % HASHSIZE);
	long blocosBucket = 2;
	long blocoAncora = localChave*(sizeof(frame)*blocosBucket);
	long offset, offsetAux;

	block_addr valorRetorno;
	frame novoBloco;																
	memset((void*) novoBloco, 0 , sizeof(frame));

	block* blocoEntradas = (block*) novoBloco;									

	short blockFactor = 7;

	lseek(arqSaida, blocoAncora, SEEK_SET);									
	read(arqSaida, novoBloco, sizeof(frame));
										
	if(blocoEntradas->numEntradas > 0){
		if(blocoEntradas->numEntradas < blockFactor){					

			memcpy(&blocoEntradas->records[blocoEntradas->numEntradas], (void*) novaEntrada, sizeof(record));

			blocoEntradas->numEntradas++;
			offset = lseek(arqSaida, 0, SEEK_CUR); 
			write(arqSaida, novoBloco, sizeof(frame));

			valorRetorno.addr = offset;

			valorRetorno.typeaddr = ADDR_DATAF;

			return valorRetorno;

		}else{
			offsetAux = lseek(arqSaida, 0, SEEK_CUR);
			read(arqSaida, novoBloco, sizeof(frame));				
			if(blocoEntradas->numEntradas < blockFactor){

				memcpy(&blocoEntradas->records[blocoEntradas->numEntradas], (void*) novaEntrada, sizeof(record));

				blocoEntradas->numEntradas++;

				offset = lseek(arqSaida, offsetAux, SEEK_SET); 
				write(arqSaida, novoBloco, sizeof(frame));

				valorRetorno.addr = offset;

				valorRetorno.typeaddr = ADDR_DATAF;

				return valorRetorno;


			}else{
				
				frame bufferEscrita, bufferLeitura;
				record_overflow* novoRegistroOverflow = (record_overflow*) bufferEscrita;			

				memcpy(&novoRegistroOverflow->records, (void*) novaEntrada, sizeof(record));

				novoRegistroOverflow->next_record_addr = -1;

				if(blocoEntradas->overflow_off_t == -1){

					lseek(arqOverflow, 0, SEEK_END);
					offset = lseek(arqSaida, 0, SEEK_CUR); 
					write(arqOverflow, bufferEscrita,sizeof(record_overflow));	

					blocoEntradas->overflow_off_t = offset;

					lseek(arqSaida, offsetAux, SEEK_SET);
					write(arqSaida, novoBloco, sizeof(frame));

					valorRetorno.addr = offset;

					valorRetorno.typeaddr = ADDR_OVER;

					return valorRetorno;



				}else{

					record_overflow* registroOverflow = (record_overflow*) bufferLeitura;

					lseek(arqOverflow, blocoEntradas->overflow_off_t,	SEEK_SET);

					offset = lseek(arqSaida, 0, SEEK_CUR); 
					read(arqOverflow, bufferLeitura, sizeof(record_overflow));

					while( registroOverflow->next_record_addr != -1 ){

						lseek(arqOverflow, registroOverflow->next_record_addr, SEEK_SET);

						offset = lseek(arqSaida, 0, SEEK_CUR);  
						read(arqOverflow, bufferLeitura,	sizeof(record_overflow));

					}

					lseek(arqOverflow, 0, SEEK_END);
					offsetAux = lseek(arqSaida, 0, SEEK_CUR); 
					write(arqOverflow, bufferEscrita, sizeof(record_overflow));

					registroOverflow->next_record_addr = offsetAux;

					lseek(arqOverflow, offset, SEEK_SET);
					write(arqOverflow, bufferLeitura, sizeof(record_overflow));

					valorRetorno.addr = offsetAux;								

					valorRetorno.typeaddr = ADDR_OVER;		
				
					return valorRetorno;

				}

			}

		}

	}else{

		frame novoBlocoRegistros;
		memset((void*) novoBlocoRegistros, 0, sizeof(frame));
		blocoEntradas = (block*) novoBlocoRegistros;

		blocoEntradas->numEntradas++;									
		blocoEntradas->overflow_off_t = -1;
		memcpy(&blocoEntradas->records[0], (void*)novaEntrada, sizeof(record));

		frame proxBlocoRegistros;
		memset((void*) proxBlocoRegistros, 0, sizeof(block));
		blocoEntradas = (block*) proxBlocoRegistros;
		blocoEntradas->overflow_off_t = -1;

		offset =  lseek(arqSaida, blocoAncora, SEEK_SET);
		write(arqSaida, novoBlocoRegistros, sizeof(frame));
		write(arqSaida, proxBlocoRegistros, sizeof(frame));
		valorRetorno.addr = offset;
		valorRetorno.typeaddr = ADDR_DATAF;

		return valorRetorno;

	}

	valorRetorno.typeaddr = -1;

	return valorRetorno;

}

// Verifica se ha campos vazios nas entradas
inline bool campoVazio(string campo){
	return campo.empty() || campo == "NULL" || campo == "\"\"" || campo[0] != '\"';
}

// Remove aspas das strings
string removeAspas(string s){
	return s.substr(1, s.size()-2);
}

int achaEntradaArqDados(int idEntrada, int arqSaida, int arqOverflow){

	long localChave = (idEntrada % HASHSIZE);
	long blocosBucket = 2;
	long endBloco = localChave*(sizeof(frame)*blocosBucket);
	int countBlocosLidos = 0;
	long totalBlocos = 0;
	long aux = 0;

	frame buffer, bufferAux;
	memset((void*) buffer, 0, sizeof(frame));
	memset((void*) bufferAux, 0, sizeof(frame));

	int i = 0, j = 0;
	int blockFactor = 7;

	block* blocoEntradas = (block*) buffer;
	record_overflow* entradaOverflow = (record_overflow*) bufferAux;

	lseek(arqSaida, endBloco, SEEK_SET);
	read(arqSaida, buffer, sizeof(frame));
	countBlocosLidos++;

	while(j < blocosBucket){													
		while(i < blockFactor){													
			if(blocoEntradas->records[i].id == idEntrada){						

				memcpy((void*) &bufferAux, (void*) buffer, sizeof(record));
				cout << "Número de blocos lidos: " << countBlocosLidos << endl;
				aux = lseek(arqSaida, 0, SEEK_END);
				totalBlocos = aux/4096;
				aux = lseek(arqOverflow, 0, SEEK_END);
				totalBlocos = totalBlocos + (aux/4096);
				cout << "Número total de blocos: " << totalBlocos << endl;
				print_record(&blocoEntradas->records[i]);

				return 1;

			}

			i++;

		}
		read(arqSaida, buffer, sizeof(frame));
		countBlocosLidos++;								
		j++;
		i = 0;

	}

	if(blocoEntradas->overflow_off_t != -1){					
		lseek(arqOverflow, blocoEntradas->overflow_off_t, SEEK_SET);
		read(arqOverflow, buffer, sizeof(record_overflow));
		countBlocosLidos++;
		while((entradaOverflow->records.id != idEntrada) && entradaOverflow->next_record_addr != -1){
			lseek(arqOverflow, entradaOverflow->next_record_addr, SEEK_SET);
			read(arqOverflow, buffer, sizeof(record_overflow));
			countBlocosLidos++;

		}
		if(entradaOverflow->records.id == idEntrada){

			memcpy((void*) &bufferAux, (void*) buffer, sizeof(record));

			cout << "Número de blocos lidos: " << countBlocosLidos << endl;
			aux = lseek(arqSaida, 0, SEEK_END);
			totalBlocos = aux/4096;
			aux = lseek(arqOverflow, 0, SEEK_END);
			totalBlocos = totalBlocos + (aux/4096);
			cout << "Número total de blocos: " << totalBlocos << endl;
			print_record(&entradaOverflow->records);

			return 1;

		}

	}

	return 1;

}

int criaArqDados(){

	int arqDados = open("datafiles/datafile", O_CREAT | O_TRUNC | O_RDWR, 00700);

	if(arqDados < 0)
		return -1;

	return close(arqDados);

}

int abreArqDados(){
	return open("datafiles/datafile", O_RDWR);
}

int criaArqOverflow(){
	int fo = open("datafiles/overflow", O_CREAT | O_TRUNC, 00700);

	if(fo < 0)
		return -1;

	return close(fo);

}
int abreArqOverflow(){
	return open("datafiles/overflow", O_RDWR);
}

int uploadArquivo(string caminhoArquivo){

	string linha, campo;
	record novaEntrada;

	size_t tamString;

	regex semicolon(";");
	regex_token_iterator<string::iterator> fim;

	ifstream arqEntrada;
	arqEntrada.open(caminhoArquivo);

	if(!arqEntrada.is_open()){
		cout << "Arquivo nao foi encontrado." << endl;
		return -1;
	}

	if(criaArqDados() < 0){
		cout << "O arquivo de dados nao pode ser criado." << endl;
		return -1;
	}
	
	int arqDados = abreArqDados();
	if(arqDados < 0){
		cout << "Arquivo de dados nao foi aberto." << endl;
		return -1;
	}

	// Arquivo de overflow
	if(criaArqOverflow() < 0){
		cout << "O arquivo de dados nao pode ser criado." << endl;
		return -1;
	}
	
	int overflowfile = abreArqOverflow();
	if(overflowfile < 0){
		cout << "Arquivo de overflow nao foi criado" << endl;
		return -1;
	}

	while(getline(arqEntrada, linha)){
		linha.pop_back(); 
		regex_token_iterator<string::iterator> next(linha.begin(), linha.end(), semicolon, -1);
		campo = *next++;
		if(campoVazio(campo) || campo.back() != '\"'){
			cout << "ERROR: ID - " << campo << " invalido!" << endl;
			continue;

		}else{

			campo = removeAspas(campo);
			novaEntrada.id = stoi(campo, nullptr, 10);
		}

		cout << "Lendo ID: " << novaEntrada.id << "..." << endl;
		campo = *next++;
		if(campoVazio(campo)){
			cout << "Warning: Titulo - " << campo << " invalido!" << endl;
			novaEntrada.title[0] = 0;

		}else{
			while(campo.back() != '\"'){
 
                if(next == fim){
                    if(getline(arqEntrada, linha)){
                        regex_token_iterator<string::iterator> new_iterator(linha.begin(), linha.end(), semicolon, -1);
                        next = new_iterator;
                         
                    }else{
                        cout << "ERROR: ID "<< novaEntrada.id  << " : Titulo - O arquivo chegou ao fim sem concluir a insercao" << endl;
               			return -1;
                    }
 
                }else{
                    campo += *next++;   
                }
            }

			campo = removeAspas(campo);
			tamString = campo.copy(novaEntrada.title, 299, 0);
			novaEntrada.title[tamString] = 0;	
		}

		campo = *next++;
		if(campoVazio(campo) || campo.back() != '\"'){
			cout << "Warning: Ano - " << campo << " invalido!" << endl;
			novaEntrada.year = 0;

		}else{

			campo = removeAspas(campo);
			novaEntrada.year = stoi(campo, nullptr, 10);
		}
		campo = *next++;
		if(campoVazio(campo)){
			cout << "Warning: Autores - " << campo << " invalido!" << endl;
			novaEntrada.autors[0] = 0;

		}else{
			while(campo.back() != '\"'){
 
                if(next == fim){
                    if(getline(arqEntrada, linha)){
                        regex_token_iterator<string::iterator> new_iterator(linha.begin(), linha.end(), semicolon, -1);
                        next = new_iterator;
                         
                    }else{
                        cout <<"ERROR: ID "<< novaEntrada.id  << " : Autores - O arquivo chegou ao fim sem concluir a insercao" << endl;
               			return -1;
                    }
 
                }else{
                    campo += *next++;   
                }
            }

			
			campo = removeAspas(campo);
			tamString = campo.copy(novaEntrada.autors, 99, 0);
			novaEntrada.autors[tamString] = 0;	
		}
		campo = *next++;
		if(campoVazio(campo) || campo.back() != '\"'){
			cout << "Warning: Citacoes - " << campo << " invalido!" << endl;
			novaEntrada.mention = 0;

		}else{

			campo = removeAspas(campo);
			novaEntrada.mention = stoi(campo, nullptr, 10);
		}
		campo = *next++;
		if(campoVazio(campo) || campo.back() != '\"'){
			cout << "Warning: Timestamp - " << campo << " invalido!" << endl;
			novaEntrada.timestamp = str_to_date("0000-00-00 00:00:00");

		}else{

			campo = removeAspas(campo);
			novaEntrada.timestamp = str_to_date(campo);

		}
		campo = *next++;
		if(campoVazio(campo)){
			cout << "Warning: Snippet - " << campo << " invalido!" << endl;
			novaEntrada.snippet[0] = 0;

		}else{
			while(campo.back() != '\"'){
 
                if(next == fim){
                    if(getline(arqEntrada, linha)){
                        regex_token_iterator<string::iterator> new_iterator(linha.begin(), linha.end(), semicolon, -1);
                        next = new_iterator;
                         
                    }else{
                        cout << "ERROR: ID "<< novaEntrada.id  << " - Snippet O arquivo chegou ao fim sem concluir a insercao" << endl;
               			return -1;
                    }
 
                }else{
                    campo += *next++;   
                }
            }
             
			campo = removeAspas(campo);
			tamString = campo.copy(novaEntrada.snippet, 99, 0);
			novaEntrada.snippet[tamString] = 0;	
		}

		novaEntrada.tombstone = 0;
		block_addr addr_id = insereRegistro(&novaEntrada, arqDados, overflowfile);

		pair_id key;
		key.key = novaEntrada.id;
		key.ptr = addr_id;

	}

	close(arqDados);
	close(overflowfile);
	arqEntrada.close();

	return 0;
}
