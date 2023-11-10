/*
 *  BlowFish.c: do BlowFish Encrypt/Decrypt
 *  来源于看雪论坛
 *  作者：blowfish
 */
  
  
#define F_FUNCTION(x, k1, k2, c)  ((((x) >> 5) + (k1)) ^ (((x) << 4) + (k2)) ^ ((c) +(x)))
#define C_BLOWFISH	16

unsigned int Key4 = 0;
unsigned int Key3 = 0;
unsigned int Key2 = 0;
unsigned int Key1 = 0;


//软件里面的解密运算过程
//注意：编译器会将有符号数的右移翻译成SAR指令，而我们需要的是SHR指令，所以必须定义成无符号数
void Decrypt(unsigned int *ebx, unsigned int *edi)
{

	int C1 = 0xE3779B90;
	int k = 0;

	for(k = 0 ; k < C_BLOWFISH; k++)
	{
		*edi -= F_FUNCTION((*ebx), Key1, Key2, C1);
		*ebx -= F_FUNCTION((*edi), Key3, Key4, C1);
		C1  += 0x61C88647;
	}
}

//上面的函数的逆
void Encrypt(unsigned int *edx, unsigned int *ebx)
{
	int C1 = 0x00000000;
	int k=0;
	for(k = 0 ; k < C_BLOWFISH; k++)
	{
		C1  -= 0x61C88647;
		(*edx) += F_FUNCTION((*ebx), Key3, Key4, C1);
		*ebx += F_FUNCTION((*edx), Key1, Key2, C1);    
	}
}




