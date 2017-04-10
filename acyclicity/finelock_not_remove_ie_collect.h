/*
 * File:
 *   finelock_not_remove_ie_collect.h
 *
 * Author(s):
 *   Dr. Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *
 * Description:
 *   Fine-Grained Lock-based implementation of a concurrent directed graph (represented as adjacency list) without deletion of incoming edges of deleted vertices, with cycle detect as collect method
 *
 * Copyright (c) 2017.
 *
 * finelock_not_remove_ie_collect.h is part of ConcurrentGraphDS
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
	atomic<int> status;  	// 1- transit, 2 - marked, 3 - added
	pthread_mutex_t lock;
	Node *next;
};

struct NodeList
{
	Node listhead;
	atomic<bool> marked;
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
	ehead1->status.store(3);
	pthread_mutex_init(&ehead1->lock, NULL);
	ehead1->next = NULL;

	Node *etail1 = (Node*) malloc(sizeof(Node));

	etail1->key = LLONG_MAX;
	etail1->next = NULL;
	etail1->status.store(3);
	pthread_mutex_init(&etail1->lock, NULL);
	etail1->next = NULL;

	ehead1->next = etail1;

	Node *ehead2 = (Node*) malloc(sizeof(Node));

	ehead2->key = LLONG_MIN;
	ehead2->next = NULL;
	ehead2->status.store(3);
	pthread_mutex_init(&ehead2->lock, NULL);
	ehead2->next = NULL;

	Node *etail2 = (Node*) malloc(sizeof(Node));

	etail2->key = LLONG_MAX;
	etail2->next = NULL;
	etail2->status.store(3);
	pthread_mutex_init(&etail2->lock, NULL);
	etail2->next = NULL;

	ehead2->next = etail2;

	vhead = (NodeList*) malloc(sizeof(NodeList));

	vhead->listhead.key = LLONG_MIN;
	vhead->listhead.next = ehead1;
	vhead->marked.store(false);
	pthread_mutex_init(&vhead->listhead.lock, NULL);
	vhead->next = NULL;

	vtail = (NodeList*) malloc(sizeof(NodeList));

	vtail->listhead.key = LLONG_MAX;
	vtail->listhead.next = ehead2;
	vtail->marked.store(false);
	pthread_mutex_init(&vtail->listhead.lock, NULL);
	vtail->next = NULL;

	vhead->next = vtail;

	for(i=1; i<=initial_vertices; i++)
	{
		Node *ehead = (Node*) malloc(sizeof(Node));

		ehead->key = LLONG_MIN;
		ehead->next = NULL;
		ehead->status.store(3);
		pthread_mutex_init(&ehead->lock, NULL);
		ehead->next = NULL;

		Node *etail = (Node*) malloc(sizeof(Node));

		etail->key = LLONG_MAX;
		etail->next = NULL;
		etail->status.store(3);
		pthread_mutex_init(&etail->lock, NULL);
		etail->next = NULL;

		ehead->next = etail;

		NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));

		newlisthead->listhead.key = i;
		newlisthead->listhead.next = ehead;
		newlisthead->marked.store(false);
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
	return !pred->marked.load() && !curr->marked.load() && pred->next == curr;
}

bool validateNode(Node *pred, Node *curr)
{
	return (pred->status.load() == 3) && (curr->status.load() == 3) && (pred->next == curr);
}

NodeList* collect()			//get the snapshot of the graph
{
	NodeList *snap_head = NULL;
        NodeList *temp_snap, *original, *snap_new;
        Node *snap_node, *original_node, *new_node;

        original = vhead;
        while(original != NULL)
        {
		if(original->marked.load() == true)	
			goto next1;

		snap_new = (NodeList*) malloc(sizeof(NodeList));

		snap_new->listhead.key = original->listhead.key;
		snap_new->listhead.next = NULL;
		snap_new->marked.store(false);
		snap_new->next = NULL;            	
		
		if(snap_head == NULL)
            	{
        	        temp_snap = snap_new;
        		snap_head = snap_new;
        	}
              	else
		{
        	    	temp_snap->next = snap_new;
        	    	temp_snap = snap_new;
        	}
  
	        original_node = original->listhead.next;
        	while(original_node != NULL)
           	{
			if(original_node->status.load() != 2)
				goto next2;

			new_node = (Node*) malloc(sizeof(Node));
			new_node->key = original_node->key;
			new_node->status.store(3);
			new_node->next = NULL;

            		if(temp_snap->listhead.next == NULL)	//1st node 
            		{
		                temp_snap->listhead.next = new_node;
		                snap_node = new_node;
            		}
            		else
            		{
              			snap_node->next = new_node;
              			snap_node = new_node;
          		}
next2:         		original_node = original_node->next;
           	}
next1:           	original = original->next;
      	}
	return snap_head;
}

NodeList* findNode(long long int key)
{
	NodeList *temp = vhead;
	while(temp!= NULL)
	{
		if(temp->listhead.key == key)
			break;
		temp = temp->next;
	}
	return temp;
}

bool DFS_visit(NodeList *temp, vector<long long int> &visited, vector<long long int> &In_stack)
{
	Node *newnode = temp->listhead.next;
	visited.push_back(temp->listhead.key);
	In_stack.push_back(temp->listhead.key);

	while(newnode != NULL)
	{
		if(find(In_stack.begin(), In_stack.end(), newnode->key) != In_stack.end())	//found node in recursion stack
			return true;
		else if(find(visited.begin(), visited.end(), newnode->key) == visited.end() && DFS_visit(findNode(newnode->key), visited, In_stack) == true)
			return true;
		newnode = newnode->next;
	}
	In_stack.erase(find(In_stack.begin(), In_stack.end()+1, temp->listhead.key));
	return false;
}

int cycle_detect()
{				
	vector<long long int> visited, In_stack;
	NodeList *temp = vhead;
	bool cycle = false;

	while(temp != NULL)
	{
		if(visited.end() == find(visited.begin(), visited.end(), temp->listhead.key))	//not visited
		{
			cycle = DFS_visit(temp, visited, In_stack);
			if(cycle)
				return true;	
		}
		temp = temp->next;
	}
	return false;
}   

int cycle_detect(long long int u, long long int v)		//is there a path from u to v?
{
	NodeList *snapshot;
	snapshot = collect();

	set<pair<long long int, bool>> reach;
	typedef set<pair<long long int, bool>>::iterator iterator_type;
	long long int key;

	NodeList *temp = snapshot;
	while(temp->listhead.key < v)
		temp = temp->next;

	if(temp->listhead.key != v || temp->marked.load() == true)
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

		temp = vhead;
		while(temp->listhead.key < key)
			temp = temp->next;

		if(temp->listhead.key != key || temp->marked.load() == true)
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
			if(adj->status.load() != 2 && reach.find(make_pair(adj->key, false)) == reach.end() && reach.find(make_pair(adj->key, true)) == reach.end())
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

int add_vertex(long long int v)
{
	Node *ehead = (Node*) malloc(sizeof(Node));

	ehead->key = LLONG_MIN;
	ehead->next = NULL;
	ehead->status.store(3);
	pthread_mutex_init(&ehead->lock, NULL);
	ehead->next = NULL;

	Node *etail = (Node*) malloc(sizeof(Node));

	etail->key = LLONG_MAX;
	etail->next = NULL;
	etail->status.store(3);
	pthread_mutex_init(&etail->lock, NULL);
	etail->next = NULL;

	ehead->next = etail;


	NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));
	newlisthead->listhead.key = v;
	newlisthead->listhead.next = ehead;
	newlisthead->marked.store(false);
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
			curr->marked.store(true);
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
	bool res;
	NodeList *temp1, *temp2;

	if(u < v)
	{
		temp1 = vhead;

		while(temp1->listhead.key < u)
			temp1 = temp1->next;
	
		if(temp1->listhead.key != u || temp1->marked.load() == true)
			return false;
		
		temp2 = temp1->next;
		
		while(temp2->listhead.key < v)
			temp2 = temp2->next;
	
		if(temp2->listhead.key != v || temp2->marked.load() == true)
			return false;
	}
	else
	{
		temp2 = vhead;

		while(temp2->listhead.key < v)
			temp2 = temp2->next;
	
		if(temp2->listhead.key != v || temp2->marked.load() == true)
			return false;
		
		temp1 = temp2->next;
		while(temp1->listhead.key < u)
			temp1 = temp1->next;
	
		if(temp1->listhead.key != u || temp1->marked.load() == true)
			return false;
	}
	
	if(temp1->marked.load() == true || temp2->marked.load() == true)
		return false;

	//found both u,v in graph - now insert edge
			
	Node *newnode = (Node*) malloc(sizeof(Node));
	newnode->key = v;
	newnode->next = NULL;
	newnode->status.store(1);
	pthread_mutex_init(&newnode->lock, NULL);

	Node *pred, *curr;

loop3:	while(true)
	{
		pred = temp1->listhead.next;
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
			res = cycle_detect(v,u);
			if(res == true)
			{
				newnode->status.store(2);
				pred->next = curr;
			}
			else
			{
				newnode->status.store(3);
			}
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
			temp1 = temp1->next;
	
		if(temp1->listhead.key != u || temp1->marked.load() == true)
			return false;
		
		temp2 = temp1->next;
		
		while(temp2->listhead.key < v)
			temp2 = temp2->next;
	
		if(temp2->listhead.key != v || temp2->marked.load() == true)
			return false;
	}
	else
	{
		temp2 = vhead;

		while(temp2->listhead.key < v)
			temp2 = temp2->next;
	
		if(temp2->listhead.key != v || temp2->marked.load() == true)
			return false;
		
		temp1 = temp2->next;
		while(temp1->listhead.key < u)
			temp1 = temp1->next;
	
		if(temp1->listhead.key != u || temp1->marked.load() == true)
			return false;
	}
	
	if(temp1->marked.load() == true || temp2->marked.load() == true)
		return false;

	//found both u,v in graph - now delete edge

	Node *pred, *curr;

loop4:	while(true)
	{	
		pred = temp1->listhead.next;
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
			curr->status.store(2);
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

	if(temp->listhead.key != u || temp->marked.load() == true)
		return false;

	return true;
}


int contains_edge(long long int u, long long int v)
{
	//search for vertex v
	NodeList *temp = vhead;
	
	while(temp->listhead.key < v)
		temp = temp->next;

	if(temp->listhead.key != v || temp->marked.load() == true)
		return false;

	temp = vhead;
	while(temp->listhead.key < u)
		temp = temp->next;

	if(temp->listhead.key != u || temp->marked.load() == true)
		return false;

	Node *pred = temp->listhead.next;

	while(pred->key < v)
		pred = pred->next;

	if(pred->key != v || pred->status.load() != 3)		//edge present
		return false;

	return true;
}
