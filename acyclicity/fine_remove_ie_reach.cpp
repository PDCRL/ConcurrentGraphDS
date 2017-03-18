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

atomic<long long int> vertexID;
int NTHREADS, numOfOperations;

struct Node
{
	long long int key;
	atomic<int> status;  // 1- transit, 2 - marked, 3 - added
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
	if ((pred->status.load() == 3) && (curr->status.load() == 3) && (pred->next == curr))
		return true;
	return false;
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
	
			if(curr->key == v)		//edge present
			{
				curr->status.store(2);
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
int cycle_detect(long long int u, long long int v)		//is there a path from u to v?
{
	set<pair<long long int, bool>> reach;
	typedef set<pair<long long int, bool>>::iterator iterator_type;
	long long int key;

	NodeList *temp = vhead;
	while(temp->listhead.key < v)
		temp = temp->next;

	if(temp->listhead.key != v || temp->marked.load() == true)
		return false;
	
	Node *adj;
/*	temp = graph;
	while(temp->listhead.key < u && temp->next != NULL)
		temp = temp->next;

	if(temp->listhead.key != u || temp->listhead.marked.load() == true)
		return false;
*/	
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
	//not removing the vertices from the adjacency list -> check whether they are marked or reachable
}

int add_edge(long long int u, long long int v)
{
	bool res;
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

	//found both u,v in graph - now insert edge
			
	Node *newnode = (Node*) malloc(sizeof(Node));
	newnode->key = v;
	newnode->next = NULL;
	newnode->status.store(1);
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

	//found both u,v in graph - now delete edge

	Node *pred = temp->listhead.next;

	while(pred->key < v)
		pred = pred->next;

	if(pred->key != v || pred->status.load() != 3)		//edge already present
		return false;

	return true;
}

void* pthread_call(void *t)
{
	long tid=(long)t;

	// rolling a coin, 50% for cycle detetion and rest 50% -> (20%addE, 15%addV, 15%removeV)
 	
	long long int u, v;
	int other, res;
 
 	int numOfOperations_addEdge = numOfOperations * 0.25; 		// 20% for add edge
  	int numOfOperations_addVertex = numOfOperations * 0.25; 	// 15% for add vertex
  	int numOfOperations_removeVertex = numOfOperations * 0.1; 	// 15% for remove vertex
  	int numOfOperations_removeEdge = numOfOperations * 0.1; 	// 15% for remove vertex
  	int numOfOperations_containsVertex = numOfOperations * 0.15; 	// 15% for remove vertex
  	int numOfOperations_containsEdge = numOfOperations * 0.1; 	// 15% for remove vertex
	int total = numOfOperations_addEdge + numOfOperations_addVertex + numOfOperations_removeVertex + numOfOperations_removeEdge + numOfOperations_containsVertex + numOfOperations_containsEdge; 
	
	while(total != 0)
	{
		int other=rand()%6;
	        if(other == 0) 
		{
     			if(numOfOperations_addEdge != 0)
       			{	      
		l1:		u = (rand() % (vertexID.load()));		//vertex IDs are from 1
				v = (rand() % (vertexID.load()));
				if(u == v || u == 0 || v == 0)			//simple graph without self loops
					goto l1;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;

				res = add_edge(u,v); 

//				if(res == true)
//				{
//					cout << "Edge (" << u << "," << v << ") added." << endl;
//					print_graph();
//				}
//				else
//					cout << "Edge (" << u << "," << v << ") addition failed." << endl;

		         	numOfOperations_addEdge = numOfOperations_addEdge - 1;
			        total = total - 1;
       			}
      		}
      		else if(other == 1)
       		{
       			if(numOfOperations_addVertex != 0)
       			{			
				v = vertexID.fetch_add(1);
//				cout << "Vertex " << v << " to be added." << endl;

				res = add_vertex(v);
//				if(res == true)
//					cout << "Vertex " << v << " added." << endl;
//				else
//					cout << "Vertex " << v << " addition failed." << endl;

			        numOfOperations_addVertex = numOfOperations_addVertex - 1;
			        total = total - 1;
        		} 
       		} 
		else if(other == 2)
       		{
       			if(numOfOperations_removeVertex != 0)
        		{		        
			l2:	v = rand() % (vertexID.load());		//dont decrement the total vertex count
				if(v == 0)
					goto l2;
		
//				cout << "Vertex " << v << " to be removed." << endl;

				res = remove_vertex(v);

				if(res == true)
				{
					adjremove(v);
//					cout << "Vertex " << v << " removed." << endl;
//					print_graph();
				}
//				else
//					cout << "Vertex " << v << " removal failed." << endl;

			        numOfOperations_removeVertex = numOfOperations_removeVertex - 1;
			        total = total - 1;
       			}
    		}
	        else if(other == 3) 
		{
     			if(numOfOperations_removeEdge != 0)
       			{	      
		l3:		u = (rand() % (vertexID.load()));		//vertex IDs are from 1
				v = (rand() % (vertexID.load()));
				if(u == v || u == 0 || v == 0)			//simple graph without self loops
					goto l3;
			
//				cout << "Edge (" << u << "," << v << ") to be removed." << endl;
				res = remove_edge(u,v); 
//				if(res == true)
//				{
//					cout << "Edge (" << u << "," << v << ") removed." << endl;
////					print_graph();
//				}
//				else
//					cout << "Edge (" << u << "," << v << ") removal failed." << endl;

		         	numOfOperations_addEdge = numOfOperations_addEdge - 1;
			        total = total - 1;
       			}
      		}
		else if(other == 4)
		{
			if(numOfOperations_containsVertex != 0)
			{
		l4:		u = (rand() % (vertexID.load()));		//vertex IDs are from 1
				if(u == 0)			//simple graph without self loops
					goto l4;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				res = contains_vertex(u); 
//				if(res == true)
//				{
//					cout << "Vertex " << u << " found." << endl;
////					print_graph();
//				}
//				else
//					cout << "Vertex " << u << " not found." << endl;
		
		         	numOfOperations_containsVertex = numOfOperations_containsVertex - 1;				        
				total = total - 1;
			}
		}
		else if(other == 5)
		{
			if(numOfOperations_containsEdge != 0)
			{
		l5:		u = (rand() % (vertexID.load()));		//vertex IDs are from 1
				v = (rand() % (vertexID.load()));
				if(u == v || u == 0 || v == 0)			//simple graph without self loops
					goto l5;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				res = contains_edge(u,v); 
//				if(res == true)
//				{
//					cout << "Edge (" << u << "," << v << ") found." << endl;
//					print_graph();
//				}
//				else
//					cout << "Edge (" << u << "," << v << ") not found." << endl;
		
		         	numOfOperations_containsEdge = numOfOperations_containsEdge - 1;				        
				total = total - 1;
			}
		}
	} 		//end of while loop
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
{						//already locked
	vector<long long int> visited, In_stack;
	NodeList *temp = vhead;
	bool cycle = false;

	while(temp != NULL)
	{
		if(visited.end() == find(visited.begin(), visited.end(), temp->listhead.key))	//not visited
		{
			cycle = DFS_visit(temp, visited, In_stack);
			if(cycle)
				return true;		//dont unlock -> delete vertex
		}
		temp = temp->next;
	}
	return false;
}  
int main(int argc, char*argv[])	//command line arguments - #threads, #vertices initially, #operations per threads
{
	vertexID.store(1);
	int i;

	if(argc < 3)
	{
		cout << "Enter 3 command line arguments - #threads, #vertices initially, #operations per threads" << endl;
		return 0;
	}

	NTHREADS = atoi(argv[1]);
	int initial_vertices = atoi(argv[2]); 		// initial number of vertices
	numOfOperations = atoi(argv[3]);		// number of operations each thread going to perform 1k,10k,50k,100k,1k^2
   	
	//create initial vertices
	vertexID.store(initial_vertices + 1);		// or +1?
	vhead = vtail = NULL;

	create_initial_vertices(initial_vertices);

	cout << "Number of Threads: " << NTHREADS << endl;
	cout << "Initial graph with " << initial_vertices << " created." << endl;

	pthread_t *thr = new pthread_t[NTHREADS];
	// Make threads Joinable for sure.
    	pthread_attr_t attr;
   	pthread_attr_init (&attr);
   	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
	struct timeval tv1, tv2;
	TIME_DIFF * difference;
	int dig,temp; 
	double duration = 0.0;

	gettimeofday(&tv1,NULL);

   	for (i=0;i < NTHREADS;i++)
       	{
       		pthread_create(&thr[i], &attr, pthread_call, (void*) i);
        }

	for (i = 0; i < NTHREADS; i++)
      	{
		pthread_join(thr[i], NULL);
	}

	gettimeofday(&tv2,NULL);

	difference = my_difftime (&tv1, &tv2);
	dig = 1;
	temp = difference->usecs;
	
	while(temp>=10)
	{	
		dig++;
		temp = temp/10;
	}
	temp =1;
	for(i=1;i<=dig;i++)
		temp = temp * 10;
	duration = (double) difference->secs + ((double)difference->usecs / (double)temp);

    	cout << "Duration (gettimeofday() function): " << duration <<" secs."<<endl;

	bool res = cycle_detect();
	if(res == true)
		cout << "CYCLE DETECTED!" << endl;
	return 0;
}
