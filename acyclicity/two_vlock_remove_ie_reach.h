/*
 * File:
 *   two_vlock_remove_ie_reach.h
 *
 * Author(s):
 *   Dr. Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *
 * Description:
 *   Fine-Grained Vertex Lock-based implementation of a concurrent directed graph (represented as adjacency list) with deletion of incoming edges of deleted vertices, with cycle detect as reach method
 *
 * Copyright (c) 2017.
 *
 * two_vlock_remove_ie_reach.h is part of ConcurrentGraphDS
*/

#include <stdio.h>
#include <iostream>
#include <assert.h>
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
	atomic<NodeList*> next;
	pthread_mutex_t lock;
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

bool validateList(NodeList *pred, NodeList *curr)
{
	return !pred->marked.load(std::memory_order_seq_cst) && !curr->marked.load(std::memory_order_seq_cst) && pred->next.load(std::memory_order_seq_cst) == curr;
}

bool validateNode1(Node *pred, Node *curr)
{
	return (pred->status.load(std::memory_order_seq_cst) == 3) && (curr->status.load(std::memory_order_seq_cst) == 3) && (pred->next.load(std::memory_order_seq_cst) == curr);
}

bool validateNode2(Node *pred, Node *curr)
{
	return (pred->status.load(std::memory_order_seq_cst) != 2) && (curr->status.load(std::memory_order_seq_cst) != 2) && (pred->next.load(std::memory_order_seq_cst) == curr);
}

void adjremove(long long int v)
{
	NodeList *temp = vhead->next;
	Node *pred, *curr;

	while(temp != vtail)
	{
		pthread_mutex_lock(&temp->lock);

		loop6:	while(true)
		{
			pred = temp->listhead.next.load(std::memory_order_seq_cst);
			curr = pred->next.load(std::memory_order_seq_cst);
			
			if(curr->next == NULL)
			{
				pthread_mutex_unlock(&temp->lock);
				goto loop5;
			}
			
			while(curr->key < v)
			{
				pred = curr;
				curr = curr->next.load(std::memory_order_seq_cst);
			}
	
			if(validateNode1(pred,curr) == false)
			{
				pthread_mutex_unlock(&temp->lock);
				goto loop6;
			}
	
			if(curr->key == v)		//edge present
			{
				curr->status.store(2, std::memory_order_seq_cst);
				pred->next.store(curr->next, std::memory_order_seq_cst);
				pthread_mutex_unlock(&temp->lock);
				goto loop5;
			}
			else
			{
				pthread_mutex_unlock(&temp->lock);
				goto loop5;
			}
		}
	
loop5:		temp = temp->next.load(std::memory_order_seq_cst);
	}
}

int add_vertex(long long int v)
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
	newlisthead->listhead.key = v;
	newlisthead->listhead.next.store(ehead, std::memory_order_seq_cst);
	newlisthead->marked.store(false, std::memory_order_seq_cst);
	newlisthead->next.store(NULL, std::memory_order_seq_cst);
	pthread_mutex_init(&newlisthead->lock, NULL);

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

		pthread_mutex_lock(&pred->lock);
		pthread_mutex_lock(&curr->lock);

		if(validateList(pred,curr) == false)
		{
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			goto loop1;
		}

		if(curr->listhead.key == v)
		{
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			return true;
		}
		else
		{
			newlisthead->next.store(curr, std::memory_order_seq_cst);
			pred->next.store(newlisthead, std::memory_order_seq_cst);
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
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

		pthread_mutex_lock(&pred->lock);
		pthread_mutex_lock(&curr->lock);

		if(validateList(pred,curr) == false)
		{
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			goto loop2;
		}

		if(curr->listhead.key == v)
		{
			curr->marked.store(true);
			pred->next.store(curr->next, std::memory_order_seq_cst);
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			return true;
		}
		else
		{
			pthread_mutex_unlock(&pred->lock);
			pthread_mutex_unlock(&curr->lock);
			return false;
		}
	}
}

int add_edge(long long int u, long long int v)
{
	bool res;
	NodeList *temp1, *temp2;

	if(u < v)
	{
		temp1 = vhead;

		while(temp1->listhead.key < u)
			temp1 = temp1->next.load(std::memory_order_seq_cst);
	
		if(temp1->listhead.key != u || temp1->marked.load(std::memory_order_seq_cst) == true)
			return false;
		
		temp2 = temp1->next.load(std::memory_order_seq_cst);
		
		while(temp2->listhead.key < v)
			temp2 = temp2->next.load(std::memory_order_seq_cst);
	
		if(temp2->listhead.key != v || temp2->marked.load(std::memory_order_seq_cst) == true)
			return false;
	}
	else
	{
		temp2 = vhead;

		while(temp2->listhead.key < v)
			temp2 = temp2->next.load(std::memory_order_seq_cst);
	
		if(temp2->listhead.key != v || temp2->marked.load(std::memory_order_seq_cst) == true)
			return false;
		
		temp1 = temp2->next;
		while(temp1->listhead.key < u)
			temp1 = temp1->next.load(std::memory_order_seq_cst);
	
		if(temp1->listhead.key != u || temp1->marked.load(std::memory_order_seq_cst) == true)
			return false;
	}
	
	if(temp1->marked.load(std::memory_order_seq_cst) == true || temp2->marked.load(std::memory_order_seq_cst) == true)
		return false;

	pthread_mutex_lock(&temp1->lock);
	pthread_mutex_lock(&temp2->lock);

	//found both u,v in graph - now insert edge
			
	Node *newnode = (Node*) malloc(sizeof(Node));
	newnode->key = v;
	newnode->next.store(NULL, std::memory_order_seq_cst);
	newnode->status.store(1);

	Node *pred, *curr, *new_pred, *new_curr;

loop3:	while(true)
	{
		pred = temp1->listhead.next.load(std::memory_order_seq_cst);
		curr = pred->next.load(std::memory_order_seq_cst);
		
		while(curr->key < v)
		{
			pred = curr;
			curr = curr->next.load(std::memory_order_seq_cst);
		}

		if(validateNode2(pred,curr) == false)
		{
			pthread_mutex_unlock(&temp1->lock);
			pthread_mutex_unlock(&temp2->lock);
			goto loop3;
		}

		if(curr->key == v)		//edge already present
		{
			pthread_mutex_unlock(&temp1->lock);
			pthread_mutex_unlock(&temp2->lock);
			return true;
		}
		else
		{
			newnode->next.store(curr, std::memory_order_seq_cst);
			pred->next.store(newnode, std::memory_order_seq_cst);

			pthread_mutex_unlock(&temp1->lock);
			pthread_mutex_unlock(&temp2->lock);

			res = cycle_detect(v, u);

			if(res == true)
			{
				pthread_mutex_lock(&temp1->lock);
				pthread_mutex_lock(&temp2->lock);

loop7:				while(true)
				{
					new_pred = temp1->listhead.next.load(std::memory_order_seq_cst);
					new_curr = new_pred->next.load(std::memory_order_seq_cst);
		
					while(new_curr->key < v)
					{
						new_pred = new_curr;
						new_curr = new_curr->next.load(std::memory_order_seq_cst);
					}

					if(validateNode2(new_pred, new_curr) == false)
					{
						pthread_mutex_unlock(&temp1->lock);
						pthread_mutex_unlock(&temp2->lock);
						goto loop7;
					}
		
					newnode->status.store(2, std::memory_order_seq_cst);
					new_pred->next.store(new_curr->next, std::memory_order_seq_cst);
		
					pthread_mutex_unlock(&temp1->lock);
					pthread_mutex_unlock(&temp2->lock);
	
					return false;
				}				
			}
			else
			{
				newnode->status.store(3, std::memory_order_seq_cst);
				return true;
			}
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
	
		if(temp1->listhead.key != u || temp1->marked.load(std::memory_order_seq_cst) == true)
			return false;
		
		temp2 = temp1->next.load(std::memory_order_seq_cst);
		
		while(temp2->listhead.key < v)
			temp2 = temp2->next.load(std::memory_order_seq_cst);
	
		if(temp2->listhead.key != v || temp2->marked.load(std::memory_order_seq_cst) == true)
			return false;
	}
	else
	{
		temp2 = vhead;

		while(temp2->listhead.key < v)
			temp2 = temp2->next.load(std::memory_order_seq_cst);
	
		if(temp2->listhead.key != v || temp2->marked.load(std::memory_order_seq_cst) == true)
			return false;
		
		temp1 = temp2->next.load(std::memory_order_seq_cst);

		while(temp1->listhead.key < u)
			temp1 = temp1->next.load(std::memory_order_seq_cst);
	
		if(temp1->listhead.key != u || temp1->marked.load(std::memory_order_seq_cst) == true)
			return false;
	}
	
	if(temp1->marked.load(std::memory_order_seq_cst) == true || temp2->marked.load(std::memory_order_seq_cst) == true)
		return false;

	pthread_mutex_lock(&temp1->lock);
	pthread_mutex_lock(&temp2->lock);

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

		if(validateNode1(pred,curr) == false)
		{
			pthread_mutex_unlock(&temp1->lock);
			pthread_mutex_unlock(&temp2->lock);
			goto loop4;
		}

		if(curr->key == v)		//edge present
		{
			curr->status.store(2, std::memory_order_seq_cst);
			pred->next.store(curr->next, std::memory_order_seq_cst);
			pthread_mutex_unlock(&temp1->lock);
			pthread_mutex_unlock(&temp2->lock);
			return true;
		}
		else
		{
			pthread_mutex_unlock(&temp1->lock);
			pthread_mutex_unlock(&temp2->lock);
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

	if(temp->listhead.key != u || temp->marked.load(std::memory_order_seq_cst) == true)
		return false;

	return true;
}


int contains_edge(long long int u, long long int v)
{
	//search for vertex v
	NodeList *temp = vhead;
	
	while(temp->listhead.key < v)
		temp = temp->next.load(std::memory_order_seq_cst);

	if(temp->listhead.key != v || temp->marked.load(std::memory_order_seq_cst) == true)
		return false;

	temp = vhead;
	while(temp->listhead.key < u)
		temp = temp->next.load(std::memory_order_seq_cst);

	if(temp->listhead.key != u || temp->marked.load(std::memory_order_seq_cst) == true)
		return false;

	Node *pred = temp->listhead.next.load(std::memory_order_seq_cst);

	while(pred->key < v)
		pred = pred->next.load(std::memory_order_seq_cst);

	if(pred->key != v || pred->status.load(std::memory_order_seq_cst) != 3)		//edge present
		return false;

	return true;
}
