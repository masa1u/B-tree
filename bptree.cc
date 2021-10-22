#include "bptree.h"
#include <vector>
#include <sys/time.h>

NODE *split(NODE *leaf, int key, DATA *data);

NODE *insert_in_parent(NODE *oldleaf, NODE *newleaf, int key, DATA *data);

struct timeval
cur_time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}

void print_tree_core(NODE *n)
{
	// printf("%d\n", Root->key[0]);
	printf("[");
	for (int i = 0; i < n->nkey; i++)
	{
		if (!n->isLeaf)
			print_tree_core(n->chi[i]);
		printf("%d", n->key[i]);

		if (i != n->nkey - 1 && n->isLeaf)
			putchar(' ');
	}
	if (!n->isLeaf)
		print_tree_core(n->chi[n->nkey]);
	printf("]");
}

void print_tree(NODE *node)
{
	print_tree_core(node);
	printf("\n");

	fflush(stdout);
}

NODE *
find_leaf(NODE *node, int key)
{
	int kid;

	if (node->isLeaf)
		return node;
	for (kid = 0; kid < node->nkey; kid++)
	{
		if (key < node->key[kid])
			break;
	}

	return find_leaf(node->chi[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
	int i;
	if (key < leaf->key[0])
	{
		for (i = leaf->nkey; i > 0; i--)
		{
			leaf->chi[i] = leaf->chi[i - 1];
			leaf->key[i] = leaf->key[i - 1];
		}
		leaf->key[0] = key;
		leaf->chi[0] = (NODE *)data;
	}
	else
	{
		for (i = 0; i < leaf->nkey; i++)
		{
			if (key < leaf->key[i])
				break;
		}
		for (int j = leaf->nkey; j > i; j--)
		{
			leaf->chi[j] = leaf->chi[j - 1];
			leaf->key[j] = leaf->key[j - 1];
		}

		/* CodeQuiz */
		leaf->key[i] = key;
		leaf->chi[i] = (NODE *)data;
	}

	leaf->nkey++;

	return leaf;
}

NODE *
insert_in_root(NODE *root, int key, NODE *oldleaf, NODE *newleaf)
{
	int i;
	if (key < root->key[0])
	{
		root->chi[root->nkey + 1] = root->chi[root->nkey];

		for (i = root->nkey; i > 0; i--)
		{
			root->chi[i] = root->chi[i - 1];
			root->key[i] = root->key[i - 1];
		}

		root->key[0] = key;
		root->chi[0] = oldleaf;
		root->chi[1] = newleaf;
	}
	else
	{

		for (i = 0; i < root->nkey; i++)
		{
			if (key < root->key[i])
				break;
		}
		root->chi[root->nkey + 1] = root->chi[root->nkey];
		for (int j = root->nkey; j > i; j--)
		{
			root->chi[j] = root->chi[j - 1];
			root->key[j] = root->key[j - 1];
		}

		root->key[i] = key;
		root->chi[i] = oldleaf;
		root->chi[i + 1] = newleaf;
	}

	root->nkey++;

	return root;
}

TEMP *
insert_in_leaf_temp(TEMP *leaf, int key, DATA *data)
{
	int i;
	if (key < leaf->key[0])
	{
		for (i = leaf->nkey; i > 0; i--)
		{
			leaf->chi[i] = leaf->chi[i - 1];
			leaf->key[i] = leaf->key[i - 1];
		}
		leaf->key[0] = key;
		leaf->chi[0] = (NODE *)data;
	}
	else
	{
		for (i = 0; i < leaf->nkey; i++)
		{
			if (key < leaf->key[i])
				break;
		}
		for (int j = leaf->nkey; j > i; j--)
		{
			leaf->chi[j] = leaf->chi[j - 1];
			leaf->key[j] = leaf->key[j - 1];
		}

		/* CodeQuiz */
		leaf->key[i] = key;
		leaf->chi[i] = (NODE *)data;
	}
	leaf->nkey++;

	return leaf;
}

NODE *
alloc_leaf(NODE *parent)
{
	NODE *node;
	if (!(node = (NODE *)calloc(1, sizeof(NODE))))
		ERR;
	node->isLeaf = true;
	node->parent = parent;
	node->nkey = 0;

	return node;
}

TEMP *
copy(NODE *node)
{
	TEMP *temp;
	if (!(temp = (TEMP *)calloc(1, sizeof(TEMP))))
		ERR;
	temp->isLeaf = true;
	temp->nkey = N - 1;
	for (int i = 0; i < N - 1; i++)
	{
		temp->key[i] = node->key[i];
		temp->chi[i] = node->chi[i];
	}
	return temp;
}

NODE *
split_root(NODE *leftroot, int newkey, DATA *data)
{
	TEMP *temp;
	//0 copy
	temp = copy(leftroot);
	//1 insert
	insert_in_leaf_temp(temp, newkey, data);

	//2,3  pointer change
	NODE *rightroot;
	rightroot = alloc_leaf(leftroot->parent);

	// leaf->chi[leaf->nkey] = (NODE *)data;
	rightroot->chi[N - 1] = (NODE *)data;

	leftroot->chi[N - 1] = rightroot;

	//4 clean up
	for (int i = 0; i < rightroot->nkey; i++)
	{
		leftroot->key[i] = 0;
		leftroot->chi[i] = NULL;
	}
	leftroot->nkey = 0;

	//5 copy to old leaf
	for (int i = 0; i < N / 2; i++)
	{
		leftroot->key[i] = temp->key[i];
		leftroot->chi[i] = temp->chi[i];
		leftroot->nkey++;
	}

	// 7
	int key = temp->key[int(N / 2)];

	//6 copy to newleaf
	for (int i = int(N / 2) + 1; i < temp->nkey; i++)
	{
		rightroot->key[i - int(N / 2) - 1] = temp->key[i];
		rightroot->chi[i - int(N / 2) - 1] = temp->chi[i];
		rightroot->nkey++;
	}
	// printf("%d %d\n", leftroot->key[0], rightroot->key[0]);

	//8 insert in parents

	insert_in_parent(leftroot, rightroot, key, data);

	return leftroot;
}

NODE *
insert_in_parent(NODE *oldleaf, NODE *newleaf, int key, DATA *data)
{
	NODE *subRoot;
	if (oldleaf->parent == NULL)
	{

		subRoot = alloc_leaf(NULL);
		Root = subRoot;

		subRoot->isLeaf = false;
	}

	else
	{
		subRoot = oldleaf->parent;
	}
	if (subRoot->nkey < (N - 1))
	{
		insert_in_root(subRoot, key, oldleaf, newleaf);
		newleaf->parent = subRoot;
		oldleaf->parent = subRoot;
	}
	else
	{
		// split_root(subRoot, key, data);
		newleaf->isLeaf = false;
		oldleaf->isLeaf = false;
	}
	return subRoot;
}

NODE *
split(NODE *leaf, int newkey, DATA *data)
{
	TEMP *temp;
	//0 copy
	temp = copy(leaf);
	//1 insert
	insert_in_leaf_temp(temp, newkey, data);

	//2,3  pointer change
	NODE *newleaf;
	newleaf = alloc_leaf(leaf->parent);

	// leaf->chi[leaf->nkey] = (NODE *)data;
	newleaf->chi[N - 1] = (NODE *)data;

	leaf->chi[N - 1] = newleaf;

	//4 clean up
	for (int i = 0; i < leaf->nkey; i++)
	{
		leaf->key[i] = 0;
		leaf->chi[i] = NULL;
	}
	leaf->nkey = 0;

	//5 copy to old leaf
	for (int i = 0; i < N / 2; i++)
	{
		leaf->key[i] = temp->key[i];
		leaf->chi[i] = temp->chi[i];
		leaf->nkey++;
	}

	//6 copy to newleaf
	for (int i = int(N / 2); i < temp->nkey; i++)
	{
		newleaf->key[i - int(N / 2)] = temp->key[i];
		newleaf->chi[i - int(N / 2)] = temp->chi[i];
		newleaf->nkey++;
	}

	//7
	int key = newleaf->key[0];

	//8 insert in parents

	insert_in_parent(leaf, newleaf, key, data);

	return leaf;
}

void insert(int key, DATA *data)
{
	NODE *leaf;

	if (Root == NULL)
	{
		leaf = alloc_leaf(NULL);
		Root = leaf;
	}
	else
	{
		leaf = find_leaf(Root, key);
	}
	if (leaf->nkey < (N - 1))
	{
		insert_in_leaf(leaf, key, data);
	}
	else
	{ // split
		// future work
		split(leaf, key, data);
	}
	// printf("%d", leaf->key[0]);
}

void init_root(void)
{
	Root = NULL;
}

int interactive()
{
	int key;

	std::cout << "Key: ";
	std::cin >> key;

	return key;
}

int main(int argc, char *argv[])
{
	struct timeval begin, end;

	init_root();

	printf("-----Insert-----\n");
	begin = cur_time();
	while (true)
	{
		insert(interactive(), NULL);
		print_tree(Root);
	}
	end = cur_time();

	return 0;
}
