/*
 * File:
 *   coarse-lock.cpp
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
 * coarse-lock.cpp is part of ConcurrentGraphDS
*/


#include "coarse-lock.h"

atomic<long long int> vertexID;
int NTHREADS, numOfOperations;

void* pthread_call(void *t)
{
	long tid=(long)t;

	long long int u, v;
	int other, res;
  	int numOfOperations_addEdge = numOfOperations * 0.25; 		// 25% for add edge
  	int numOfOperations_addVertex = numOfOperations * 0.25; 	// 25% for add vertex
  	int numOfOperations_removeVertex = numOfOperations * 0.1; 	// 10% for remove vertex
  	int numOfOperations_removeEdge = numOfOperations * 0.1; 	// 10% for remove edge
  	int numOfOperations_containsVertex = numOfOperations * 0.15; 	// 15% for contains vertex
  	int numOfOperations_containsEdge = numOfOperations * 0.15; 	// 15% for contains edge

	int total = numOfOperations_addEdge + numOfOperations_addVertex + numOfOperations_removeVertex + numOfOperations_removeEdge + numOfOperations_containsVertex + numOfOperations_containsEdge; 
	
	while(total > 0)
	{
		int other=rand()%6;	
	        if(other == 0) 
		{
			if(numOfOperations_addEdge > 0)
   			{	      
		l1:		u = (rand() % (vertexID.load()));		//vertex IDs are from 1
				v = (rand() % (vertexID.load()));
				if(u == v || u == 0 || v == 0)			//simple graph without self loops
					goto l1;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				pthread_mutex_lock(&lock);

				res = add_edge(u,v); 
				if(res == true)
				{
//					cout << "Edge (" << u << "," << v << ") added." << endl;
//					print_graph();
					res = cycle_detect(v, u);
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

				pthread_mutex_unlock(&lock);

		         	numOfOperations_addEdge = numOfOperations_addEdge - 1;				        
				total = total - 1;
       			}
      		}
      		else if(other == 1)
       		{
     			if(numOfOperations_addVertex > 0)
        		{			
				v = vertexID.fetch_add(1);		//vertices do not come again
//				cout << "Vertex " << v << " to be added." << endl;
				
				pthread_mutex_lock(&lock);

				res = add_vertex(v);
//				print_graph();
//				if(res == true)
//					cout << "Vertex " << v << " added." << endl;
//				else
//					cout << "Vertex " << v << " addition failed." << endl;

				pthread_mutex_unlock(&lock);
			        numOfOperations_addVertex = numOfOperations_addVertex - 1;
			        total = total - 1;
        		} 
       		} 
	     	else if(other == 2)
     		{
       			if(numOfOperations_removeVertex > 0)
       			{		        
			l2:	v = rand() % (vertexID.load());		//dont decrement the total vertex count
				if(v == 0)
					goto l2;
//				cout << "Vertex " << v << " to be removed." << endl;

				pthread_mutex_lock(&lock);

				res = remove_vertex(v);
				if(res == true)
				{
					adjremove(v);
//					cout << "Vertex " << v << " removed." << endl;
//					print_graph();
				}
//				else
//					cout << "Vertex " << v << " removal failed." << endl;

				pthread_mutex_unlock(&lock);
			
			        numOfOperations_removeVertex = numOfOperations_removeVertex - 1;
			        total = total - 1;
        		} 
       		}
		else if(other == 3)
		{
			if(numOfOperations_removeEdge > 0)
			{
		l3:		u = (rand() % (vertexID.load()));		//vertex IDs are from 1
				v = (rand() % (vertexID.load()));
				if(u == v || u == 0 || v == 0)	
					goto l3;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				pthread_mutex_lock(&lock);

				res = remove_edge(u,v); 
//				if(res == true)
//				{
//					cout << "Edge (" << u << "," << v << ") removed." << endl;
//					print_graph();
//				}
//				else
//					cout << "Edge (" << u << "," << v << ") not removed." << endl;
		
				pthread_mutex_unlock(&lock);

		         	numOfOperations_removeEdge = numOfOperations_removeEdge - 1;				        
				total = total - 1;
			}
		}
		else if(other == 4)
		{
			if(numOfOperations_containsVertex > 0)
			{
		l4:		u = (rand() % (vertexID.load()));		//vertex IDs are from 1
				if(u == 0)	
					goto l4;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				pthread_mutex_lock(&lock);

				res = contains_vertex(u); 
//				if(res == true)
//				{
//					cout << "Vertex " << u << " found." << endl;
//					print_graph();
//				}
//				else
//					cout << "Vertex " << u << " not found." << endl;
		
				pthread_mutex_unlock(&lock);

		         	numOfOperations_containsVertex = numOfOperations_containsVertex - 1;				        
				total = total - 1;
			}
		}
		else if(other == 5)
		{
			if(numOfOperations_containsEdge > 0)
			{
		l5:		u = (rand() % (vertexID.load()));		//vertex IDs are from 1
				v = (rand() % (vertexID.load()));
				if(u == v || u == 0 || v == 0)
					goto l5;
			
//				cout << "Edge (" << u << "," << v << ") to be added." << endl;
			
				pthread_mutex_lock(&lock);

				res = contains_edge(u,v); 
//				if(res == true)
//				{
//					cout << "Edge (" << u << "," << v << ") found." << endl;
//					print_graph();
//				}
//				else
//					cout << "Edge (" << u << "," << v << ") not found." << endl;
		
				pthread_mutex_unlock(&lock);

		         	numOfOperations_containsEdge = numOfOperations_containsEdge - 1;				        
				total = total - 1;
			}
		}
	} 		//end of while loop
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
				return true;	
		}
		temp = temp->next;
	}
	return false;
}   

int main(int argc, char*argv[])		//command line arguments - #threads, #vertices initially, #operations per threads
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
	pthread_mutex_init(&lock, NULL);
	
	vertexID.store(initial_vertices + 1);
	graph = NULL;

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

	NodeList *temp1 = graph;
	while(temp1->next != NULL)
	{
		NodeList *temp2 = temp1->next;
		assert(temp1->listhead.key < temp2->listhead.key);

		if(temp1->listhead.next == NULL)
		{
			temp1 = temp1->next;
			continue;
		}
		Node *temp3 = temp1->listhead.next;
		while(temp3->next != NULL)
		{
			Node *temp4 = temp3->next;
			assert(temp3->key < temp4->key);
			temp3 = temp3->next;
		}
		temp1 = temp1->next;
	}

	//sequential cycle detection
	temp1 = graph;
	while(temp1 != NULL)
	{
		bool res = cycle_detect();
		if(res == true)
			cout << "CYCLE DETECTED!" << endl;
		temp1 = temp1->next;
	}
	return 0;
}
