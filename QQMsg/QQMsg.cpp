// msg.cpp : Defines the entry point for the console application.
//

/*
 *
 * QQmsg.c: Dump QQ message
 *
 * QUQU<quhongjun@msn.com>
 * 2006/12/27
 *
 */


#include <AFXPRIV.H>
#include <stdio.h>
#include <windows.h>
#include <ole2.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "winsock2.h"

#include "tree.h"
#include "md5.h"
#include "QQMsgEx.h"

#pragma comment(lib, "ole32.lib" )
#pragma comment(lib, "ws2_32.lib")

TNode Tree, *TParent;
char pMsgExKey[16];
int iMsgType = 0;
FILE *fpout=NULL;

char *strtime(time_t *time)
{
	struct tm *tm;
	static char s[20];

	tm = localtime(time);
	if(tm == NULL){
		sprintf(s, "(TIME=0x%X)", *(int*)time);
		return s;
	}
	sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", 
		tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, 
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return s;

}


/*
struct RecordStruct{
	int time;
	unsigned char type;
	int nameLen;
};
*/
int MsgEx_ShowMsg(char *pOut, int retLen)
{
	char *pName, *pMsg, *pOffset;;
	int msgType=0, nameLen=0, msgLen=0;
	char *strTime;
	time_t t;
	int offset=0;



		//time
		offset = 0;
		pOffset = pOut;	//time
		t = *(int*)pOffset;
		strTime = strtime(&t);
		fprintf(fpout, "%s ", strTime);

		//msgtype
		offset += 4;	//bypass time
		pOffset = pOut+offset;
		msgType = *(char*)pOffset;

		switch(iMsgType)
		{
		case QQEX_MSGTYPE_C2CMSG:
		case QQEX_MSGTYPE_MOBILEMSG:
			offset += 1;	//bypass type
			break;
		case QQEX_MSGTYPE_SYSMSG:
		case QQEX_MSGTYPE_TEMPSESSIONMSG:
			offset += 4;
			break;
		case QQEX_MSGTYPE_GROUPMSG:
			{
			int from=*(int*)pOffset;
			int to=*(int*)(pOffset+4);
			offset += 8;
			break;
			}
		case QQEX_MSGTYPE_DISCMSG:
			offset += 1;	//bypass type=1
			offset += 4;	//bypass unknown
			offset += 8;	//bypass from, to
			break;
		case QQEX_MSGTYPE_IMINFO:
		default:
			offset += 1;	//bypass type
			break;
		}
		pOffset = pOut+offset;

		//nameLen
		nameLen = *(int*)pOffset;
		if(nameLen > (retLen-offset)){
			//printf("nameLen overflow 0x(%x), iMsgtype=%d\n", nameLen, iMsgType);
			return -1;
		}

		//name
		offset += 4;	//bypass nameLen
		pOffset = pOut+offset;
		if(offset > retLen){
			//printf("offset overflow\n");
			return -1;
		}
		pName = pOffset;

		//msgLen
		offset += nameLen;	//bypass name, now points to recordLen
		if(offset > retLen){
			//printf("offset overflow\n");
			return -1;
		}
		pOffset = pOut+offset;
		msgLen = *(int*)(pOffset);
		if(msgLen > (retLen-offset)){
			//printf("msgLen overflow (0x%x)\n", msgLen);
			return -1;
		}

		*(pName+nameLen) = 0x0;		//this will overwrite msgLen, so put it afterward
		fprintf(fpout, "%s\n", pName);

		//msg
		offset += 4;	//bypass msgLen, now points to msg
		pOffset = pOut + offset;
		if(offset >= retLen){
			//printf("offset overflow\n");
			return -1;
		}
		pMsg = pOffset;

		//throw font message
		{
			char *p = pMsg;
			while(*p && p<(pOut+retLen))
			{
				if(((*p)==0x13) && ((*(p+1))==0x15)){
					*p = 0x0;
					break;
				}
				p++;
			}
		}
		fprintf(fpout, "%s\n\n", pMsg);

	
	return 0;
}


int MsgEx_DecodeMsg(char *pData, char *pIndex, ULONG dSize, ULONG iSize)
{
	int nRecord=0;
	unsigned int Offset=0, lastOffset=0, nextOffset=0;
	unsigned int RecordLen=0;
	int i;
	char *pIn;
	char pOut[MAX_MSG_LEN];
	int *p;
	int retLen=0;
	int ret=0;

	memset(pOut, 0, MAX_MSG_LEN);

	nRecord = iSize/4;
	
//	printf("Decoding %d records ..\n", nRecord);
	lastOffset = 0;
	for(i=0; i<nRecord; i++){
		p = (int*)(pIndex + i*4);
		Offset = *(int *)p;
		if(i == (nRecord-1))
			nextOffset = dSize;
		else
			nextOffset = *(int*)(p+1);
//		printf("\tRecord[%d]: Offset=%d\n", i, Offset);
		RecordLen = nextOffset - Offset;
		pIn = pData + Offset;
		retLen = RecordLen;
		memset(pOut, 0, sizeof(pOut));
		lastOffset = Offset;
		ret = QQMSG_decode(pIn, RecordLen, pMsgExKey, pOut, &retLen);
		if(ret < 0){
			printf("Decoding message #%d failed!\n", i);
			continue;
		}

//		printf("%d bytes decoded\n", retLen);
//		MsgEx_DumpHex(pOut, retLen);
		MsgEx_ShowMsg(pOut, retLen);
	}

	return 0;

}


int MsgEx_SetMsgType(char *name)
{

	if(strcmp(name, "C2CMsg") == 0){
		iMsgType = QQEX_MSGTYPE_C2CMSG;
		fprintf(fpout, "\n\n==================================================\n");
		fprintf(fpout, "消息类型：用户消息\n"); 
		fprintf(fpout, "==================================================\n\n");
		return 0;
	}
	if(strcmp(name, "SysMsg") == 0){
		iMsgType = QQEX_MSGTYPE_SYSMSG;
		fprintf(fpout, "\n\n==================================================\n");
		fprintf(fpout, "消息类型：系统消息\n"); 
		fprintf(fpout, "==================================================\n\n");
		return 0;
	}
	if(strcmp(name, "IMInfo") == 0){
		iMsgType = QQEX_MSGTYPE_IMINFO;
		fprintf(fpout, "\n\n==================================================\n");
		fprintf(fpout, "消息类型：IMInfo\n"); 
		fprintf(fpout, "==================================================\n\n");
		return 0;
	}
	if(strcmp(name, "DiscMsg") == 0){
		iMsgType = QQEX_MSGTYPE_DISCMSG;
		fprintf(fpout, "\n\n==================================================\n");
		fprintf(fpout, "消息类型：DiscMsg\n"); 
		fprintf(fpout, "==================================================\n\n");
		return 0;
	}
	if(strcmp(name, "GroupMsg") == 0){
		iMsgType = QQEX_MSGTYPE_GROUPMSG;
		fprintf(fpout, "\n\n==================================================\n");
		fprintf(fpout, "消息类型：群组消息\n"); 
		fprintf(fpout, "==================================================\n\n");
		return 0;
	}
	if(strcmp(name, "MobileMsg") == 0){
		iMsgType = QQEX_MSGTYPE_MOBILEMSG;
		fprintf(fpout, "\n\n==================================================\n");
		fprintf(fpout, "消息类型：移动消息\n"); 
		fprintf(fpout, "==================================================\n\n");
		return 0;
	}
	if(strcmp(name, "TempSessionMsg") == 0){
		iMsgType = QQEX_MSGTYPE_TEMPSESSIONMSG;
		fprintf(fpout, "\n\n==================================================\n");
		fprintf(fpout, "消息类型：临时会话\n"); 
		fprintf(fpout, "==================================================\n\n");
		return 0;
	}

	return 0;
}



int MsgEx_DumpTnodeMsg(TNode *T)
{
	TNode *tData, *tIndex;

	if(!T)
		return -1;

	tData = TNode_find(T, "Data.msj");
	if(!tData)
		return -1;
	tIndex = TNode_find(T, "Index.msj");
	if(!tIndex)
		return -1;

	fprintf(fpout, "--------------------------------------------------\n");
	fprintf(fpout, "消息对象：%s\n", T->name);
	fprintf(fpout, "--------------------------------------------------\n");
	MsgEx_DecodeMsg(tData->data, tIndex->data, tData->len, tIndex->len);
	return 0;
}


int MsgEx_DumpMsg(TNode *T)
{
	TNode *t;

	if(!T)
		return -1;


	if(!T->firstchild)
		return -1;

	MsgEx_SetMsgType(T->name);
	for(t=T->firstchild; t; t=t->nextsibling){
		if(t->type == STGTY_STORAGE){
				MsgEx_DumpMsg(t);
		}else{
			MsgEx_DumpTnodeMsg(T);
			return 0;
		}
	}

	return 0;
}




/*
Matrix.db:
struct MatrixDbRecord{
	char RecordType;
	short nameLen;
	char name[...];
	int RecordLen;
	char Record[...];
};
*/


char * MsgEx_FindMatrixDB(char *pMatrix, int Len, char *name, int *retLen)
{
	char *pLocal;
	unsigned char RecordType;
	unsigned short nameLen;
	char *RecordName;
	unsigned int RecordLen=0;
	char *RecordData=NULL;
	int count = 0;

	pLocal = pMatrix+6;
	count += 6;

	while(count < Len)
	{
		RecordType = *pLocal;
		nameLen = *(short*)(pLocal+1);
		RecordName = pLocal+3;
		RecordLen = *(int*)(pLocal+3+nameLen);
		RecordData = pLocal+3+nameLen+4;
		if(strncmp(RecordName, name, strlen(name)) == 0){
			*retLen = RecordLen;
			return RecordData;
		}
		pLocal += (3+nameLen+4+RecordLen);
		count += (3+nameLen+4+RecordLen);
	}

	*retLen = 0;
	return NULL;
}


int MsgEx_DecodeMatrixDB(char *pIn, int Len)
{
/*
入口：
eax: pMatrixDB
edx: Len

本地变量：
[ebp-4]: char *pIn
[ebp-8]: int *pLen
[ebp-C]: ret
[ebp-10]: i
[ebp-14]: int n;
[ebp-18]: int count=Len
[ebp-1C]: char *pLocal = pIn+6
[ebp-1D]: char charD = rType;
[ebp-1E]: char charE;
[ebp-20]: short nLen
*/
	char *pLocal;
	int i;
	int count = Len;
	char rType;
	char charE;
	short nLen;
	int rLen;

	if(Len > 0x7FFF)
		return -1;
	if(*pIn != 'Q')
		return -1;
	if(*(pIn+1) != 'D')
		return -1;

	pLocal = pIn+6;
	count = Len-6;
	
	while(count > 7)
	{
		short ax;
		char al;
		
		rType = *pLocal;		//0x2
		nLen = (short)*(pLocal+1);	//0x0003
		ax = nLen;
		ax >>= 8;
		al = (char)ax;		//0x0
		al ^= (char)nLen;	//0x0^0x3=3
		charE = al;		//0x3
		pLocal += 3;
		count -= 3;
	
		for(i=0; i<nLen; i++)	//EBP20=0x3
		{
			char *p = pLocal; //0xaf
			char al;
			al = *p;
			al ^= charE;	//0xaf^0x3=0xAC
			al = ~al;	//not 0xac = 0x53
			*p = al;
			pLocal++;
			count--;
		}
		if(rType > 7)
			return -1;
		
		rLen = *(int*)pLocal;	//0x0001
		pLocal += 4;
		count -= 4;
		
		if((rType == 6) || (rType == 7))
		{
			short ax;
			
			ax = (short)rLen;
			ax >>= 8;
			al = (char)ax;
			al ^= (byte)rLen;
			charE = al;
			
			for(i=0; i<rLen; i++){
				char *p = pLocal;
				char al;
				al = (char)*p;
				al ^= charE;
				al = ~al;
				*p = al;
				pLocal++;
				count--;		
			}
			
			continue;
		}
		
		pLocal += rLen;
		continue;
	} //end of while
	
	return 0;
}


int MsgEx_OpenSubStream(IStorage *psubStg, LPOLESTR pwcsName, ULONG len, char *pOut)
{
	ULONG nsize=0;
	HRESULT hr;
	IStream *pStm = NULL;
	char *buf=NULL;
	
	if(pOut == NULL)
		return -1;
	
//	wprintf(L"Reading: %s\n", pwcsName);

	hr = psubStg->OpenStream(
		pwcsName, 
		NULL,
		STGM_READ | STGM_SHARE_EXCLUSIVE,
		0,
		&pStm
	);
	
	if(hr != S_OK){
		wprintf(L"Failed to open stream %s, errno:0x%X\n", pwcsName, hr);
		goto out;
	}
	
	hr = pStm->Read(pOut, len, &nsize);
	if(hr != S_OK){
		wprintf(L"Failed to read stream %s, errno:0x%X\n", pwcsName, hr);
		goto out;
	}

//	printf("%lu bytes read\n", nsize);
	//MsgEx_DumpHex(pOut, nsize);
	

out:	
	if( pStm )	pStm->Release();

	return nsize;
}



/* return: &psubStg */
int MsgEx_OpenSubStorage(IStorage *pStg, LPOLESTR pwcsName, IStorage **psubStg) 
{
	HRESULT hr;

		hr = pStg->OpenStorage(
			pwcsName, 
			NULL,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			0,
			0,
			psubStg);

	if(hr != S_OK){
		wprintf(L"Failed to open %s, errno:0x%X\n", pwcsName, hr);
		return -1;
	}
	
	return 0;
}



int MsgEx_EnumStorage(IStorage *pStg) 
{
	HRESULT hr;
	IStorage *psubStg = NULL;
	IEnumSTATSTG *pEnum=NULL;
	STATSTG statstg;
	ULONG nSize, maxSize=0;
	char *pOut=NULL;
	TNode *t=NULL;
	
	hr = pStg->EnumElements( 0, NULL, 0, &pEnum );
	ASSERT( SUCCEEDED(hr) );

	while( NOERROR == pEnum->Next( 1, &statstg, NULL) )
	{
		int iSize;
		char* pszMultiByte;

		t = TNode_alloc();
		TNODE_INIT((*t));
		TNode_add(TParent, t);

		//convert wide char to multibyte
		iSize = WideCharToMultiByte(CP_ACP, 0, statstg.pwcsName, -1, NULL, 0, NULL, NULL);
		pszMultiByte = (char*)malloc((iSize+1)/**sizeof(char)*/);
		WideCharToMultiByte(CP_ACP, 0, statstg.pwcsName, -1, pszMultiByte, iSize, NULL, NULL);


		nSize = statstg.cbSize.QuadPart;
		t->len = nSize;
		t->type = statstg.type;
		sprintf(t->name, "%s", pszMultiByte);
		if(t->len > 0){
			t->data = (char*)malloc(t->len);
		}


		if(statstg.type == STGTY_STORAGE){
			if(MsgEx_OpenSubStorage(pStg, statstg.pwcsName, &psubStg)>=0){
				TParent = t;
				MsgEx_EnumStorage(psubStg);
				TParent = t->parent;
				if(psubStg) {psubStg->Release(); psubStg=NULL;}
			}
		}else
		if(statstg.type == STGTY_STREAM){
			MsgEx_OpenSubStream(pStg, statstg.pwcsName, t->len, t->data);
		}
		CoTaskMemFree( statstg.pwcsName );
	}


	if( pEnum )	pEnum->Release();
	if( psubStg )	psubStg->Release();
		
	return 0;

}



int MsgEx_OpenStorageFile(char *fname) 
{	
	LPCTSTR lpFileName = _T( fname );
	IStorage *pStg = NULL;
	USES_CONVERSION;
	LPCOLESTR lpwFileName = T2COLE( lpFileName );	// 转换T类型为宽字符
	HRESULT hr;

	hr = StgIsStorageFile( lpwFileName );	// 是复合文件吗？
	if( FAILED(hr) ){
		printf("Invalid QQ message file: %s\n", fname);
		return -1;
	}

	hr = StgOpenStorage(			// 打开复合文件
		lpwFileName,			// 文件名称
		NULL,
		STGM_READ | STGM_SHARE_DENY_WRITE,
		0,
		0,
		&pStg);				// 得到根存储接口指针

	if( FAILED(hr) ){
		printf("Failed to open file %s, errno: 0x%x\n", fname, hr);
		return -1;
	}

	MsgEx_EnumStorage(pStg);


	if( pStg )	pStg->Release();

	return 0;

}




char *Find_QQPath_Number(char *path, char *retPath, char *pUid)
{
	char *p;
	int pos1=-1, pos2=-1;
	int offset=0;

	offset = strlen(path)-1;

	/* D:\tencent\1234567\ */
	p = path+offset;
	if(isdigit(*p))
	{
		sprintf(retPath, "%s\\MsgEx.db", path);
	}else
	if((*p) == '\\')
	{
		sprintf(retPath, "%sMsgEx.db", path);
	}else
	{
		sprintf(retPath, "%s", path);
	}


	while(offset > 0){
		p = path+offset;
		if(isdigit(*p)){
			pos1 = offset;
			break;
		}
		offset--;
	}

	if(pos1 < 0){
		return NULL;
	}
	offset = pos1;
	while(offset >= 0){
		p = path+offset;
		if(!isdigit(*p))
			break;
		pos2 = offset;
		offset--;
	}

	if(pos2 < 0){
		return NULL;
	}

	p = path+pos2;
	strncpy(pUid, p, pos1-pos2+1);
	return pUid;
}


char *Find_QQNumber(char *path, char *pUid)
{
	char *p;
	int pos1=-1, pos2=-1;
	char buf[20];
	int n=0;
	int offset=0;

	memset(buf, 0, sizeof(buf));
	offset = strlen(path)-1;
	while(offset > 0){
		p = path+offset;
		if(*p == '\\'){
			pos1 = offset;
			break;
		}
		offset--;
	}

	if(pos1 < 0){
		return NULL;
	}
	offset = pos1-1;
	while(offset >= 0){
		p = path+offset;
		if(!isdigit(*p))
			break;
		buf[n] = *p;
		n++;
		pos2 = offset;
		offset--;
	}

	if(pos2 < 0){
		return NULL;
	}

	p = path+pos2;
	strncpy(pUid, p, pos1-pos2);
	return pUid;
}


/* QQmsg <MsgEx.db> [uid] [output] */
int main(int argc, char* argv[])
{
	TNode *node;
	char *pUid = NULL;
	char *pMatrix;
	unsigned int pMatrixLen;
	char UidMd5[16];
	char *pCRK;
	int pCRKLen;
	int pLen=0;
	char *pUser=NULL;
	char strUid[20], strPath[512];
	time_t t;
	char fname[40];

//	printf("Hello World!\n");
	if(argc < 2){
		printf("Usage: %s <MsgEx.db> [QQ-Number]\n", argv[0]);
		return -1;
	}

	if(argc >= 3)
	{
		strcpy(strUid, argv[2]);
	}else
	{
		/* try to get QQ number from path */
		memset(strUid, 0, sizeof(strUid));
		memset(strPath, 0, sizeof(strPath));

		if(Find_QQPath_Number(argv[1], strPath, strUid) == NULL){
			printf("Please tell me the QQ number\n");
			return -1;
		}
	}
	pUid = strUid;


	if(argc >= 4)
		pUser = argv[3];


	TNODE_INIT(Tree);
	Tree.type = 1;
	sprintf(Tree.name, "MsgEx.db");
	TParent = &Tree;

	if(MsgEx_OpenStorageFile(strPath) < 0)
		return -1;

//	TNode_traverse(&Tree);

	node = TNode_find(&Tree, "Matrix.db");
	if(!node){
		printf("Failed to open Matrix.db!\n");
		return -1;
	}
	pMatrix = node->data;
	pMatrixLen = node->len;
	MsgEx_DecodeMatrixDB(pMatrix, pMatrixLen);
	//MsgEx_DumpHex(pMatrix, pMatrixLen);

	pCRK = MsgEx_FindMatrixDB(pMatrix, pMatrixLen, "CRK", &pCRKLen);
	//printf("CRKLen=%d\n", pCRKLen);
	//MsgEx_DumpHex(pCRK, pCRKLen);


	MD5(pUid, strlen(pUid), UidMd5);
	//printf("UidMd5:\n");
	//MsgEx_DumpHex(UidMd5, 16);

	pLen = 16;
	QQMSG_decode(pCRK, pCRKLen, UidMd5, pMsgExKey, &pLen);
	//msgex_decode(pCRK, pCRKLen, pMsgExKey, UidMd5);
//	printf("pMsgExKey:\n");
//	MsgEx_DumpHex(pMsgExKey, 16);

/*
	//show QQ message from certain user
	node = TNode_find(&Tree, pUser);
	if(node)
		MsgEx_DumpMsg(node);
	else
		printf("Tnode not found\n");
*/

	sprintf(fname, "%s.txt", pUid);
	fpout = fopen(fname, "w");
	if(fpout == NULL){
		printf("Can't open file for writing\n");
		fpout = stdout;
	}
	printf("处理中，请稍候...\n");
	fprintf(fpout, "\n");
	fprintf(fpout, "==================================================\n");
	fprintf(fpout, "本文件由QQMsg生成\n");
	fprintf(fpout, "友情提醒：查看别人的聊天记录属于不道德行为！\n");
	fprintf(fpout, "本人不对此软件产生的任何后果负责！\n");
	fprintf(fpout, "\n");
	fprintf(fpout, "QUQU<quhongjun@msn.com>\n");
	fprintf(fpout, "2006/12/28\n");
	fprintf(fpout, "==================================================\n");
	fprintf(fpout, "\n\n");
	
	t = time(0);
	fprintf(fpout, "用户：%s\n\n", pUid); 
	fprintf(fpout, "生成时间：%s\n", strtime(&t));

	MsgEx_DumpMsg(&Tree);

	printf("聊天记录已保存至: %s\n", fname);
	fclose(fpout);
	return 0;
}

