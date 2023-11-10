/*
 *
 * QQmsg_decode.c
 *
 * QUQU 2006/12/27
 *
 */


#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")

extern unsigned int Key4;
extern unsigned int Key3;
extern unsigned int Key2;
extern unsigned int Key1;

extern void Decrypt(unsigned int *ebx, unsigned int *edi);

/* input: 8 bytes long */
int blowfish_decrypt(char *pIn, char *key, char *pOut)
{
	unsigned int ebx, edi;

	Key4 = *(int *)key;
	Key3 = *(int *)(key+4);
	Key2 = *(int *)(key+8);
	Key1 = *(int *)(key+12);
	
	Key1 = ntohl(Key1);
	Key2 = ntohl(Key2);
	Key3 = ntohl(Key3);
	Key4 = ntohl(Key4);

	ebx = *(unsigned int *)(pIn+0);
	edi = *(unsigned int *)(pIn+4);
	
	ebx = ntohl(ebx);
	edi = ntohl(edi);

	Decrypt(&ebx, &edi);

	ebx = ntohl(ebx);
	edi = ntohl(edi);

	memcpy(pOut, &ebx, 4);
	memcpy(pOut+4, &edi, 4);
	//MsgEx_DumpHex(pOut, 8);

	return 0;
}


/* total_len: len of pOut, should be given at calling */
int QQMSG_decode(char *pIn, int len, char *key, char *pOut, int *total_len)
{
	int n;
	char *pEdi;	//原始指针，指向前8个字节
	char *pLocal;	//前驱指针，指向后8个字节
	int BLOCK_LEN=8;
	int count=0;
	int offset = 0;
	char *pbuffer;
	char buffer[8];
	char ebx_edi[8];
	char cl=0;
	int i=0;
	int retLen;
	
	if((len % 8) != 0)
		return -1;
	
	if(len < 0x10)
		return -1;
	
	n = len;
	pLocal = pIn;
	blowfish_decrypt(pIn, key, ebx_edi);
	cl = *ebx_edi;
	cl &= 0x7;
	n -= cl;
	n -= 0xa;
	if(*total_len < n)
		return -1;
	
	if(n == 0)
		return -1;

	*total_len = n;
	retLen = n;

	n=0;
	offset = BLOCK_LEN;
	
	memset(buffer, 0, 8);
	pEdi = pIn;
	pLocal += BLOCK_LEN;
	pbuffer = buffer;
	n = cl+1;
	count = 1;

	while(count <= 2)
	{
		if(n < BLOCK_LEN)
		{
			n++;
			count++;
			continue;
		}
		if(n != BLOCK_LEN)
			continue;
		
		pbuffer = pEdi;	//pIn
		pIn = pLocal;
		
		/* xor ebx_edi with bytes afterward */
		for(i=0; i < BLOCK_LEN; i++)
		{
			char *p;
			if((offset+i) >= len)
				return -1;
			p = pLocal+i;
			cl = *p;
			p = ebx_edi+i;
			(*p) ^= cl;
		}
		
		/* compute new ebx_edi */
		blowfish_decrypt(ebx_edi, key, ebx_edi);
		offset += BLOCK_LEN;
		pEdi = pLocal;
		pLocal += BLOCK_LEN;
		n = 0;
	};
	
	//end of while

	if(*total_len == 0)
		goto final_process;

	while(*total_len != 0)
	{
		char *p;
		if(n < BLOCK_LEN){
			p = pOut;
			cl = *(char*)(pbuffer+n);
			cl ^= *(char*)(ebx_edi+n);
			pOut++;
			n++;
			(*total_len)--;
			*p = cl;
			continue;
		}
		if(n != BLOCK_LEN)
			continue;
		pbuffer = pEdi;
		pIn = pLocal;
		
		i = 0;
		/* xor ebx_edi with bytes afterward */
		do{
			if((offset+i) >= len)
				return -1;
			cl = *(pLocal+i);
			*(ebx_edi+i) ^= cl;
			i++;
		}while(i<BLOCK_LEN);
		
		/* compute new ebx_edi */
		blowfish_decrypt(ebx_edi, key, ebx_edi);
		offset += BLOCK_LEN;
		pEdi = pLocal;
		pLocal += BLOCK_LEN;
		n = 0;
	}//end of while
	
final_process:
	count = 1;
	do{
		if(n < BLOCK_LEN){
			char *p;
			p = pOut;
			cl = *(char*)(pbuffer+n);
			cl ^= *(char*)(ebx_edi+n);
			if(cl != 0)
				return -1;
			n++;
			count++;
			continue;
		}

		if(n != BLOCK_LEN)
			continue;
		
		pbuffer = pEdi;
		pIn = pLocal;
		i = 0;
		do{
			if((offset+i) >= len)
				return -1;
			cl = *(pLocal+i);
			*(ebx_edi+i) ^= cl;
			i++;
			
		}while(i<BLOCK_LEN);

		/* compute new ebx_edi */
		blowfish_decrypt(ebx_edi, key, ebx_edi);
		offset += BLOCK_LEN;
		pEdi = pLocal;
		pLocal += BLOCK_LEN;
		n = 0;		
	}while(count < 7);
	
	*total_len = retLen;
	return 0;
	
}


