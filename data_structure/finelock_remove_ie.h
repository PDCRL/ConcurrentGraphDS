/*
 * File:
 *   finelock-remove_ie.h
 *
 * Author(s):
 *   Dr. Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *
 * Description:
 *   Fine-Grained Lock-based implementation of a concurrent directed graph (represented as adjacency list) with deletion of incoming edges of deleted vertices
 *
 * Copyright (c) 2017.
 *
 * finelock-remove_ie.h is part of ConcurrentGraphDS
*/


#include <stdio.h>
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
	Node *next;
};

struct NodeList
{
	Node listhead;
	NodeList *next;
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
		temp2 = temp1->listhead.next;
		while(temp2 != NULL)
		{
			cout << temp2->key << " ";
			temp2 = temp2->next;
		}
		cout << endl;
		temp1 = temp1->next;
	}
}

void create_initial_vertices(int initial_vertices)
{
	int i;

	Node *ehead1 = (Node*) malloc(sizeof(Node));

	ehead1->key = LLONG_MIN;
	ehead1->next = NULL;
	ehead1->marked.store(false);
	pthread_mutex_init(&ehead1->lock, NULL);
	ehead1->next = NULL;

	Node *etail1 = (Node*) malloc(sizeof(Node));

	etail1->key = LLONG_MAX;
	etail1->next = NULL;
	etail1->marked.store(false);
	pthread_mutex_init(&etail1->lock, NULL);
	etail1->next = NULL;

	ehead1->next = etail1;

	Node *ehead2 = (Node*) malloc(sizeof(Node));

	ehead2->key = LLONG_MIN;
	ehead2->next = NULL;
	ehead2->marked.store(false);
	pthread_mutex_init(&ehead2->lock, NULL);
	ehead2->next = NULL;

	Node *etail2 = (Node*) malloc(sizeof(Node));

	etail2->key = LLONG_MAX;
	etail2->next = NULL;
	etail2->marked.store(false);
	pthread_mutex_init(&etail2->lock, NULL);
	etail2->next = NULL;

	ehead2->next = etail2;

	vhead = (NodeList*) malloc(sizeof(NodeList));

	vhead->listhead.key = LLONG_MIN;
	vhead->listhead.next = ehead1;
	vhead->listhead.marked.store(false);
	pthread_mutex_init(&vhead->listhead.lock, NULL);
	vhead->next = NULL;

	vtail = (NodeList*) malloc(sizeof(NodeList));

	vtail->listhead.key = LLONG_MAX;
	vtail->listhead.next = ehead2;
	vtail->listhead.marked.store(false);
	pthread_mutex_init(&vtail->listhead.lock, NULL);
	vtail->next = NULL;

	vhead->next = vtail;

	for(i=1; i<=initial_vertices; i++)
	{
		Node *ehead = (Node*) malloc(sizeof(Node));

		ehead->key = LLONG_MIN;
		ehead->next = NULL;
		ehead->marked.store(false);
		pthread_mutex_init(&ehead->lock, NULL);
		ehead->next = NULL;

		Node *etail = (Node*) malloc(sizeof(Node));

		etail->key = LLONG_MAX;
		etail->next = NULL;
		etail->marked.store(false);
		pthread_mutex_init(&etail->lock, NULL);
		etail->next = NULL;

		ehead->next = etail;

		NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));

		newlisthead->listhead.key = i;
		newlisthead->listhead.next = ehead;
		newlisthead->listhead.marked.store(false);
		pthread_mutex_init(&newlisthead->listhead.lock, NULL);
		newlisthead->next = NULL;

		NodeList *temp = vhead;
		while(temp->next != vtail)
			temp = temp->next;

		temp->next = newlisthead;
		newlisthead->next = vtail;
	}
}

bool validateList(NodeList *pred, NodeList *curr)
{
	return !pred->listhead.marked.load() && !curr->listhead.marked.load() && pred->next == curr;
}

bool validateNode(Node *pred, Node *curr)
{
	return !pred->marked.load() && !curr->marked.load() && pred->next == curr;
}

int add_vertex(long long int v)
{
	Node *ehead = (Node*) malloc(sizeof(Node));

	ehead->key = LLONG_MIN;
	ehead->next = NULL;
	ehead->marked.store(false);
	pthread_mutex_init(&ehead->lock, NULL);
	ehead->next = NULL;

	Node *etail = (Node*) malloc(sizeof(Node));

	etail->key = LLONG_MAX;
	etail->next = NULL;
	etail->marked.store(false);
	pthread_mutex_init(&etail->lock, NULL);
	etail->next = NULL;

	ehead->next = etail;


	NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));
	newlisthead->listhead.key = v;
	newlisthead->listhead.next = ehead;
	newlisthead->listhead.marked.store(false);
	newlisthead->next = NULL;
	pthread_mutex_init(&newlisthead->listhead.lock, NULL);

	NodeList *pred, *curr;

loop1:	while(true)
	{
		pred = vhead;
		curr = pred->next;
	
		while(curr->listhead.key < v)
		{
			pred = curr;
			curr = curr->next;
		}

		pthread_mutex_lock(&pred->listhead.lock);
		pthread_mutex_lock(&curr->listhead.lock);

		if(validateList(pred,curr) == false)
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
			newlisthead->next = curr;
			pred->next = newlisthead;
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
		curr = pred->next;
	
		while(curr->listhead.key < v)
		{
			pred = curr;
			curr = curr->next;
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
			curr->listhead.marked.store(true);
			pred->next = curr->next;
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
	//search for vertex v
	NodeList *temp = vhead;

	while(temp->listhead.key < v)
		temp = temp->next;

	if(temp->listhead.key != v || temp->listhead.marked.load() == true)
		return false;
	
	temp = vhead;
	while(temp->listhead.key < u)
		temp = temp->next;

	if(temp->listhead.key != u || temp->listhead.marked.load() == true)
		return false;

	//found both u,v in graph - now insert edge
			
	Node *newnode = (Node*) malloc(sizeof(Node));
	newnode->key = v;
	newnode->next = NULL;
	newnode->marked.store(false);
	pthread_mutex_init(&newnode->lock, NULL);

	Node *pred, *curr;

loop3:	while(true)
	{
		pred = temp->listhead.next;
		curr = pred->next;
		
		while(curr->key < v)
		{
			pred = curr;
			curr = curr->next;
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
			newnode->next = curr;
			pred->next = newnode;
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			return true;
		}
	}
}

int remove_edge(long long int u, long long int v)
{
	//search for vertex v
	NodeList *temp = vhead;
	
	while(temp->listhead.key < v)
		temp = temp->next;

	if(temp->listhead.key != v || temp->listhead.marked.load() == true)
		return false;
	
	temp = vhead;
	while(temp->listhead.key < u)
		temp = temp->next;

	if(temp->listhead.key != u || temp->listhead.marked.load() == true)
		return false;

	//found both u,v in graph - now delete edge

	Node *pred, *curr;

loop4:	while(true)
	{	
		pred = temp->listhead.next;
		curr = pred->next;
		
		while(curr->key < v)
		{
			pred = curr;
			curr = curr->next;
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
			curr->marked.store(true);
			pred->next = curr->next;
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
		temp = temp->next;

	if(temp->listhead.key != u || temp->listhead.marked.load() == true)
		return false;

	return true;
}


int contains_edge(long long int u, long long int v)
{
	//search for vertex v
	NodeList *temp = vhead;
	
	while(temp->listhead.key < v)
		temp = temp->next;

	if(temp->listhead.key != v || temp->listhead.marked.load() == true)
		return false;

	temp = vhead;
	while(temp->listhead.key < u)
		temp = temp->next;

	if(temp->listhead.key != u || temp->listhead.marked.load() == true)
		return false;

	Node *pred = temp->listhead.next;

	while(pred->key < v)
		pred = pred->next;

	if(pred->key != v || pred->marked.load() == true)
		return false;

	return true;
}

void adjremove(long long int v)
{
	NodeList *temp = vhead;
	Node *pred, *curr;

	while(temp != NULL)
	{
		loop6:	while(true)
		{
			pred = temp->listhead.next;
			curr = pred->next;
			
			while(curr->key < v)
			{
				pred = curr;
				curr = curr->next;
			}
	
			pthread_mutex_lock(&pred->lock);
			pthread_mutex_lock(&curr->lock);
	
			if(validateNode(pred,curr) == false)
			{
				pthread_mutex_unlock(&pred->lock);
				pthread_mutex_unlock(&curr->lock);
				goto loop6;
			}
	
			if(curr->key == v)
			{
				curr->marked.store(true);
				pred->next = curr->next;
				pthread_mutex_unlock(&pred->lock);
				pthread_mutex_unlock(&curr->lock);
				goto loop5;
			}
			else
			{
				pthread_mutex_unlock(&pred->lock);
				pthread_mutex_unlock(&curr->lock);
				goto loop5;
			}
		}
	
loop5:		temp = temp->next;
	}
}
