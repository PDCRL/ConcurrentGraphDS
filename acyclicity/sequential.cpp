#include <stdio.h>
#include <iostream>
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

long long int vertexID;
int NTHREADS, numOfOperations;

struct Node
{
	long long int key;
//	atomic<bool> marked;
	Node *next;
};

struct NodeList
{
	Node listhead;
	NodeList *next;
};

NodeList *graph;

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
	int i;
	for(i=1;i<=initial_vertices;i++)
	{
		NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));

		newlisthead->listhead.key = i;
		newlisthead->listhead.next = NULL;
//		newlisthead->listhead.marked = false;
		newlisthead->next = NULL;
		if(i==1)
		{
			graph = newlisthead;
		}
		else
		{
			NodeList *temp = graph;
			while(temp->next != NULL)
				temp = temp->next;
			temp->next = newlisthead;
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

NodeList* findNode(long long int key)
{
	NodeList *temp = graph;
	while(temp!= NULL)
	{
		if(temp->listhead.key == key)
			break;
		temp = temp->next;
	}
	return temp;
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
	NodeList *temp = graph;
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

int add_vertex(long long int v)
{
	NodeList *newlisthead = (NodeList*) malloc(sizeof(NodeList));
	newlisthead->listhead.key = v;
	newlisthead->listhead.next = NULL;
//	newlisthead->listhead.marked = false;
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
	
	while(curr->listhead.key < v && curr->next != NULL)		//if pred is greater than v?
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
	// remove that vertex in all adj list also?
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
		return true;		//if true then dont unlock
	}

	if(pred->key == v)		//edge already present
		return true;

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

	if(pred->key == v)		//edge already present
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
	if(curr->key == v)		//edge already present
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

	//found both u,v in graph - now delete edge

	Node *pred = temp->listhead.next;
	if(pred == NULL)	
		return false;

	if(pred->key == v)		//edge already present
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
	if(curr->key == v)		//edge already present
		return true;
	else
		return false;
}

void pthread_call()
{
	long long int u, v;
	int other, res;
  	int numOfOperations_addEdge = numOfOperations * 0.25; 		// 20% for add edge
  	int numOfOperations_addVertex = numOfOperations * 0.25; 	// 15% for add vertex
  	int numOfOperations_removeVertex = numOfOperations * 0.1; 	// 15% for remove vertex
  	int numOfOperations_removeEdge = numOfOperations * 0.1; 	// 15% for remove vertex
  	int numOfOperations_containsVertex = numOfOperations * 0.15; 	// 15% for remove vertex
  	int numOfOperations_containsEdge = numOfOperations * 0.15; 	// 15% for remove vertex
	int total = numOfOperations_addEdge + numOfOperations_addVertex + numOfOperations_removeVertex + numOfOperations_removeEdge + numOfOperations_containsVertex + numOfOperations_containsEdge; 
	
	while(total != 0)
	{
		int other=rand()%6;				//use exponential distribution here
	        if(other == 0) 
		{
			if(numOfOperations_addEdge != 0)
   			{	      
		l1:		u = (rand() % (vertexID));		//vertex IDs are from 1
				v = (rand() % (vertexID));
				if(u == v || u == 0 || v == 0)			//simple graph without self loops
					goto l1;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				res = add_edge(u,v); 
				if(res == true)
				{
//					cout << "Edge (" << u << "," << v << ") added." << endl;
//					print_graph();
					res = cycle_detect(v,u);
					if(res == true)
					{
						res = remove_edge(u,v);
//						if(res == true)
//							cout << "Edge (" << u << "," << v << ") removed." << endl;						
/*						res = remove_vertex(v);
						if(res == true)
						{
							adjremove(v);							
							cout << "Vertex " << v << " removed." << endl;
//						}
*/					}
//					cout << "Cycle detect Done." << endl;
				}
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
				v = vertexID;		//vertices do not come again
				vertexID++;
//				cout << "Vertex " << v << " to be added." << endl;
				
				res = add_vertex(v);
//				print_graph();
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
			l2:	v = rand() % (vertexID);		//dont decrement the total vertex count
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
		l3:		u = (rand() % (vertexID));		//vertex IDs are from 1
				v = (rand() % (vertexID));
				if(u == v || u == 0 || v == 0)			//simple graph without self loops
					goto l3;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				res = remove_edge(u,v); 
//				if(res == true)
//				{
//					cout << "Edge (" << u << "," << v << ") removed." << endl;
//					print_graph();
//				}
//				else
//					cout << "Edge (" << u << "," << v << ") not removed." << endl;
		
		         	numOfOperations_removeEdge = numOfOperations_removeEdge - 1;				        
				total = total - 1;
			}
		}
		else if(other == 4)
		{
			if(numOfOperations_containsVertex != 0)
			{
		l4:		u = (rand() % (vertexID));		//vertex IDs are from 1
				if(u == 0)			//simple graph without self loops
					goto l4;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				res = contains_vertex(u); 
//				if(res == true)
//				{
//					cout << "Vertex " << u << " found." << endl;
//					print_graph();
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
		l5:		u = (rand() % (vertexID));		//vertex IDs are from 1
				v = (rand() % (vertexID));
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

int main(int argc, char*argv[])	//command line arguments - #threads, #vertices initially, #operations per threads
{
	vertexID = 1;
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
	vertexID = initial_vertices + 1;		// or +1?
	graph = NULL;

	create_initial_vertices(initial_vertices);

	cout << "Number of Threads: " << NTHREADS << endl;
	cout << "Initial graph with " << initial_vertices << " created." << endl;

	struct timeval tv1, tv2;
	TIME_DIFF * difference;
	int dig,temp; 
	double duration = 0.0;

	gettimeofday(&tv1,NULL);

	pthread_call();

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

	//sequential cycle detection
	NodeList *temp1 = graph;
	while(temp1 != NULL)
	{
		bool res = cycle_detect();
		if(res == true)
			cout << "CYCLE DETECTED!" << endl;
		temp1 = temp1->next;
	}
	return 0;
}
