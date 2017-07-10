/*
 * File:
 *   velock_not_remove_ie.h
 *
 * Author(s):
 *   Dr. Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *
 * Description:
 *   Fine-Grained Lock-based implementation of a concurrent directed graph (represented as adjacency list) without deletion of incoming edges of deleted vertices
 *
 * Copyright (c) 2017.
 *
 * velock_not_remove_ie.h is part of ConcurrentGraphDS
*/


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
	atomic<bool> marked;
	pthread_mutex_t lock;
	atomic<Node*> next;
};

struct NodeList
{
	Node listhead;
	atomic<NodeList*> next;
};

NodeList *vhead, *vtail;

typedef struct 
{
    int     secs;
    int     usecs;
}TIME_DIFF;

TIME_DIFF * my_difftime (struct timeval * start, struct timeval * end)
{
	TIME_DIFF * diff = (TIME_DIFF *) malloc ( sizeof (TIME_DIFF) );
 
	if (start->tv_sec == end->tv_sec) 
	{
        	diff->secs = 0;
        	diff->usecs = end->tv_usec - start->tv_usec;
    	}
   	else 
	{
        	diff->usecs = 1000000 - start->tv_usec;
        	diff->secs = end->tv_sec - (start->tv_sec + 1);
        	diff->usecs += end->tv_usec;
        	if (diff->usecs >= 1000000) 
		{
        	    diff->usecs -= 1000000;
	            diff->secs += 1;
	        }
	}
        return diff;
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
	int i,j;

	Node *ehead1 = (Node*) malloc(sizeof(Node));

	ehead1->key = LLONG_MIN;
	ehead1->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&ehead1->lock, NULL);
	ehead1->next.store(NULL, std::memory_order_seq_cst);

	Node *etail1 = (Node*) malloc(sizeof(Node));

	etail1->key = LLONG_MAX;
	etail1->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&etail1->lock, NULL);
	etail1->next.store(NULL, std::memory_order_seq_cst);

	ehead1->next.store(etail1, std::memory_order_seq_cst);

	Node *ehead2 = (Node*) malloc(sizeof(Node));

	ehead2->key = LLONG_MIN;
	ehead2->next.store(NULL, std::memory_order_seq_cst);
	ehead2->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&ehead2->lock, NULL);

	Node *etail2 = (Node*) malloc(sizeof(Node));

	etail2->key = LLONG_MAX;
	etail2->next.store(NULL, std::memory_order_seq_cst);
	etail2->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&etail2->lock, NULL);

	ehead2->next.store(etail2, std::memory_order_seq_cst);

	vhead = (NodeList*) malloc(sizeof(NodeList));

	vhead->listhead.key = LLONG_MIN;
	vhead->listhead.next.store(ehead1, std::memory_order_seq_cst);
	vhead->listhead.marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&vhead->listhead.lock, NULL);
	vhead->next.store(NULL, std::memory_order_seq_cst);

	vtail = (NodeList*) malloc(sizeof(NodeList));

	vtail->listhead.key = LLONG_MAX;
	vtail->listhead.next.store(ehead2, std::memory_order_seq_cst);
	vtail->listhead.marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&vtail->listhead.lock, NULL);
	vtail->next.store(NULL, std::memory_order_seq_cst);

	vhead->next.store(vtail, std::memory_order_seq_cst);

	for(i=1; i<=initial_vertices; i++)
	{
		Node *ehead = (Node*) malloc(sizeof(Node));

		ehead->key = LLONG_MIN;
		ehead->next.store(NULL, std::memory_order_seq_cst);
		ehead->marked.store(false, std::memory_order_seq_cst);
		pthread_mutex_init(&ehead->lock, NULL);

		Node *etail = (Node*) malloc(sizeof(Node));

		etail->key = LLONG_MAX;
		etail->next.store(NULL, std::memory_order_seq_cst);
		etail->marked.store(false, std::memory_order_seq_cst);
		pthread_mutex_init(&etail->lock, NULL);

		ehead->next.store(etail, std::memory_order_seq_cst);

		NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));

		newlisthead->listhead.key = i;
		newlisthead->listhead.next.store(ehead, std::memory_order_seq_cst);
		newlisthead->listhead.marked.store(false, std::memory_order_seq_cst);
		pthread_mutex_init(&newlisthead->listhead.lock, NULL);
		newlisthead->next.store(NULL, std::memory_order_seq_cst);

		NodeList *temp = vhead;
		while(temp->next.load(std::memory_order_seq_cst) != vtail)
			temp = temp->next.load(std::memory_order_seq_cst);

		temp->next.store(newlisthead, std::memory_order_seq_cst);
		newlisthead->next.store(vtail, std::memory_order_seq_cst);

		for(j=i+1;j<=initial_vertices;j++)
		{
			if(i == j)
				continue;	//avoid self loops

			Node *newnode = (Node*) malloc(sizeof(Node));
			newnode->key = j;
			newnode->next.store(NULL, std::memory_order_seq_cst);
			newnode->marked.store(false, std::memory_order_seq_cst);
			pthread_mutex_init(&newnode->lock, NULL);

			Node *pred = newlisthead->listhead.next;
			Node *curr = pred->next;

			while(curr->key < j)
			{
				pred = curr;
				curr = curr->next;
			}
			pred->next = newnode;
			newnode->next = curr;
		}
	}
}

bool validateList(NodeList *pred, NodeList *curr)
{
	return !pred->listhead.marked.load(std::memory_order_seq_cst) && !curr->listhead.marked.load(std::memory_order_seq_cst) && pred->next.load(std::memory_order_seq_cst) == curr;
}

bool validateNode(Node *pred, Node *curr)
{
	return !pred->marked.load(std::memory_order_seq_cst) && !curr->marked.load(std::memory_order_seq_cst) && pred->next.load(std::memory_order_seq_cst) == curr;
}

int add_vertex(long long int v)
{
	Node *ehead = (Node*) malloc(sizeof(Node));

	ehead->key = LLONG_MIN;
	ehead->next.store(NULL, std::memory_order_seq_cst);
	ehead->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&ehead->lock, NULL);

	Node *etail = (Node*) malloc(sizeof(Node));

	etail->key = LLONG_MAX;
	etail->next.store(NULL, std::memory_order_seq_cst);
	etail->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&etail->lock, NULL);

	ehead->next.store(etail, std::memory_order_seq_cst);

	NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));
	newlisthead->listhead.key = v;
	newlisthead->listhead.next.store(ehead, std::memory_order_seq_cst);
	newlisthead->listhead.marked.store(false, std::memory_order_seq_cst);
	newlisthead->next.store(NULL, std::memory_order_seq_cst);
	pthread_mutex_init(&newlisthead->listhead.lock, NULL);

	NodeList *pred, *curr;

loop1:	while(true)
	{
		pred = vhead;
		curr = pred->next.load(std::memory_order_seq_cst);
	
		while(curr->listhead.key < v)
		{
			pred = curr;
			curr = curr->next.load(std::memory_order_seq_cst);
		}

		pthread_mutex_lock(&pred->listhead.lock);
		pthread_mutex_lock(&curr->listhead.lock);

		if(validateList(pred, curr) == false)
		{
			pthread_mutex_unlock(&pred->listhead.lock);
			pthread_mutex_unlock(&curr->listhead.lock);
			goto loop1;
		}

		if(curr->listhead.key == v)
		{
			pthread_mutex_unlock(&pred->listhead.lock);
			pthread_mutex_unlock(&curr->listhead.lock);
			return true;
		}
		else
		{
			newlisthead->next.store(curr, std::memory_order_seq_cst);
			pred->next.store(newlisthead, std::memory_order_seq_cst);
			pthread_mutex_unlock(&pred->listhead.lock);
			pthread_mutex_unlock(&curr->listhead.lock);
			return true;
		}
	}
}

int remove_vertex(long long int v)
{
	NodeList *pred, *curr;

loop2:	while(true)
	{
		pred = vhead;
		curr = pred->next.load(std::memory_order_seq_cst);
	
		while(curr->listhead.key < v)
		{
			pred = curr;
			curr = curr->next.load(std::memory_order_seq_cst);
		}

		pthread_mutex_lock(&pred->listhead.lock);
		pthread_mutex_lock(&curr->listhead.lock);

		if(validateList(pred,curr) == false)
		{
			pthread_mutex_unlock(&pred->listhead.lock);
			pthread_mutex_unlock(&curr->listhead.lock);
			goto loop2;
		}

		if(curr->listhead.key == v)
		{
			curr->listhead.marked.store(true, std::memory_order_seq_cst);
			pred->next.store(curr->next, std::memory_order_seq_cst);
			pthread_mutex_unlock(&pred->listhead.lock);
			pthread_mutex_unlock(&curr->listhead.lock);
			return true;
		}
		else
		{
			pthread_mutex_unlock(&pred->listhead.lock);
			pthread_mutex_unlock(&curr->listhead.lock);
			return false;
		}
	}
}

int add_edge(long long int u, long long int v)
{		
	NodeList *temp1, *temp2;

	if(u < v)
	{
		temp1 = vhead;

		while(temp1->listhead.key < u)
			temp1 = temp1->next.load(std::memory_order_seq_cst);
	
		if(temp1->listhead.key != u || temp1->listhead.marked.load(std::memory_order_seq_cst) == true)
			return false;
		
		temp2 = temp1->next;
		
		while(temp2->listhead.key < v)
			temp2 = temp2->next;
	
		if(temp2->listhead.key != v || temp2->listhead.marked.load(std::memory_order_seq_cst) == true)
			return false;
	}
	else
	{
		temp2 = vhead;

		while(temp2->listhead.key < v)
			temp2 = temp2->next.load(std::memory_order_seq_cst);
	
		if(temp2->listhead.key != v || temp2->listhead.marked.load(std::memory_order_seq_cst) == true)
			return false;
		
		temp1 = temp2->next.load(std::memory_order_seq_cst);
		while(temp1->listhead.key < u)
			temp1 = temp1->next.load(std::memory_order_seq_cst);
	
		if(temp1->listhead.key != u || temp1->listhead.marked.load(std::memory_order_seq_cst) == true)
			return false;
	}
	
	if(temp1->listhead.marked.load(std::memory_order_seq_cst) == true || temp2->listhead.marked.load(std::memory_order_seq_cst) == true)
		return false;

	//found both u,v in graph - now insert edge
			
	Node *newnode = (Node*) malloc(sizeof(Node));
	newnode->key = v;
	newnode->next.store(NULL, std::memory_order_seq_cst);
	newnode->marked.store(false, std::memory_order_seq_cst);
	pthread_mutex_init(&newnode->lock, NULL);

	Node *pred, *curr;

loop3:	while(true)
	{
		pred = temp1->listhead.next.load(std::memory_order_seq_cst);
		curr = pred->next.load(std::memory_order_seq_cst);
		
		while(curr->key < v)
		{
			pred = curr;
			curr = curr->next.load(std::memory_order_seq_cst);
		}

		pthread_mutex_lock(&pred->lock);
		pthread_mutex_lock(&curr->lock);

		if(validateNode(pred,curr) == false)
		{
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			goto loop3;
		}

		if(curr->key == v)		//edge already present
		{
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			return true;
		}
		else
		{
			newnode->next.store(curr, std::memory_order_seq_cst);
			pred->next.store(newnode, std::memory_order_seq_cst);
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			return true;
		}
	}
}

int remove_edge(long long int u, long long int v)
{
	NodeList *temp1, *temp2;

	if(u < v)
	{
		temp1 = vhead;

		while(temp1->listhead.key < u)
			temp1 = temp1->next.load(std::memory_order_seq_cst);
	
		if(temp1->listhead.key != u || temp1->listhead.marked.load(std::memory_order_seq_cst) == true)
			return false;
		
		temp2 = temp1->next.load(std::memory_order_seq_cst);
		while(temp2->listhead.key < v)
			temp2 = temp2->next.load(std::memory_order_seq_cst);
	
		if(temp2->listhead.key != v || temp2->listhead.marked.load(std::memory_order_seq_cst) == true)
			return false;
	}
	else
	{
		temp2 = vhead;

		while(temp2->listhead.key < v)
			temp2 = temp2->next.load(std::memory_order_seq_cst);
	
		if(temp2->listhead.key != v || temp2->listhead.marked.load(std::memory_order_seq_cst) == true)
			return false;
		
		temp1 = temp2->next.load(std::memory_order_seq_cst);
		while(temp1->listhead.key < u)
			temp1 = temp1->next.load(std::memory_order_seq_cst);
	
		if(temp1->listhead.key != u || temp1->listhead.marked.load(std::memory_order_seq_cst) == true)
			return false;
	}
	
	if(temp1->listhead.marked.load(std::memory_order_seq_cst) == true || temp2->listhead.marked.load(std::memory_order_seq_cst) == true)
		return false;

	//found both u,v in graph - now delete edge

	Node *pred, *curr;

loop4:	while(true)
	{	
		pred = temp1->listhead.next.load(std::memory_order_seq_cst);
		curr = pred->next.load(std::memory_order_seq_cst);
		
		while(curr->key < v)
		{
			pred = curr;
			curr = curr->next.load(std::memory_order_seq_cst);
		}

		pthread_mutex_lock(&pred->lock);
		pthread_mutex_lock(&curr->lock);

		if(validateNode(pred,curr) == false)
		{
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			goto loop4;
		}

		if(curr->key == v)		//edge present
		{
			curr->marked.store(true, std::memory_order_seq_cst);
			pred->next.store(curr->next, std::memory_order_seq_cst);
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			return true;
		}
		else
		{
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			return true;
		}
	}
}

int contains_vertex(long long int u)
{
	//search for vertex v
	NodeList *temp = vhead;
	
	while(temp->listhead.key < u)
		temp = temp->next.load(std::memory_order_seq_cst);

	if(temp->listhead.key != u || temp->listhead.marked.load(std::memory_order_seq_cst) == true)
		return false;

	return true;
}


int contains_edge(long long int u, long long int v)
{
	//search for vertex v
	NodeList *temp = vhead;
	
	while(temp->listhead.key < v)
		temp = temp->next.load(std::memory_order_seq_cst);

	if(temp->listhead.key != v || temp->listhead.marked.load(std::memory_order_seq_cst) == true)
		return false;

	temp = vhead;
	while(temp->listhead.key < u)
		temp = temp->next.load(std::memory_order_seq_cst);

	if(temp->listhead.key != u || temp->listhead.marked.load(std::memory_order_seq_cst) == true)
		return false;

	Node *pred = temp->listhead.next.load(std::memory_order_seq_cst);

	while(pred->key < v)
		pred = pred->next.load(std::memory_order_seq_cst);

	if(pred->key != v || pred->marked.load(std::memory_order_seq_cst) == true)
		return false;

	return true;
}
