/*
 * File:
 *   coarse-lock.h
 *
 * Author(s):
 *   Dr. Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *
 * Description:
 *   Coarse Lock-based implementation of a concurrent directed graph (represented as adjacency list)
 *
 * Copyright (c) 2017.
 *
 * coarse-lock.h is part of ConcurrentGraphDS
*/

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <list>
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
#include <list>
#include <set>
#include <vector>

using namespace std;

struct Node
{
	long long int key;
	Node *next;
};

struct NodeList
{
	Node listhead;
	NodeList *next;
};

NodeList *graph;
pthread_mutex_t lock;

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

	NodeList *temp = graph;
	while(temp->listhead.key < v && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != v)
		return false;
	
	Node *adj;

	reach.insert(make_pair(u, false));
	iterator_type i;
	while(true)
	{
		for(i=reach.begin(); i!=reach.end(); i++)
			if(i->second == false)
				break;

		if(i == reach.end())
			break;

		key = i->first;

		temp = graph;
		while(temp->listhead.key < key && temp->next != NULL)
			temp = temp->next;

		if(temp->listhead.key != key)
		{
			for(iterator_type i=reach.begin(); i!=reach.end(); i++)
				if(i->first == key)
				{
					reach.erase(i);
					break;
				}
			continue;
		}

		adj = temp->listhead.next;
		while(adj != NULL)
		{
			if(reach.find(make_pair(adj->key, false)) == reach.end() && reach.find(make_pair(adj->key, true)) == reach.end())
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
	NodeList *temp1 = graph;
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
	int i, j;
	for(i=1;i<=initial_vertices;i++)
	{
		NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));

		newlisthead->listhead.key = i;
		newlisthead->listhead.next = NULL;
		newlisthead->next = NULL;
		if(i==1)
			graph = newlisthead;
		else
		{
			NodeList *temp = graph;
			while(temp->next != NULL)
				temp = temp->next;
			temp->next = newlisthead;
		}

		for(j=1;j<=initial_vertices;j++)
		{
			if(i == j)
				continue;	//avoid self loops
			Node *newnode = (Node*) malloc(sizeof(Node));
			newnode->key = j;
			newnode->next = NULL;
			if(newlisthead->next == NULL)
				newlisthead->listhead.next = newnode;
			else
			{
				Node *temp = newlisthead->listhead.next;
				while(temp->next != NULL)
					temp = temp->next;
				temp->next = newnode;
				if(cycle_detect(j,i))
					temp->next = NULL;
			}
		}
	}
}

void adjremove(long long int key)
{
	NodeList *temp = graph;
	Node *pred, *curr;
	while(temp != NULL)
	{
		pred = temp->listhead.next;
		if(pred == NULL)
			goto l3;
		if(pred->key == key)
		{
			temp->listhead.next = temp->listhead.next->next;
			goto l3;
		}
		if(pred->next == NULL)
			goto l3;

		curr = pred->next;

		while(curr->key < key && curr->next != NULL)
		{
			pred = pred->next;
			curr = curr->next;
		}
		
		if(curr->key == key)
			pred->next = curr->next;

l3:		temp = temp->next;
	}
}

int add_vertex(long long int v)
{
	NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));
	newlisthead->listhead.key = v;
	newlisthead->listhead.next = NULL;
	newlisthead->next = NULL;
	
	if(graph == NULL)
	{
		graph = newlisthead;
		return true;
	}

	if(graph->next == NULL)
	{
		if(graph->listhead.key < v)
		{
			graph->next = newlisthead;
			return true;
		}
	}
	if(graph->listhead.key == v)
		return true;

	if(graph->listhead.key > v)
	{
		newlisthead->next = graph;
		graph = newlisthead;
		return true;
	}

	NodeList *pred = graph;
	NodeList *curr = pred->next;
	
	while(curr->listhead.key < v && curr->next != NULL)
	{
		pred = curr;
		curr = curr->next;
	}

	if(curr->listhead.key == v)
		return true;

	else if(curr->listhead.key < v)
	{
		curr->next = newlisthead;
		return true;
	}
	else if(curr->listhead.key > v)
	{
		newlisthead->next = curr;
		pred->next = newlisthead;
		return true;
	}
}

int remove_vertex(long long int v)
{
	if(graph == NULL)
	{
		return false;
	}

	if(graph->next == NULL)
		if(graph->listhead.key != v)
			return false;

	if(graph->listhead.key == v)
	{
		graph = graph->next;
		return true;
	}

	NodeList *pred = graph;
	NodeList *curr = pred->next;

	while(curr->listhead.key < v && curr->next != NULL)
	{
		pred = curr;
		curr = curr->next;
	}

	if(curr->listhead.key == v)
	{
		pred->next = curr->next;
		return true;
	}
	else
		return false;
}

int add_edge(long long int u, long long int v)
{
	if(graph == NULL || graph->next == NULL)	//need 2 vertices in graph at least
		return false;

	//search for vertex v
	NodeList *temp = graph;
	
	while(temp->listhead.key != v && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != v)
		return false;

	temp = graph;
	while(temp->listhead.key != u && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != u)
		return false;

	//found both u,v in graph - now insert edge
	Node *newnode = (Node*) malloc(sizeof(Node));
	newnode->key = v;
	newnode->next = NULL;

	Node *pred = temp->listhead.next;
	if(pred == NULL)	
	{
		temp->listhead.next = newnode;
		return true;
	}

	if(pred->key == v)		//edge already present
		return true;

	if(pred->key > v)
	{
		temp->listhead.next = newnode;
		newnode->next = pred;
		return true;
	}

	if(pred->next == NULL)
	{
		pred->next = newnode;
		return true;
	}

	Node *curr = pred->next;
	while(curr->key < v && curr->next != NULL)
	{
		pred = curr;
		curr = curr->next;
	}
	if(curr->key == v)		//edge already present
		return true;

	if(curr->key < v)
	{
		curr->next = newnode;
		return true;
	}
	else if(curr->key > v)
	{
		newnode->next = curr;
		pred->next = newnode;
		return true;
	}
}

int remove_edge(long long int u, long long int v)
{
	if(graph == NULL || graph->next == NULL)	//need 2 vertices in graph at least
		return false;

	//search for vertex v
	NodeList *temp = graph;
	
	while(temp->listhead.key != v && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != v)
		return false;

	temp = graph;
	while(temp->listhead.key != u && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != u)
		return false;

	//found both u,v in graph - now delete edge

	Node *pred = temp->listhead.next;
	if(pred == NULL)	
		return true;

	if(pred->key == v)		//edge present
	{
		temp->listhead.next = pred->next;
		return true;
	}

	if(pred->next == NULL)
		return true;

	Node *curr = pred->next;
	while(curr->key < v && curr->next != NULL)
	{
		pred = curr;
		curr = curr->next;
	}
	if(curr->key == v)		//edge present
		pred->next = curr->next;
	return true;
}

int contains_vertex(long long int u)
{
	if(graph == NULL || graph->next == NULL)	//need 2 vertices in graph at least
		return false;

	//search for vertex v
	NodeList *temp = graph;
	
	while(temp->listhead.key != u && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != u)
		return false;

	return true;
}


int contains_edge(long long int u, long long int v)
{
	if(graph == NULL || graph->next == NULL)	//need 2 vertices in graph at least
		return false;

	//search for vertex v
	NodeList *temp = graph;
	
	while(temp->listhead.key != v && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != v)
		return false;

	temp = graph;
	while(temp->listhead.key != u && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != u)
		return false;

	Node *pred = temp->listhead.next;
	if(pred == NULL)	
		return false;

	if(pred->key == v)		//edge present
	{
		return true;
	}

	if(pred->next == NULL)
		return false;

	Node *curr = pred->next;
	while(curr->key < v && curr->next != NULL)
	{
		pred = curr;
		curr = curr->next;
	}
	if(curr->key == v)		//edge present
	{
		return true;
	}
	else
		return false;
}
