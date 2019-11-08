/*
 * Programa  crm_sales.cpp
 *
 * Remaq Com�rcio e Representa��es LTDA.
 *
 * Autor: Mario Sergio 
 * <mario@remaqbh.com.br>
 *
 * Agosto 2018.
 *
 * Compila��o:
 * g++ -DCARREGAMENTO_DINAMICO -Wall -g -fno-strict-aliasing -c "crm_sales.cpp" -o "crm_sales.o"
 * g++ -o crm_sales crm_sales.o -lpthread -ldl
 *
 */
#ifdef __unix__                    /* __unix__ is usually defined by compilers targeting Unix systems */

#define OS_Windows 0
#include <stdio_ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#elif defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
#include <stdio_ext.h>
#include <unistd.h> 
#include <getopt.h> 

#endif
#include "getopt.h"

#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif

#define MAX_VERSION	1
#define MIN_VERSION	5
#define SIZE 999999

#define ARQUIVO_ESCOLHIDO 1

//ANSWERS
#define EXIT_SUCESSO 0
#define EXIT_ERRO -1
using namespace std;

//VARIAVEIS GLOBAIS  
ofstream logdir, logrotate, agenda , voucherCreate, voucherlistCreate, qteItem;
int i = 0,  viaPinpad = 0, flag = 0, acao = 1, lixo = 0, portSrv =0, socket_desc;
string DATA_ATUAL;
string trace, command, action, type, layoutAfter, cpf , ipSrv, crmDisable, ignoreDisable;
char dta[11], dta_log[30], delimit[] = "|", delimit2[] = "/", codItem[15], store[5], pos[4], respostaServidor[SIZE];
int CUPOM = 0, status = 0, state = 0, ticket = 0, sequence = 0, resposta;
float qtde = 0, unitPrice = 0, total = 0;
FILE *crm_sales, *agendamento, *TRACE_LOG = NULL, *voucher, *voucherlst;
bool DEBUG = false;

//PROTOTIPOS DE FUNCOES
void criarAgendamento(void);
void IdClienteCRM(void);
void evaluate(void);
void evaluateBegin(void);
int finalizar(int);
int validaCPF(string);
void consultaCrm(string);
void aplicarDescontoItem(void);
void queimarVoucher(string lst);
//void geraLog	   		(void);
void vTrace(string, ...);
int createSocketClient(string, int);
void enviaMsgSocket(string);
void recebeMsgSocket();
void limpabuffer(void);
struct Lista{
	string sku;
	string interno;
	float descpercent;
	int fixo1;
	int fixo2;
	float qteMax;
	int fixo3;
	int idVoucher;
};

struct ListQteVendaItem {
	float qteItemVenda;
};

struct Lista list[9999];

struct ListQteVendaItem listItem[255];

int createSocketClient(string ipServidor, int portaServidor) {
	vTrace("createSocketClient - Inicio.");
	//	TIMEVAL = Timeout;
	//	Timeout.Tv_sec = timeout;
	//	timeout = 0;
	struct sockaddr_in servidor;

	//	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (socket_desc == -1) {
		vTrace("Nao foi possivel criar socket");
		if ( type == "self" && !layoutAfter.empty() ){
			cout << "LAYOUT " << layoutAfter << endl;
		}

		finalizar(EXIT_SUCESSO);
		exit(0);
	}
	servidor.sin_addr.s_addr = inet_addr(ipServidor.c_str());
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(portaServidor);
	vTrace("Aguarde, iniciando comunicacao...");

	if (connect(socket_desc, (struct sockaddr *) &servidor, sizeof(servidor)) < 0) {
		vTrace("Nao foi possivel conectar . ");
		if ( type == "self" && !layoutAfter.empty() ){
			cout << "LAYOUT " << layoutAfter << endl;
		}

		finalizar(EXIT_SUCESSO);
		exit(0);
	}
	vTrace("Conectado !");
	vTrace("createSocketClient - Fim.");
	return 0;
}

void enviaMsgSocket(string mensagem) {
	vTrace("Enviar mensagem socket - Inicio.");
	if (send(socket_desc, mensagem.c_str(), strlen(mensagem.c_str()), 0) < 0) {
		vTrace("Erro ao enviar mensagem!");
		finalizar(EXIT_SUCESSO);
		exit(0);
	}
	vTrace("Enviar mensagem socket - Fim.");

}

void limpabuffer(void) // Funçao utilitária para limpar o buffer do teclado
{
	char c;
	while ((c = getchar()) != '\n' && c != EOF);
}


void recebeMsgSocket() {
	int valRead = 0, tmp = 3 ;
	vTrace("Receber  mensagem socket - Inicio.");
	memset(respostaServidor, 0, sizeof respostaServidor);
	while (tmp > -1){
		cout << "DISPLAY Consultando, aguarde ... (" << tmp << ")" << endl;
		sleep(1);
		tmp--;
	}

	valRead = read(socket_desc, respostaServidor, SIZE);

	logdir << dta_log << "Execucao leitura -> " << valRead << endl;

	vTrace("Receber mensagem socket - Fim.");
}

void closeSocket() {
	vTrace("Fechar socket - Inicio.");
	close(socket_desc);
	vTrace("Fechar socket - Fim.");

}

void queimarVoucher(string lst){
	vTrace("Queimar Vouvher. - Inicio");

	logdir << dta_log << "Verificando conexao com o Servidor..." << endl;

	if (createSocketClient(ipSrv, portSrv) == 0) {

		logdir << dta_log << "Conectado !" << endl;

		string queimar; 

		queimar = "php -q crmsales.php --command=marcar --voucher_id='" + lst + "' --store=" + store + " --pos=" + pos;
		logdir << dta_log << "ENVIO .: php -q crmsales.php --command=marcar --voucher_id='" << lst << "' --store=" << store << " --pos=" << pos << endl;

		enviaMsgSocket(queimar.c_str());
		closeSocket();

	}else{
		logdir << dta_log << "Falha ao Conectar ao Servidor." << endl;

	}
	vTrace("Queimar Vouvher. - Fim");
}

void consultaCrm(string CPFCons){
	vTrace("Consulta socket. - Inicio");

	cout << "DISPLAY Verificando conexao com o Servidor..." << endl;
	logdir << dta_log << "DISPLAY Verificando conexao com o Servidor..." << endl;

	if (createSocketClient(ipSrv, portSrv) == 0) {

		cout << "DISPLAY Conectado !" << endl;

		string cliente; 

		cliente = "php -q crmsales.php --command=consultar --cpf=" + CPFCons + " --store=" + store + " --pos=" + pos;
		logdir << dta_log << "ENVIO .: php -q crmsales.php --command=consultar --cpf=" << CPFCons << " --store=" << store << " --pos=" << pos << endl;

		cout << "DISPLAY Consultando ..." << endl;

		enviaMsgSocket(cliente.c_str());
		recebeMsgSocket();


		logdir << dta_log << "RETORNO .: " << respostaServidor << endl;

		char *tk; string  voucherCli,stk;
		voucherCli = respostaServidor;	
		tk = strtok(respostaServidor, "|");
		stk = tk;

		if (stk == "-1"){
			logdir << dta_log << "Error no Retorno .: " << respostaServidor << endl;

		}else if (stk == "-2"){
			logdir << dta_log << "Cliente nao possui voucher .: " << respostaServidor << endl;
		}
		else{	
			int crltAteos = 0;
			voucher = fopen("/var/venditor/WRK/VOUCHER.crm", "w");
			if (voucher){
				voucherlistCreate.open("/var/venditor/WRK/VOUCHER.crm", ofstream::app);
				if (voucherlistCreate.is_open()){
					voucherlistCreate << voucherCli;
					logdir << dta_log << " VOUCHER CRIADO! " << endl;
					crltAteos = 1;
				}else{
					voucherlistCreate.clear();
				}
				fclose(voucher);
			}

			closeSocket();
		}
	}else{
		cout << "DISPLAY Falha ao Conectar ao Servidor." << endl;
		if ( type == "self" && !layoutAfter.empty() ){
			cout << "LAYOUT " << layoutAfter << endl;
		}
		finalizar(EXIT_SUCESSO);

	}
	vTrace("Consulta socket. - Fim");
}

int validaCPF(string CPFInformado){

	int digito1 =0 , digito2 = 0, i, j, CpfSize = 0;
	CpfSize = CPFInformado.size();
	if(CpfSize != 11){
		return 0;
	}
	else if((CPFInformado == "00000000000")  || (CPFInformado == "11111111111") || (CPFInformado == "22222222222")  ||
			(CPFInformado == "33333333333") || (CPFInformado == "44444444444") || (CPFInformado == "55555555555") ||
			(CPFInformado == "66666666666") || (CPFInformado == "77777777777") || (CPFInformado == "88888888888") ||
			(CPFInformado == "99999999999")){
		return 0; ///se o CPF tiver todos os números iguais ele é inválido.
	}
	else{
		//digito 1---------------------------------------------------
		for(i = 0, j = 10; i < CpfSize-2; i++, j--) //multiplica os números de 10 a 2 e soma os resultados dentro de digito1
			digito1 += (CPFInformado[i]-48) * j;
		digito1 %= 11;
		if(digito1 < 2)
			digito1 = 0;
		else
			digito1 = 11 - digito1;
		if((CPFInformado[9]-48) != digito1)
			return 0; //se o digito 1 não for o mesmo que o da validação CPF é inválido
		else	//digito 2--------------------------------------------------
		{
			for(i = 0, j = 11; i < CpfSize-1; i++, j--) //multiplica os números de 11 a 2 e soma os resultados dentro de digito2
				digito2 += (CPFInformado[i]-48) * j;
			digito2 %= 11;
			if(digito2 < 2)
				digito2 = 0;
			else
				digito2 = 11 - digito2;
			if((CPFInformado[10]-48) != digito2)
				return 0; //se o digito 2 não for o mesmo que o da validação CPF é inválido
		}
	}
	return 1;

};

int finalizar(int tipoEncerramento) {
	if (tipoEncerramento == EXIT_ERRO) {
		vTrace("Encerrado com erro.");
		vTrace("--------------------------------------------------------- Fim.");
		logdir.close();
		cout << "BYE -1" << endl;
		exit(-1);
		return -1;
	}
	else if (tipoEncerramento == EXIT_SUCESSO) {
		vTrace("Encerrado com sucesso.");
		vTrace("--------------------------------------------------------- Fim.");
		logdir.close();
		cout << "BYE 1" << endl;
	}
	else {
		vTrace("Encerrado.");
		vTrace("--------------------------------------------------------- Fim.");
		logdir.close();
		cout << "BYE 1" << endl;
	}
	return 0;
}

void aplicarDescontoItem(void){
	vTrace("aplicarDescontoItem - Inicio");
	FILE *itemVenda;	
	voucher = fopen("/var/venditor/WRK/VOUCHER.crm" , "r");
	if (voucher){
		char *token2, *retorno, linha[SIZE];
		string id, id1, ean, ean1, idLocal;

		while (!feof(voucher) && i < SIZE){
			retorno = fgets(linha, SIZE, voucher);
			if(retorno)
				token2 = strtok(retorno, delimit);
			int j = 0;
			while (token2 != NULL){
				switch (j){
					case 0:
						if (strlen(token2) > 0){
							list[i].sku=token2;
						}
						break;
					case 1:
						if (strlen(token2) > 0){
							list[i].interno=token2;
						}
						break;
					case 2:
						if (strlen(token2) > 0){
							list[i].descpercent = atof(token2);
						}
						break;
					case 3:
						if (strlen(token2) > 0){
							list[i].fixo1= atoi(token2);
						}	
						break;
					case 4:
						if (strlen(token2) > 0){
							list[i].fixo2= atoi(token2);
						}
						break;
					case 5:
						if (strlen(token2) > 0){
							list[i].qteMax= atof(token2);
						}
						break;
					case 6:
						if (strlen(token2) > 0){
							list[i].fixo3= atoi(token2);
						}
						break;
					case 7:
						if (strlen(token2) > 0){
							list[i].idVoucher= atoi(token2);
						}
						break;
				}
				token2 = strtok(NULL, delimit);

				j++;	
			}
			//			logdir << dta_log << "SKU: " << list[i].sku << " INTERNO: " << list[i].interno << " %: "<< list[i].descpercent << " FIXO 4: " << list[i].fixo1 << " FIXO 5: " << list[i].fixo2 << " QTE: " << list[i].qteMax << " FIXO 7: " << list[i].fixo3 << " ID: " << list[i].idVoucher << endl;

			id1 = "";
			ean1 = "";
			id = list[i].interno;
			ean = list[i].sku;
			//printf( " %s\n",item.c_str());
			//	int ct2 = 0;
			int tid = 0,tean = 0;
			tid = id.length();
			tean = ean.length(); 
			for (int ct = 0; ct < tid;ct++){
				if (ct > 0){
					id1 += id[ct];
					//				ct2++;	
				}
			}

			for (int ct = 0; ct < tean;ct++){
				if (ct > 0){
					ean1 += ean[ct];
					//			ct3++;
				}
			}
			//	printf("SKU: %s ID: %s\n",ean1.c_str(),id1.c_str());
			//	printf("cod lista : %s  codigo informado: %s\n", ean1.c_str(), codItem);	

			if ( codItem == ean1 ){
				float qtd_vd = 0.000,qtd_vd_ = 0.000, newTotal = 0.000, desconto = 0.000;

				logdir << dta_log << " Cod informado [" << codItem  << "] - Codigo Lista [" <<  ean1 << "]" << endl;
				logdir << dta_log << " -------------------------------------------" << endl;

				logdir << dta_log << " Cupom...............[" << CUPOM  << "] " << endl;
				logdir << dta_log << " Produto.............[" << list[i].interno << "] " << endl;
				logdir << dta_log << " Quantidade Maxima...[" << list[i].qteMax << "] " << endl;
				logdir << dta_log << " Percentual Desconto.[" << list[i].descpercent << "] " << endl;
				logdir << dta_log << " Voucher Code........[" << list[i].fixo3  << "] " << endl;
				logdir << dta_log << " Voucher ID..........[" << list[i].idVoucher  << "] " << endl;
				logdir << dta_log << " -------------------------------------------" << endl;

				idLocal = "/var/venditor/WRK/" + id1 + ".crm";	
				//printf(" %s\n",idLocal.c_str());

				itemVenda = fopen(idLocal.c_str() , "r");
				if (itemVenda){
					int qt = 0;
					char *ret , *tk, line[255];
					while (!feof(itemVenda) && qt < 255 ){
						ret = fgets(line, 255, itemVenda);
						if (ret)
							tk = strtok(ret, delimit);
						int v = 0;
						while (tk != NULL){
							switch (v){
								case 0:
									if (strlen(tk) > 0){
										listItem[qt].qteItemVenda= atof(tk);
										qtd_vd = listItem[qt].qteItemVenda;
									}	
									break;
							}
							tk = strtok(NULL, delimit);
							v++;
						}
						logdir << dta_log << " Quantidade p/ aplicar...[" << listItem[qt].qteItemVenda << "] " << endl;
						qt++;
					}
					fclose(itemVenda);
				}

				qtd_vd_ = qtd_vd + qtde;
				logdir << dta_log << " Quantidade Total Vendida[" << qtd_vd_ << "] " << endl;
				float qtdValida = 0;
				if ( qtd_vd_ < list[i].qteMax ){
					logdir << dta_log << " Quantidade Vendida, menor que quantidade Maxima." << endl;
					qtdValida = qtd_vd_;
				}else{
					qtdValida = list[i].qteMax - qtd_vd;
					logdir << dta_log << " Saldo Valido [" << qtdValida << "] = Qte p/ aplicar [" << list[i].qteMax << "] - Qte Vendida [" << qtd_vd << "]" << endl;
					if ( qtde > qtdValida ){
						logdir << dta_log << " Saldo Valido [" << qtdValida << "] - quantidade informada  [" << qtde << "]" << endl;
						qtde = qtdValida;


					}
				}


				if (qtdValida > 0 ){
				//	total = qtd_vd_ * unitPrice;	
					total = qtde * unitPrice;	
					logdir << dta_log << " 1 - quatidade vendida .[" << qtd_vd_ << "]"   << endl;
					logdir << dta_log << " 2 - quatidade venda atual .[" << qtde << "] -  preco unitario [" << unitPrice << "]" << endl;
					logdir << dta_log << " Total antes do desconto. [" << total << "] " << endl;

					//float trucante = floorf(unitPrice - (unitPrice / 100.00 * list[i].descpercent )) * qtd_vd_;
				//	newTotal = (unitPrice - (unitPrice / 100.00 * list[i].descpercent )) * qtd_vd_;
					newTotal = (unitPrice - (unitPrice / 100.00 * list[i].descpercent )) * qtde;
					logdir << dta_log << " Total com desconto......[" << newTotal << "] " << endl;
					desconto = total - newTotal;
					//cout.precision(2);
					char descFormat [50];

					sprintf (descFormat, "%.2f", desconto);
					logdir << dta_log << " Desconto: [" << desconto << "] =  total [" << total << "] - newTotal [" << newTotal << "]" << endl;
					logdir << dta_log << " Valor do Desconto.......[" << desconto << "] " << endl;
					logdir << dta_log << " Desconto Format.........[" << descFormat << "]" << endl;
					cout << "AUTHORIZE OFF" << endl;
					cout << "COMMAND 85 " << sequence << " " << descFormat << endl;
					cout << "AUTHORIZE ON" << endl;

					cout << "STEP ANSWER 379 0 Voucher_ID " << list[i].idVoucher << endl;
					cout << "INTERNAL ANSWER 379 " << list[i].idVoucher << endl;

					int ctrlLst = 0;
					voucherlst = fopen("/var/venditor/WRK/VOUCHERLST.crm" , "r");
					if (voucherlst){
						//logdir << dta_log << " Existe lista de Voucher." << endl;
						int qt = 0;
						char *ret , *tk, line[255];
						while (!feof(voucherlst) && qt < 255){
							ret = fgets(line, 255, voucherlst);
							if (ret)
								tk =strtok(ret,delimit);
							int v = 0;
							while (tk != NULL){

								if (strlen(tk) > 0 ){
									int idV = atoi(tk);
									if ( idV == list[i].idVoucher ){
										logdir << dta_log << " Ja existe o ID [" << idV << "] , na lista de voucher." << endl;
										ctrlLst = 1;	
									}
								}

								//	logdir << dta_log << " Resultado tk [" << tk << "] " << endl;
								tk = strtok(NULL, delimit);
								v++;
							}
							qt++;
						}
						fclose(voucherlst);
					}

					if (ctrlLst == 0 ){
						voucherlst = fopen("/var/venditor/WRK/VOUCHERLST.crm" , "r");
						if (voucherlst){
							voucherlst = fopen("/var/venditor/WRK/VOUCHERLST.crm" , "a");
							voucherCreate.open("/var/venditor/WRK/VOUCHERLST.crm", ofstream::app);
							if (voucherCreate.is_open()){
								voucherCreate << "|" << list[i].idVoucher;	
								logdir << dta_log << " Adicionando ID [" << list[i].idVoucher << "] , na lista de Voucher."<< endl;
							}
							fclose(voucherlst);
						}else{
							voucherlst = fopen("/var/venditor/WRK/VOUCHERLST.crm" , "w");
							if (voucherlst){
								voucherCreate.open("/var/venditor/WRK/VOUCHERLST.crm", ofstream::app);
								if (!voucherCreate.is_open()){
									voucherCreate.clear();	
								}else{
									voucherCreate << list[i].idVoucher;	
									logdir << dta_log << " Criando lista de voucher e adicionando o ID [" << list[i].idVoucher << "]."<< endl;
								}
								fclose(voucherlst);

							}
						}
					}

					itemVenda = fopen(idLocal.c_str() , "w");
					if (itemVenda){
						qteItem.open(idLocal.c_str(),ofstream::app);
						if (!qteItem.is_open()){
							qteItem.clear();
						}else{
							qteItem << qtd_vd_ ;
							logdir << dta_log << " Adicionado a Qte [" << qtd_vd_ << "], no controle do item."<< endl;
						}	
						fclose(itemVenda);
					}

				}
				i++;
			}

		}
	}
}

void criarAgendamento(void) {
	vTrace("Inicio agendar. ");
	char *token2, *retorno, linha2[255];

	agendamento = fopen("/var/venditor/WRK/AT_EOS.dat", "r");
	if (!agendamento) {
		agenda.open("/var/venditor/WRK/AT_EOS.dat", ofstream::app);
		if (!agenda.is_open()) {
			vTrace("Nao foi possivel abrir o arquivo de agendamento.");
			agenda.clear();
		}
		else {
			cout << "AT_EOS SHELL ./crm_sales --command=rm\n";
			vTrace("Agendamento criado.");
		}
	}
	else {
		while (!feof(agendamento) && i < 255) { // Loop para ler cada linha do arquivo enquanto houver linhas
			retorno = fgets(linha2, 255, agendamento);
			if (retorno)
				token2 = strtok(retorno, delimit2);
			int j = 0;
			while (token2 != NULL) {
				switch (j) {
					case 1:
						if (strcmp(token2, "crm_sales") > 0) {
							vTrace("Ja existe um agendamento criado. ");
						}
						else {
							agenda.open("/var/venditor/WRK/AT_EOS.dat", ofstream::app);
							if (!agenda.is_open()) {
								vTrace("Nao foi possivel abrir o arquivo de agendamento. ");
								agenda.clear();
							}
							cout << "AT_EOS SHELL ./crm_sales --command=rm\n";
							vTrace("Agendamento criado. ");
						}
						break;
				}
				//	printf(" %s\n", token2);
				token2 = strtok(NULL, delimit2);
				j++;
			}
		}
	}
}


void evaluateBegin(void){
	cout << "EVALUATE {STATUS}" << endl;
	cin >> status;
	setbuf(stdin, NULL);

	cout << "EVALUATE {STATE}" << endl;
	cin >> state;
	setbuf(stdin, NULL);

	cout << "EVALUATE {CUPOM}" << endl;
	cin >> CUPOM;
	setbuf(stdin, NULL);

	cout << "EVALUATE {EMPORIUM_IP}" << endl;
	cin >> ipSrv;
	setbuf(stdin, NULL);

	cout << "EVALUATE {EMPORIUM_PORT}" << endl;
	cin >> portSrv;
	setbuf(stdin, NULL);

	cout << "EVALUATE {STORE}" << endl;
	cin >> store;
	setbuf(stdin, NULL);

	cout << "EVALUATE {POS}" << endl;
	cin >> pos;
	setbuf(stdin, NULL);

	cout << "EVALUATE {SALE_TICKET}" << endl;
	cin >> ticket;
	setbuf(stdin, NULL);

	cout << "EVALUATE {CRMDISABLE}" << endl;
	cin >> crmDisable;
	setbuf(stdin, NULL);

}

void evaluate(void){
	cout << "EVALUATE {STATUS}" << endl;
	cin >> status;
	setbuf(stdin, NULL);

	cout << "EVALUATE {STATE}" << endl;
	cin >> state;
	setbuf(stdin, NULL);

	cout << "EVALUATE {CUPOM}" << endl;
	cin >> CUPOM;
	setbuf(stdin, NULL);

	cout << "EVALUATE {ITEM_COD_TOTAL}" << endl;
	cin >> codItem;
	setbuf(stdin, NULL);

	cout << "EVALUATE {ITEM_QUANTIDADE}" << endl;
	cin >> qtde;
	setbuf(stdin, NULL);

	cout << "EVALUATE {ITEM_UNIT_PRICE}" << endl;
	cin >> unitPrice;
	setbuf(stdin, NULL);

	cout << "EVALUATE {ITEM_SEQUENCE}" << endl;
	cin >> sequence;
	setbuf(stdin, NULL);

	cout << "EVALUATE {ITEM_TOTAL_AMOUNT}" << endl;
	cin >> total;
	setbuf(stdin, NULL);

	cout << "EVALUATE {EMPORIUM_IP}" << endl;
	cin >> ipSrv;
	setbuf(stdin, NULL);

	cout << "EVALUATE {EMPORIUM_PORT}" << endl;
	cin >> portSrv;
	setbuf(stdin, NULL);

	cout << "EVALUATE {STORE}" << endl;
	cin >> store;
	setbuf(stdin, NULL);

	cout << "EVALUATE {POS}" << endl;
	cin >> pos;
	setbuf(stdin, NULL);

	cout << "EVALUATE {SALE_TICKET}" << endl;
	cin >> ticket;
	setbuf(stdin, NULL);
}

void IdClienteCRM(void) {
	vTrace("------------------------------------------------------ Inicio IdClienteCRM. ");
	while ( flag == 0 ){
		logdir << dta_log << "Acao : [" << acao << "]" << endl;
		switch (acao) {
			case 1:
				logdir << dta_log << "Identificacao do Cliente." << endl;
				if ( type == "self"){
					cout << "LAYOUT 26" << endl;
				}
				cout << "ACCEPT TITLE SOLICITE AO CLIENTE QUE" << endl;
				cout << "ACCEPT PROMPT INFORME O CPF NO PINPAD" << endl;
				
				cout << "COMMAND 111" << endl;
				cin >> lixo;
				setbuf(stdin, NULL);		
				
				cout << "ACCEPT PIN_MSG Digite Seu CPF:" << endl;
				cout << "ACCEPT PINDATA" << endl;
				getline(cin, cpf);
				setbuf(stdin, NULL);
				__fpurge(stdin);

				logdir << dta_log << "Entrada Pinpad cpf: " << cpf << endl;

				if (cpf.empty() || cpf.compare("NULL") == 0){
					vTrace(" CPF nao informado. Repetir?");
					acao = 2;
				}
				else{
					int valid;
					valid = validaCPF(cpf);

					if ( valid == 1 ){
						cout << "CLIENTECRM= " << endl;
						logdir << dta_log << "CPF Valido .: " << cpf << endl;
						cout << "CLIENTECRM=" << cpf << endl;
						cout << "INTERNAL ANSWER 191 " << cpf << endl;
						cout << "STEP ANSWER 191 0 Cliente " << cpf << endl;
						consultaCrm(cpf);
						criarAgendamento();
						flag =1;
					}
					else{
						logdir << dta_log << "CPF Invalido .: " << cpf << endl;
						acao = 3;
					}
				}

				break;
			case 2:
				if ( type == "self" ){
					cout << "LAYOUT 47" << endl;
					system("nohup /usr/bin/play /var/venditor/SOUND/erros_genericos.wav 2>> /dev/null &");
					cout << "ERROR=CPF nao informado." << endl;
					setbuf(stdin, NULL);

					int simnao = 0;

					cout << "ACCEPT TITLE CPF NAO INFORMADO." << endl;
					cout << "ACCEPT YESNO TENTAR NOVAMENTE?" << endl;
					cin >> simnao;
					setbuf(stdin, NULL);
					logdir << dta_log << "Resposta do CPF nao informado : " << simnao << endl;

					if ( simnao == 1 ){
						acao = 1;
					}else{
						cout << "ERROR=CPF nao informado." << endl;
						finalizar(EXIT_SUCESSO);	
					}
				}else{	
					cout << "ERROR=CPF nao informado." << endl;
					cout << "ACCEPT YESNO CPF nao informado. Deseja repetir? " << endl;
					cin >> resposta;
					setbuf(stdin, NULL);
					logdir << dta_log << "Resposta do CPF nao informado : " << resposta << endl;

					if ( resposta == 1 ){
						acao = 1;
					}else{
						cout << "ERROR=CPF nao informado." << endl;
						finalizar(EXIT_SUCESSO);	
					}
				}
				break;
			case 3:
				if ( type == "self" ){
					cout << "LAYOUT 47" << endl;
					system("nohup /usr/bin/play /var/venditor/SOUND/erros_genericos.wav 2>> /dev/null &");

					cout << "ERROR=CPF nao informado." << endl;
					cout << "ACCEPT TITLE CPF INVALIDO." << endl;
					cout << "ACCEPT YESNO TENTAR NOVAMENTE?" << endl;
					cin >> resposta;
					setbuf(stdin, NULL);
					logdir << dta_log << "Resposta do CPF nao informado : " << resposta << endl;

					if ( resposta == 1 ){
						acao = 1;
					}else{
						cout << "ERROR=CPF nao informado." << endl;
						finalizar(EXIT_SUCESSO);	
					}
				}else{
					cout << "ACCEPT YESNO CPF invalido. Deseja repetir? " << endl;
					cin >> resposta;
					setbuf(stdin, NULL);
					logdir << dta_log << "Resposta do invalido : " << resposta << endl;

					if ( resposta == 1 ){
						acao = 1;
					}else{
						finalizar(EXIT_SUCESSO);	
					}
					break;
				}
		}
	}
}

void vTrace(string texto, ...) {
	//Trabalhando com Datas
	time_t rawtime;
	struct tm *local;
	char dta_log[30];
	time(&rawtime);
	local = localtime(&rawtime);
	strftime(dta_log, 30, "%d-%m-%Y %H:%M:%S - ", local);
	logdir << dta_log << texto << endl;
	//	fprintf(logdir,"%s \n", texto);  
}

void help() {
	printf("Options:\n\n");
	printf("[--version -V]\n");
	printf("[<Output version and exit.>]\n\n");
	printf("[--help -h]\n");
	printf("[<Output help and exit.>]\n\n");
	printf("[--trace -t]\n");
	printf("[<file> Arquivo de log da aplicacao.]\n\n");
	printf("[--ignore-disable -i]\n");
	printf("[<Ignora variavel de crm desabilitado.>]\n\n");
	printf("[--via-pinpad -p]\n");
	printf("[<Entrada de telefone via Pinpad.>]\n\n");
	printf("[--type -T]\n");
	printf("[<Identificacao para SelfCheckout.>]\n\n");
	printf("[--layoutAfter -l]\n");
	printf("[<Layout apos Identificacao.>]\n\n");
	printf("[--command -C]\n");
	printf("[<Comando para funcao de entrada.>]\n\n");
	exit(0);

}

//INICIO --> main()
int main(int argc, char** argv) {
	char optc = 0;
	struct option OpcoesLongas[] = {
		{"trace"  , optional_argument  , NULL, 't'},
		{"command", required_argument  , NULL, 'C'},
		{"type", optional_argument  , NULL, 'T'},
		{"via-pinpad", optional_argument  , NULL, 'p'},
		{"ignore-disable", optional_argument  , NULL, 'i'},
		{"layoutAfter", optional_argument  , NULL, 'l'},
		{"help"   , no_argument        , NULL, 'h'},
		{"version", no_argument        , NULL, 'V'},
		{        0,                 0,    0,   0}
	};
	if (argc == 1) { // Sem argumentos
		printf("System %s\n", argv[0]);
		printf("Sem arqumentos!\n");
		help();
	}
	while ((optc = getopt_long(argc, argv, "tp:TilC:hV", OpcoesLongas, NULL)) != -1) {
		switch (optc) {
			case 'h':
				printf("System %s\n", argv[0]);
				help();
			case 'V': // Versao
				printf("======================================================\n");
				printf("Desenvolvido por.: Mario Sergio\n");
				printf("Contato..........: mario@remaqbh.com.br\n");
				printf("System %s\n", argv[0]);
				printf("Version %d.%d - Date: [%s %s]\n", MAX_VERSION, MIN_VERSION, __DATE__, __TIME__);
				printf("======================================================\n");
				exit(0);
			case 't': //trace
				trace = optarg;
				break;
			case 'C':
				command = optarg;
				break;
			case 'T': //Self 
				type = optarg;
				break;
			case 'l': 
				layoutAfter = optarg;
				break;
			case 'i': //ignora crm Disable
				ignoreDisable = optarg;
				break;

			case 'p': //viaPinpad
				viaPinpad = atoi(optarg);
				break;

			default: // Qualquer parametro nao tratado
				printf("System %s\n", argv[0]);
				printf("Parametro nao tratado %c \n", optc);
				help();
		}
	}
	crm_sales = fopen("/etc/logrotate.d/crmsales", "r");
	if (!crm_sales) {
		logrotate.open("/etc/logrotate.d/crmsales");
		logrotate << "/var/log/crmsales*.log {\n";
		logrotate << "    missingok \n";
		logrotate << "    daily \n";
		logrotate << "    rotate 30\n";
		logrotate << "    compress \n";
		logrotate << " } \n";
		logrotate.close();
		
		system("sudo chmod 644 /etc/logrotate.d/crmsales");
	}
	//Trabalhando com Datas
	time_t rawtime;
	struct tm *local;
	time(&rawtime);
	local = localtime(&rawtime);
	//Formatando datas
	strftime(dta, 11, "%Y%m%d", local);
	strftime(dta_log, 30, "%d-%m-%Y %H:%M:%S - ", local);
	DATA_ATUAL = dta;

	if (!trace.empty()) {
		//logdir.open(trace, ofstream::app); nao est� funcionando.
		logdir.open("/var/log/crmsales.log", ofstream::app);
		if (!logdir.is_open()) {
			printf(" Nao foi possivel abrir o arquivo de log!\n");
			logdir.clear();
		}
	}
	else {
		logdir.open("/var/log/crmsales.log", ofstream::app);
		if (!logdir.is_open()) {
			printf(" Nao foi possivel abrir o arquivo de log!\n");
			logdir.clear();
		}
	}
	vTrace("--------------------------------------------------------- Inicio.");

	if (command == "Customer"){

		evaluateBegin();
		cpf = "0";
		logdir << dta_log << "**** LOJA: " << store << " - PDV: " << pos << " - CUPOM: " << CUPOM << " - VERSAO.: " << MAX_VERSION << "." << MIN_VERSION  << " ****"<< endl;
		logdir << " Status CRM disable [" << crmDisable << "]" << endl; 
		if ( ignoreDisable == "1" )
			crmDisable = "0";
		
		if ( cpf == "0" && crmDisable != "1" ){

			logdir << dta_log << " IDENTIFICACAO CRM ." << endl;
			logdir << dta_log << " Situacao inicial, aguardando confirmacao: [" << resposta << "]" << endl;

			if ( state == 0 ){
				if ( type == "self" ){
					cout << "LAYOUT 27" << endl;
				}
				cout << "ACCEPT TITLE SOLICITE AO CLIENTE QUE" << endl;
				if (viaPinpad == 1 ){
			/*		string pin;
					cout << "ACCEPT PROMPT CONFIRME NO PIN PAD." << endl;
					cout << "ACCEPT PIN_MSG *   PROMOCOES  *IDENTIFICAR CPF?" << endl;
					cout << "ACCEPT PIN_YESNO" << endl;
					getline(cin, pin);
					setbuf(stdin, NULL);
					__fpurge(stdin);
					
					logdir << dta_log << "Resposta pin yes no: [" << pin << "]" << endl;

					if ( pin == "NULL" ){
						resposta = 0;
					}else{
						resposta = 1;
					}
			*/
					resposta = 1;
				}else{
					cout << "ACCEPT YESNO IDENTIFICAR CPF NO CUPOM? " << endl;
					cin >> resposta;
					setbuf(stdin, NULL);
				}
				logdir << dta_log << " Situacao final para identificar cpf : [" << resposta << "]" << endl;
			}else{
				resposta = 1;

			}

			if ( resposta == 1 ){
				IdClienteCRM();
			}else{
				cout << "CLIENTECRM= " << endl;
				if ( type == "self" && !layoutAfter.empty() ){
					cout << "LAYOUT " << layoutAfter << endl;
				}

			}
			if (state > 0 ){
				cout << "COMMAND 14" << endl;
				cin >> lixo;
				setbuf(stdin, NULL);	
			}

		}	       

		if ( type == "self" && !layoutAfter.empty() ){
			cout << "LAYOUT " << layoutAfter << endl;
		}

		finalizar(EXIT_SUCESSO);	

	}
	else if (command == "rm"){
		vTrace("Ateos - Inicio");
		voucherlst = fopen("/var/venditor/WRK/VOUCHERLST.crm","r");
		if (voucherlst){
			evaluateBegin();
			cout << "CLIENTECRM= " << endl;
			if (action.empty()) {
				vTrace("Carregando action.");
				cout << "EVALUATE {ACTION}" << endl;
				getline(cin, action);
				setbuf(stdin, NULL);
			}
			logdir << dta_log << "Action. " << action << endl;
			if ((action == "OK") || (action.empty())) {

				char line[SIZE];string lstVoucher;
				lstVoucher = fgets(line ,SIZE, voucherlst);

				logdir << dta_log << "Lista de Voucher: [" << lstVoucher << "]" << endl;
				queimarVoucher(lstVoucher);
			}

			remove("/var/venditor/WRK/VOUCHERLST.crm");
		}
		remove("/var/venditor/WRK/VOUCHER.crm");
		system("rm -f /var/venditor/WRK/*.crm 2> /dev/null");
		vTrace("Ateos - Fim");
		finalizar(EXIT_SUCESSO);	
	}
	else if (command == "Apply-discount"){
		evaluate();
		if ( codItem != NULL )
			aplicarDescontoItem();	


		finalizar(EXIT_SUCESSO);	
	}
	else {
		logdir << dta_log << "Comando invalido. " << command << endl;
	}
	finalizar(EXIT_SUCESSO);
}
