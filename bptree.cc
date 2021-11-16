#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include <random>

#define DATA_SIZE 100

struct list
{
	int data;
	struct list *next;
};

NODE *split(NODE *leaf, int key, DATA *data);

NODE *insert_in_parent(NODE *oldleaf, NODE *newleaf, int key);

struct timeval
cur_time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}

void print_tree_core(NODE *n)
{
	printf("[");
	fflush(stdout);
	for (int i = 0; i < n->nkey; i++)
	{

		if (!n->isLeaf)
			print_tree_core(n->chi[i]);
		printf("%d", n->key[i]);
		fflush(stdout);

		if (i != n->nkey - 1 && n->isLeaf)
		{
			putchar(' ');
			fflush(stdout);
		}
	}
	if (!n->isLeaf)
		print_tree_core(n->chi[n->nkey]);
	printf("]");
	fflush(stdout);
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
insert_in_root_temp(TEMP *root, NODE *leftnode, NODE *rightnode, int key)
{
	int i;
	// printf("%d", root->nkey);

	if (key < root->key[0])
	{
		root->chi[N] = root->chi[N - 1];

		for (i = root->nkey; i > 0; i--)
		{
			root->chi[i] = root->chi[i - 1];
			root->key[i] = root->key[i - 1];
		}

		root->key[0] = key;
		root->chi[0] = leftnode;
		root->chi[1] = rightnode;
	}
	else
	{
		for (i = 0; i < root->nkey; i++)
		{
			if (key < root->key[i])
				break;
		}
		root->chi[N] = root->chi[N - 1];
		for (int j = root->nkey; j > i; j--)
		{
			root->chi[j] = root->chi[j - 1];
			root->key[j] = root->key[j - 1];
		}
		root->key[i] = key;
		root->chi[i] = leftnode;
		root->chi[i + 1] = rightnode;
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
	temp->chi[N - 1] = node->chi[N - 1];
	return temp;
}

NODE *
split_root(NODE *leftnode, NODE *rightnode, NODE *upperleftnode, int newkey)
{
	TEMP *temp;
	//0 copy

	temp = copy(upperleftnode);
	temp->isLeaf = false;
	//1 insert
	insert_in_root_temp(temp, leftnode, rightnode, newkey);

	//2,3  pointer change
	NODE *upperrightnode;
	upperrightnode = alloc_leaf(upperleftnode->parent);

	upperrightnode->isLeaf = false;

	// leftnode->chi[leftnode->nkey] = (NODE *)data;
	// rightnode->chi[N - 1] = (NODE *)data;

	// leftnode->chi[N - 1] = rightnode;

	//4 clean up
	for (int i = 0; i < N - 1; i++)
	{
		upperleftnode->key[i] = 0;
		upperleftnode->chi[i] = NULL;
	}
	upperleftnode->chi[N - 1] = NULL;
	upperleftnode->nkey = 0;

	//5 copy to old leaf
	for (int i = 0; i < N / 2; i++)
	{
		upperleftnode->key[i] = temp->key[i];
		upperleftnode->chi[i] = temp->chi[i];
		upperleftnode->nkey++;
	}
	upperleftnode->chi[N / 2] = temp->chi[N / 2];

	// 7
	int key = temp->key[int(N / 2)];

	//6 copy to newleaf
	for (int i = int(N / 2) + 1; i < temp->nkey; i++)
	{
		upperrightnode->chi[i - int(N / 2) - 1] = temp->chi[i];
		upperrightnode->key[i - int(N / 2) - 1] = temp->key[i];
		upperrightnode->nkey++;
	}
	upperrightnode->chi[upperrightnode->nkey] = temp->chi[temp->nkey];

	//8 insert in parents

	insert_in_parent(upperleftnode, upperrightnode, key);

	//parent change
	for (int i = 0; i <= upperrightnode->nkey; i++)
	{
		upperrightnode->chi[i]->parent = upperrightnode;
	}

	return leftnode;
}

NODE *
insert_in_parent(NODE *leftnode, NODE *rightnode, int key)
{
	NODE *uppernode;
	if (leftnode->parent == NULL)
	{
		uppernode = alloc_leaf(NULL);
		Root = uppernode;

		uppernode->isLeaf = false;
	}

	else
	{
		uppernode = leftnode->parent;
	}
	rightnode->parent = uppernode;
	leftnode->parent = uppernode;
	if (uppernode->nkey < (N - 1))
	{
		insert_in_root(uppernode, key, leftnode, rightnode);
	}
	else
	{
		split_root(leftnode, rightnode, uppernode, key);
	}
	return uppernode;
}

NODE *
split(NODE *leftleaf, int newkey, DATA *data)
{
	TEMP *temp;
	//0 copy
	temp = copy(leftleaf);
	//1 insert
	insert_in_leaf_temp(temp, newkey, data);

	//2,3  pointer change
	NODE *rightleaf;
	rightleaf = alloc_leaf(leftleaf->parent);

	// leftleaf->chi[leftleaf->nkey] = (NODE *)data;
	// rightleaf->chi[N - 1] = leftleaf->chi[N - 1];
	rightleaf->chi[N - 1] = (NODE *)data;

	leftleaf->chi[N - 1] = rightleaf;

	//4 clean up
	for (int i = 0; i < N - 1; i++)
	{
		leftleaf->key[i] = 0;
		leftleaf->chi[i] = NULL;
	}
	leftleaf->nkey = 0;

	//5 copy to old leaf
	for (int i = 0; i < N / 2; i++)
	{
		leftleaf->key[i] = temp->key[i];
		leftleaf->chi[i] = temp->chi[i];
		leftleaf->nkey++;
	}

	//6 copy to newleaf
	for (int i = int(N / 2); i < temp->nkey; i++)
	{
		rightleaf->key[i - int(N / 2)] = temp->key[i];
		rightleaf->chi[i - int(N / 2)] = temp->chi[i];
		rightleaf->nkey++;
	}

	//7
	int insertion_key = rightleaf->key[0];

	//8 insert in parents

	insert_in_parent(leftleaf, rightleaf, insertion_key);

	return leftleaf;
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

struct list *ascending()
{
	struct list *temp_list, *start;
	for (int i = 0; i < DATA_SIZE; i++)
	{

		if (i == 0)
		{
			temp_list = (list *)malloc(sizeof(struct list));
			start = temp_list;
		}
		else
		{
			temp_list->next = (list *)malloc(sizeof(struct list));
			temp_list = temp_list->next;
		}

		temp_list->data = i + 1;
	}
	temp_list->next = NULL;
	return start;
}

struct list *descending()
{
	struct list *temp_list, *start;
	for (int i = 0; i < DATA_SIZE; i++)
	{

		if (i == 0)
		{
			temp_list = (list *)malloc(sizeof(struct list));
			start = temp_list;
		}
		else
		{
			temp_list->next = (list *)malloc(sizeof(struct list));
			temp_list = temp_list->next;
		}

		temp_list->data = DATA_SIZE - i;
	}
	temp_list->next = NULL;
	return start;
}
struct list *random_gene()
{
	struct list *temp_list, *start;
	std::random_device rnd; // 非決定的な乱数生成器
	std::mt19937 mt(rnd()); // メルセンヌ・ツイスタの32ビット版、引数は初期シード
	std::uniform_int_distribution<> randDATA_SIZE(0, DATA_SIZE * 10);
	for (int i = 0; i < DATA_SIZE; i++)
	{

		if (i == 0)
		{
			temp_list = (list *)malloc(sizeof(struct list));
			start = temp_list;
		}
		else
		{
			temp_list->next = (list *)malloc(sizeof(struct list));
			temp_list = temp_list->next;
		}

		temp_list->data = randDATA_SIZE(mt);
	}
	temp_list->next = NULL;
	return start;
}

int main(int argc, char *argv[])
{
	struct timeval begin, end;
	struct list *top;

	init_root();

	// printf("-----Insert-----\n");
	begin = cur_time();
	// while (true)
	// {
	// 	insert(interactive(), NULL);
	// 	print_tree(Root);
	// }

	//前から
	// top = ascending();

	//後ろから
	// top = descending();

	//ランダム
	top = random_gene();

	//insert
	while (top)
	{
		insert(top->data, NULL);
		top = top->next;
	}

	// print_tree(Root);
	printf("build done\n");

	//search

	NODE *leaf;

	while (top)
	{
		leaf = find_leaf(Root, top->data);
		for (int i = 0; i < leaf->nkey; i++)
		{
			if (top->data == leaf->key[i])
			{
				// printf("found %d\n", top->data);
				break;
			}
			else if (i == leaf->nkey)
			{
				printf("not found %d\n", top->data);
			}
		}
	}

	printf("search done\n");

	end = cur_time();

	return 0;
}
