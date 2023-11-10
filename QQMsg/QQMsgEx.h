#ifndef __QQMSGEX_H__
#define __QQMSGEX_H__


#define QQEX_MSGTYPE_C2CMSG		1
#define QQEX_MSGTYPE_SYSMSG		3
#define QQEX_MSGTYPE_IMINFO		4
#define QQEX_MSGTYPE_DISCMSG		5
#define QQEX_MSGTYPE_GROUPMSG		6
#define QQEX_MSGTYPE_MOBILEMSG		7
#define QQEX_MSGTYPE_TEMPSESSIONMSG	8


#define MAX_MSG_LEN 	512000	//500K


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

extern int msgex_decode(char *data, int size, char *out, char *key);
extern void MsgEx_DumpHex(char *hextodump, int size);
extern int QQMSG_decode(char *pIn, int len, char *key, char *pOut, int *total_len);

#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif

