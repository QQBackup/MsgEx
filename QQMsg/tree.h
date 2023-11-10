#ifndef __QQMSG_TREE_H__
#define __QQMSG_TREE_H__

typedef struct TNode{
	int type;
	char name[16];
	int len;
	char *data;
	struct TNode *firstchild, *nextsibling, *parent;
}TNode;


#define TNODE_INIT(x) do{\
	(x).type=0;\
	(x).len=0;\
	(x).data=NULL;\
	(x).firstchild=NULL;\
	(x).nextsibling=NULL;\
	(x).parent=NULL;\
	memset((x).name, 0, sizeof((x).name));\
}while(0)


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

extern int TNode_traverse(TNode *T);
extern TNode *TNode_find(TNode *T, char *name);
extern int TNode_add(TNode *Root, TNode *node);
extern TNode * TNode_alloc();

#ifdef __cplusplus
}
#endif  /* __cplusplus */




#endif

