#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <list>
#include <set>
#include <vector>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <random>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <sys/time.h>
#include <atomic>
#include <limits.h>

using namespace std;

struct Node
{
	long long int key;
	atomic<int> status;  	// 1- transit, 2 - marked, 3 - added
	atomic<Node*> next;
};

struct NodeList
{
	Node listhead;
	atomic<bool> marked;
	pthread_mutex_t lock;
	atomic<NodeList*> next;
};

NodeList *vhead, *vtail;

int cycle_detect(long long int u, long long int v)		//is there a path from u to v?
{
	set<pair<long long int, bool>> reach;
	typedef set<pair<long long int, bool>>::iterator iterator_type;
	long long int key;

	NodeList *temp = vhead->next;
	while(temp->listhead.key < v)
		temp = temp->next.load(std::memory_order_seq_cst);

	if(temp->listhead.key != v || temp->marked.load(std::memory_order_seq_cst) == true)
		return false;
	
	Node *adj;
	reach.insert(make_pair(u, false));
	iterator_type i;
	while(true)
	{
		//find an element whose visited is set to false
		for(i=reach.begin(); i!=reach.end(); i++)
			if(i->second == false)
				break;

		if(i == reach.end())
			break;

		key = i->first;

		temp = vhead->next;
		while(temp->listhead.key < key)
			temp = temp->next.load(std::memory_order_seq_cst);

		if(temp->listhead.key != key || temp->marked.load(std::memory_order_seq_cst) == true)
		{
			for(iterator_type i=reach.begin(); i!=reach.end(); i++)
				if(i->first == key)
				{
					reach.erase(i);
					break;
				}
			continue;
		}

		adj = temp->listhead.next.load(std::memory_order_seq_cst);

		while(adj != NULL)
		{
			if(adj->status.load(std::memory_order_seq_cst) != 2 && reach.find(make_pair(adj->key, false)) == reach.end() && reach.find(make_pair(adj->key, true)) == reach.end())
				reach.insert(make_pair(adj->key, false));
			adj = adj->next;
		}

		if(reach.find(make_pair(v, false)) != reach.end())
			return true;

		for(iterator_type i=reach.begin(); i!=reach.end(); i++)
			if(i->first == key)
			{
				reach.erase(i);
				break;
			}

		reach.insert(make_pair(key, true));		//mark as visited
	}	
	return false;		//no cycle
}

void print_graph()
{
	NodeList *temp1 = vhead;
	Node *temp2;

	while(temp1 != NULL)
	{
		cout << temp1->listhead.key << " ";
		temp2 = temp1->listhead.next.load(std::memory_order_seq_cst);
		while(temp2 != NULL)
		{
			cout << temp2->key << " ";
			temp2 = temp2->next.load(std::memory_order_seq_cst);
		}
		cout << endl;
		temp1 = temp1->next.load(std::memory_order_seq_cst);
	}
}

void create_initial_vertices(int initial_vertices)
{
	int i, j;

	Node *ehead1 = (Node*) malloc(sizeof(Node));

	ehead1->key = LLONG_MIN;
	ehead1->next.store(NULL, std::memory_order_seq_cst);
	ehead1->status.store(3, std::memory_order_seq_cst);

	Node *etail1 = (Node*) malloc(sizeof(Node));

	etail1->key = LLONG_MAX;
	etail1->next.store(NULL, std::memory_order_seq_cst);
	etail1->status.store(3, std::memory_order_seq_cst);

	ehead1->next.store(etail1, std::memory_order_seq_cst);

	Node *ehead2 = (Node*) malloc(sizeof(Node));

	ehead2->key = LLONG_MIN;
	ehead2->next.store(NULL, std::memory_order_seq_cst);
	ehead2->status.store(3);

	Node *etail2 = (Node*) malloc(sizeof(Node));

	etail2->key = LLONG_MAX;
	etail2->next.store(NULL, std::memory_order_seq_cst);
	etail2->status.store(3, std::memory_order_seq_cst);

	ehead2->next.store(etail2, std::memory_order_seq_cst);

	vhead = (NodeList*) malloc(sizeof(NodeList));

	vhead->listhead.key = LLONG_MIN;
	vhead->listhead.next.store(ehead1, std::memory_order_seq_cst);
	vhead->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&vhead->lock, NULL);
	vhead->next.store(NULL, std::memory_order_seq_cst);

	vtail = (NodeList*) malloc(sizeof(NodeList));

	vtail->listhead.key = LLONG_MAX;
	vtail->listhead.next.store(ehead2, std::memory_order_seq_cst);
	vtail->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&vtail->lock, NULL);
	vtail->next.store(NULL, std::memory_order_seq_cst);

	vhead->next.store(vtail, std::memory_order_seq_cst);

	for(i=1; i<=initial_vertices; i++)
	{
		Node *ehead = (Node*) malloc(sizeof(Node));

		ehead->key = LLONG_MIN;
		ehead->next.store(NULL, std::memory_order_seq_cst);
		ehead->status.store(3, std::memory_order_seq_cst);

		Node *etail = (Node*) malloc(sizeof(Node));

		etail->key = LLONG_MAX;
		etail->next.store(NULL, std::memory_order_seq_cst);
		etail->status.store(3, std::memory_order_seq_cst);

		ehead->next.store(etail, std::memory_order_seq_cst);

		NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));

		newlisthead->listhead.key = i;
		newlisthead->listhead.next.store(ehead, std::memory_order_seq_cst);
		newlisthead->marked.store(false, std::memory_order_seq_cst);
		pthread_mutex_init(&newlisthead->lock, NULL);
		newlisthead->next.store(NULL, std::memory_order_seq_cst);

		NodeList *temp = vhead;
		while(temp->next.load(std::memory_order_seq_cst) != vtail)
			temp = temp->next.load(std::memory_order_seq_cst);

		temp->next.store(newlisthead, std::memory_order_seq_cst);
		newlisthead->next.store(vtail, std::memory_order_seq_cst);

		for(j=1;j<=initial_vertices;j++)
		{
			if(i == j)
				continue;	//avoid self loops

			Node *newnode = (Node*) malloc(sizeof(Node));
			newnode->key = j;
			newnode->next.store(NULL, std::memory_order_seq_cst);
			newnode->status.store(3, std::memory_order_seq_cst);

			Node *pred = newlisthead->listhead.next;
			Node *curr = pred->next;

			while(curr->key < j)
			{
				pred = curr;
				curr = curr->next;
			}
			pred->next = newnode;
			newnode->next = curr;
		
			if(cycle_detect(j, i)) //remove edge
				pred->next = curr;
		}
	}
}

int main(int argc, char*argv[])	//command line arguments - #threads, #vertices initially, #operations per threads
{
	if(argc < 1)
	{
		cout << "Enter 1 command line arguments - #vertices initially" << endl;
		return 0;
	}

	int initial_vertices = atoi(argv[1]); 		// initial number of vertices
	vhead = vtail = NULL;

	create_initial_vertices(initial_vertices);

//	print_graph();

	ofstream outfile;
   	outfile.open("graph.txt");

	NodeList *temp1 = vhead->next;
	Node *temp2;

	while(temp1 != vtail)
	{
		outfile << temp1->listhead.key << " ";
		temp2 = temp1->listhead.next.load(std::memory_order_seq_cst);
		temp2 = temp2->next;
		while(temp2->next != NULL)
		{
			outfile << temp2->key << " ";
			temp2 = temp2->next.load(std::memory_order_seq_cst);
		}
		outfile << endl;
		temp1 = temp1->next.load(std::memory_order_seq_cst);
	}

	outfile.close();
	
	return 0;
}

