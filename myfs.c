/*
 *  myfs.c - Implementacao do sistema de arquivos MyFS
 *
 *  Autores: Larissa Rezende Fazza - 202335021
 *  Projeto: Trabalho Pratico II - Sistemas Operacionais
 *  Organizacao: Universidade Federal de Juiz de Fora
 *  Departamento: Dep. Ciencia da Computacao
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "myfs.h"
#include "vfs.h"
#include "inode.h"
#include "util.h"

// Declaracoes globais
char fsid = 3;						// Identificador do tipo de sistema de arquivos
char *fsname = "LarissaFileSystem"; // Nome do tipo de sistema de arquivos
int myFSslot;

struct fileDescriptor
{
	int status; // 0 se estiver fechado, 1 se estiver aberto.
	int type;
	unsigned int fd;
	Disk *disk;
	Inode *inode;
	char *path;
};

typedef struct fileDescriptor FileDescriptor;
FileDescriptor *fileDescriptors[MAX_FDS];

#define SUPER_SECTOR 0			   //    Bloco em que o superblock é escrito
#define SUPER_METADATA_NUM_ITEMS 8 //    Quantidade de metadados que superblock possui
#define SUPER_FS_ID 0			   //    Item 1: Número identificador de FS
#define SUPER_BLOCK_SIZE 1		   //    Item 2: Tamanho de bloco
#define SUPER_BLOCK_AMOUNT 2	   //    Item 3: Quantidade de blocos
#define SUPER_INODE_AMOUNT 3	   //    Item 4: Quantidade de inodes
#define SUPER_INODE_BEGIN_SECTOR 3 //    Item 4: Quantidade de inodes
#define SUPER_ROOT_BEGIN_SECTOR 3  //    Item 4: Quantidade de inodes
#define SUPER_ROOT_END_SECTOR 3	   //    Item 4: Quantidade de inodes
#define SUPER_DATA_BITMAP_SECTOR 6 //    Item 6: Bloco em que começa bitmap de dados
#define SUPER_DATA_AMOUNT 7		   //    Item 4: Quantidade de blocos de dados
#define SUPER_DATA_BEGIN_SECTOR 8  //    Item 8: Bloco em que começa os dados

// função auxiliar para calcular o tamanho de um bloco em bytes
unsigned int getBlockSize(Disk *d)
{
	unsigned char *sectorBuf[DISK_SECTORDATASIZE];
	if (diskReadSector(d, SUPER_SECTOR, sectorBuf) == -1)
		return -1;

	unsigned int blockSize;
	char2ul(sectorBuf[SUPER_BLOCK_SIZE], blockSize);
	return blockSize;
}

// Funcao para verificacao se o sistema de arquivos está ocioso, ou seja,
// se nao ha quisquer descritores de arquivos em uso atualmente. Retorna
// um positivo se ocioso ou, caso contrario, 0.
int myFSIsIdle(Disk *d)
{
	// EXPLICANDO A FUNÇÃO: passa por todos os descritores de arquivos (o vetor de fds fd[]),
	// verificando se existe algum deles em uso ou seja, verifica se existe algum file descriptor
	// no sistema de arquivos que, simultaneamente não está vazio e se está salvo em um disco d existente.
	// Caso algum filedescriptor esteja em uso, a função retorna zero, porém, se percorrer a lista toda
	// e nenhum descritor de arquivo estiver preenchido e conectado corretamente, a unção retorna 1
	// pois significa que o sistema de arquivos está ocioso.
	int i = 0;
	while (i < MAX_FDS)
	{
		if (fileDescriptors[i] != NULL && diskGetId(d) == diskGetId(fileDescriptors[i]->disk))
			return 0;
		else
			i++;
	}
	return 1;
}

// Funcao para formatacao de um disco com o novo sistema de arquivos
// com tamanho de blocos igual a blockSize. Retorna o numero total de
// blocos disponiveis no disco, se formatado com sucesso. Caso contrario,
// retorna -1.
int myFSFormat(Disk *d, unsigned int blockSize)
{
	// blockSize = tamanho de cada bloco

	// unsigned int numBlocks = (diskGetNumSectors(d) - inodeAreaBeginSector()) / blockSize;

	return -1;
}

// Funcao para abertura de um arquivo, a partir do caminho especificado
// em path, no disco montado especificado em d, no modo Read/Write,
// criando o arquivo se nao existir. Retorna um descritor de arquivo,
// em caso de sucesso. Retorna -1, caso contrario.
int myFSOpen(Disk *d, const char *path)
{
	return -1;
}

// Funcao para a leitura de um arquivo, a partir de um descritor de
// arquivo existente. Os dados lidos sao copiados para buf e terao
// tamanho maximo de nbytes. Retorna o numero de bytes efetivamente
// lidos em caso de sucesso ou -1, caso contrario.
int myFSRead(int fd, char *buf, unsigned int nbytes)
{
	// verificar se o arquivo está aberto e, se não esiver, retorna -1
	// e verifica se fd possui um valor válido
	if (fileDescriptors[fd] == NULL || fileDescriptors[fd]->status == 0 || fd <= 0 || fd > MAX_FDS)
		return -1;

	Inode *i = fileDescriptors[fd]->inode;
	Disk *d = fileDescriptors[fd]->disk;
	unsigned int blockSize = getBlockSize(d);

	unsigned int blockAddr = inodeGetBlockAddr(i, 0);
	if (blockAddr == -1)
	{
		printf("\nUm erro ocorreu, bloco não encontrado no Inode");
		return -1; // um erro ocorreu
	}

	// o número de setores por bloco é igual ao tamanho do bloco em bytes, dividido pelo número de bytes por setor
	unsigned int sectorsPerBlock = blockSize / DISK_SECTORDATASIZE; // DISK_SECTORDATASIZE = Tamanho padrao do setor em bytes
	unsigned int firstSector = (blockAddr - 1) * sectorsPerBlock + 1;
	unsigned int bytesRead = 0; // quantos bytes foram lidos do arquivo

	while (bytesRead < nbytes) // enquanto o número de bytes lidos for menor que o máximo permitido, fazer leitura
	{
		for (int i = firstSector; i < sectorsPerBlock; i++) // percorrer todos os setores do bloco
		{
			int readingResult = diskReadSector(d, i, buf);
			if (readingResult == -1)
			{
				printf("Um erro ocorreu ao ler o setor do disco");
				return -1;
			}
			bytesRead += readingResult;
		}
	}

	return bytesRead;
}

// Funcao para a escrita de um arquivo, a partir de um descritor de
// arquivo existente. Os dados de buf serao copiados para o disco e
// terao tamanho maximo de nbytes. Retorna o numero de bytes
// efetivamente escritos em caso de sucesso ou -1, caso contrario
int myFSWrite(int fd, const char *buf, unsigned int nbytes)
{
	// verificar se o arquivo está aberto e, se não esiver, retorna -1
	// e verifica se fd possui um valor válido
	if (fileDescriptors[fd] == NULL || fileDescriptors[fd]->status == 0 || fd <= 0 || fd > MAX_FDS)
		return -1;

	Inode *i = fileDescriptors[fd]->inode;
	Disk *d = fileDescriptors[fd]->disk;
	unsigned int blockSize = getBlockSize(d);

	// achar um bloco livre
	unsigned int freeBlock = getFreeBlock();
	if (inodeAddBlock(i, freeBlock) == -1)
	{
		"Não foi possível escrever arquivo";
		return -1;
	}

	// Tamanho padrao do setor de qualquer disco, em bytes, definido como DISK_SECTORDATASIZE 512
	unsigned int freeBlock;						   // caminho do bloco livre
	unsigned int bytesWritten = 0;				   // quantos bytes foram escritos do arquivo
	unsigned int firstSector = 0;				   // qual é o primeir setor desse bloco
	unsigned int sectorsPerBlock = 0;			   // quantidade de setores por bloco
	unsigned char *sectorBuf[DISK_SECTORDATASIZE]; // setor do disco, de tamanho máximo 512

	while (bytesWritten < nbytes)
	{
		freeBlock = getFreeBlock();
		if (inodeAddBlock(i, freeBlock) == -1)
		{
			return -1;
		}

		// o número de setores por bloco é igual ao tamanho do bloco em bytes, dividido pelo número de bytes por setor
		sectorsPerBlock = blockSize / DISK_SECTORDATASIZE; // DISK_SECTORDATASIZE = Tamanho padrao do setor em bytes

		unsigned int dataBeginSector;
		char2ul(sectorBuf[SUPER_DATA_BEGIN_SECTOR], dataBeginSector); // setor onde começam os dados

		firstSector = freeBlock * sectorsPerBlock + dataBeginSector; // primeiro setor do bloco

		int j = 0;
		for (int i = firstSector; i < sectorsPerBlock; i++)
		{
			for (j = 0; j < DISK_SECTORDATASIZE; j++) // percorro o vetor de dados
			{
				if (!buf[j])
				{
					return bytesWritten += j;
				}
				sectorBuf[j] = buf[j];
			}
			if (diskWriteSector(d, i, sectorBuf) == -1)
			{
				printf("Um erro ocorreu ao escrever setor no disco");
				return -1;
			}
		}

		bytesWritten += j;
	}

	return bytesWritten;
}

// Funcao para fechar um arquivo, a partir de um descritor de arquivo
// existente. Retorna 0 caso bem sucedido, ou -1 caso contrario
int myFSClose(int fd)
{
	// verificar se o arquivo está aberto e, se não esiver, retorna -1
	// e verifica se fd possui um valor válido
	FileDescriptor *fileDescriptor = fileDescriptors[fd];
	if (fileDescriptor == NULL || fileDescriptor->status == 0 || fd <= 0 || fd > MAX_FDS)
	{
		printf("O descritor de arquivo já está fechado\n");
		return -1;
	}

	fileDescriptors[fd]->status = 0;			 // muda o status do file descriptor
	fileDescriptors[fd]->disk == NULL;			 // limpa o ponteiro pro disco no file descriptor
	if (inodeClear(fileDescriptor->inode) == -1) // chama a função de limpar o inode
	{
		printf("Ocorreu um erro ao limpar o inode\n");
		return -1;
	}
	free(fileDescriptor); // limpa o file descriptor desse arquivo

	return 0;
}

// Funcao para abertura de um diretorio, a partir do caminho
// especificado em path, no disco indicado por d, no modo Read/Write,
// criando o diretorio se nao existir. Retorna um descritor de arquivo,
// em caso de sucesso. Retorna -1, caso contrario.
int myFSOpenDir(Disk *d, const char *path)
{
	return -1;
}

// Funcao para a leitura de um diretorio, identificado por um descritor
// de arquivo existente. Os dados lidos correspondem a uma entrada de
// diretorio na posicao atual do cursor no diretorio. O nome da entrada
// e' copiado para filename, como uma string terminada em \0 (max 255+1).
// O numero do inode correspondente 'a entrada e' copiado para inumber.
// Retorna 1 se uma entrada foi lida, 0 se fim de diretorio ou -1 caso
// mal sucedido
int myFSReadDir(int fd, char *filename, unsigned int *inumber)
{
	return -1;
}

// Funcao para adicionar uma entrada a um diretorio, identificado por um
// descritor de arquivo existente. A nova entrada tera' o nome indicado
// por filename e apontara' para o numero de i-node indicado por inumber.
// Retorna 0 caso bem sucedido, ou -1 caso contrario.
int myFSLink(int fd, const char *filename, unsigned int inumber)
{
	return -1;
}

// Funcao para remover uma entrada existente em um diretorio,
// identificado por um descritor de arquivo existente. A entrada e'
// identificada pelo nome indicado em filename. Retorna 0 caso bem
// sucedido, ou -1 caso contrario.
int myFSUnlink(int fd, const char *filename)
{
	return -1;
}

// Funcao para fechar um diretorio, identificado por um descritor de
// arquivo existente. Retorna 0 caso bem sucedido, ou -1 caso contrario.
int myFSCloseDir(int fd)
{
	return -1;
}

// Funcao para instalar seu sistema de arquivos no S.O., registrando-o junto
// ao virtual FS (vfs). Retorna um identificador unico (slot), caso
// o sistema de arquivos tenha sido registrado com sucesso.
// Caso contrario, retorna -1
int installMyFS(void)
{
	FSInfo *fs_info = (FSInfo *)malloc(sizeof(FSInfo));
	fs_info->fsname = fsname;
	fs_info->fsid = fsid;
	fs_info->closeFn = myFSClose;
	fs_info->closedirFn = myFSCloseDir;
	fs_info->formatFn = myFSFormat;
	fs_info->isidleFn = myFSIsIdle;
	fs_info->linkFn = myFSLink;
	fs_info->openFn = myFSOpen;
	fs_info->opendirFn = myFSOpenDir;
	fs_info->readFn = myFSRead;
	fs_info->readdirFn = myFSReadDir;
	fs_info->unlinkFn = myFSUnlink;
	fs_info->writeFn = myFSWrite;
	myFSslot = vfsRegisterFS(fs_info); // identificador unico (slot) do file system
	return myFSslot;
}
