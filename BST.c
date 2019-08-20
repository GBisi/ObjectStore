#include <BST.h>

/*
DESCRIPTION: Cerca nel BST il valore x
PARAMETERS: root -> BST da utilizzare, x -> valore da cercare
RETURN VALUE: 
*/
static node* find_minimum(node *root)
{
    if(root == NULL)
        return NULL;
    else if(root->left_child != NULL) // node with minimum value will have no left child
        return find_minimum(root->left_child); // left most element will be minimum
    return root;
}

//OVERVIEW: The same of deleteNode but root isn't modified
static node* delete(node *root, int x)
{ 
    //searching for the item to be deleted
    if(root==NULL)
        return NULL;
    if (x>root->data)
        root->right_child = delete(root->right_child, x);
    else if(x<root->data)
        root->left_child = delete(root->left_child, x);
    else
    {
        //No Children
        if(root->left_child==NULL && root->right_child==NULL)
        {
            free(root);
            return NULL;
        }

        //One Child
        else if(root->left_child==NULL || root->right_child==NULL)
        {
            node *temp;
            if(root->left_child==NULL)
                temp = root->right_child;
            else
                temp = root->left_child;
            free(root);
            return temp;
        }

        //Two Children
        else
        {
            node *temp = find_minimum(root->right_child);
            root->data = temp->data;
            root->right_child = delete(root->right_child, temp->data);
        }
    }
    return root;
}


node* search(node *root, int x)
{
    if(root==NULL || root->data==x) //if root->data is x then the element is found
        return root;
    else if(x>root->data) // x is greater, so we will search the right subtree
        return search(root->right_child, x);
    else //x is smaller than the data, so we will search the left subtree
        return search(root->left_child,x);
}

int isIn(node* root, int x){ return (search(root,x) == NULL ? 0 : 1); }

node* new_node(int x)
{
    node *p;
    p = (node*)malloc(sizeof(node));
    if(p==NULL) return NULL;
    p->data = x;
    p->left_child = NULL;
    p->right_child = NULL;

    return p;
}

node* insertNode(node *root, int x)
{
    //searching for the place to insert
    if(root==NULL)
        return new_node(x);
    else if(x>root->data) // x is greater. Should be inserted to right
        root->right_child = insertNode(root->right_child, x);
    else // x is smaller should be inserted to left
        root->left_child = insertNode(root->left_child,x);
    return root;
}

node* deleteNode(node* root, int x)
{

	root = delete(root,x);
	return root;
}


void inorder(node *root)
{
    if(root!=NULL) // checking if the root is not null
    {
        inorder(root->left_child); // visiting left child
        printf("%d ", root->data); // printing data at root
        inorder(root->right_child);// visiting right child
    }

}

void deleteBst(node* root)
{

	while(root != NULL){
		root = delete(root,root->data);
	}

}
