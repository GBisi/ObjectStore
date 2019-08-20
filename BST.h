#ifndef BST_H
#define BST_H

#include <stdio.h>
#include <stdlib.h>

//original source: https://www.codesdope.com/blog/article/binary-search-tree-in-c/

typedef struct _node
{
    int data; //node will store an integer
    struct _node *right_child; // right child
    struct _node *left_child; // left child
} node;

/*
DESCRIPTION: Cerca nel BST il valore x
PARAMETERS: root -> BST da utilizzare, x -> valore da cercare
RETURN VALUE: un nodo che contiene il valore x, NULL altrimenti
*/
node* search(node *root, int x);

/*
DESCRIPTION: Cerca nel BST il valore x
PARAMETERS: root -> BST da utilizzare, x -> valore da cercare
RETURN VALUE: 1 valore presente, 0 altrimenti
*/
int isIn(node* root, int x);


/*
DESCRIPTION: Crea un nuovo nodo con valore x
PARAMETERS: x -> valore nodo
RETURN VALUE: il nodo creato, NULL in caso di errore
*/
node* new_node(int x);

/*
DESCRIPTION: Inserisce nel BST il valore x
PARAMETERS: root -> BST da utilizzare, x -> valore da inserire
RETURN VALUE: il nodo che contiene il valore x, NULL in caso di errore
*/
node* insertNode(node *root, int x);

/*
DESCRIPTION: Elimina nel BST il valore x. Modifica anche il parametro root.
PARAMETERS: root -> BST da utilizzare, x -> valore da eliminare
RETURN VALUE: la nuova root
*/
node* deleteNode(node* root, int x);

/*
DESCRIPTION: Stampa in ordine crescente i valori contenuti nel BST
PARAMETERS: root -> BST da utilizzare
*/
void inorder(node *root);

/*
DESCRIPTION: Elimina tutti i nodi dal BST
PARAMETERS: root -> BST da utilizzare
*/
void deleteBst(node* root);

#endif
