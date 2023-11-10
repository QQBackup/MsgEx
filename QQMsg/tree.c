/*
 * tree.c: Tree node operations
 *
 * QUQU 2006/12/26
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "tree.h"


int TNode_traverse(TNode *T)
{
	static int level=0;
	int i=0;

	level++;
	{
		printf("|");
		for(i=0; i<(level-1)*4; i++){
			printf(" ");
		}
		printf("|----");
	}

	if(T) 
	{ 
		TNode *child; 
		printf("%s (type=%d, len=%d)\n", T->name, T->type, T->len);
		for(child = T->firstchild; child != NULL; child = child->nextsibling) 
		{ 
			TNode_traverse(child); 
		} 
	}
	level--;
	return 0;
}


TNode *TNode_find(TNode *T, char *name)
{
	TNode *t, *n;

	if(!T)
		return NULL;

	if(strcmp(T->name, name) == 0)
		return T;

	for(t=T->firstchild; t; t=t->nextsibling){
		if(strcmp(t->name, name) == 0)
			return t;
		if((n=TNode_find(t, name)))
			return n;
	}

	return NULL;
}


int TNode_add(TNode *Root, TNode *node)
{
	TNode *t;

	if(!Root)
		return -1;

	//printf("Adding tree node: type=%d\n", node->type);

	if(!Root->firstchild){
		/* insert first child */
		Root->firstchild = node;
		node->nextsibling = NULL;
		node->parent = Root;
		return 0;
	}
	for(t=Root->firstchild; t; t=t->nextsibling){
		if(t->nextsibling == NULL)
			break;
	}

	t->nextsibling = node;
	node->nextsibling = NULL;
	node->parent = Root;
	return 0;
}

TNode * TNode_alloc()
{
		TNode *node;
		node = (TNode*)malloc(sizeof(TNode));
		TNODE_INIT((*node));

		return node;
}

/*
int main(int argc, char **argv)
{
	TNode tree;
	int i;

	TNODE_INIT(tree);

	for(i=0; i<10; i++){
		TNode *node;
		node = (TNode*)malloc(sizeof(TNode));
		TNODE_INIT((*node));
		node->type = 1;
		node->len = 100*i;
		node->data = (char*)malloc(node->len);
		TNode_add(&tree, node);
	}

	Traverse(tree.firstchild);

	return 0;
}
*/



