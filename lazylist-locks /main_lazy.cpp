/*
main_seq.cpp:
 *
 * Author(s):
 *     
 * Description:
 *   Coarse-grain locking implementation of a acycle graph
 * Copyright (c) 2017.
 * last Updated: 28/10/2017
 *
*/


#include<iostream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <pthread.h>


//#include"acyclic_lazy.cpp"
#include"cgds_lazy.cpp"

using namespace std;


FILE *fp;

 int vertexID;
double seconds;
struct timeval tv1, tv2;
TIME_DIFF * difference;
int NTHREADS, ops;
int  total = 0, total1 = 0;
enum type {ADDV, ADDE, REMV, REME, CONV, CONE};
int optype; // what type of opearations 
char out[30]; // dataset file name,
int naddV=0, naddE=0, nremV=0, nremE=0, nconV=0, nconE=0; 
pthread_mutex_t lock;
typedef struct infothread{
  long tid;
  Graph G;
}tinfo;

void* pthread_call(void* t)
{
        tinfo *ti=(tinfo*)t;
        long Tid = ti->tid;
        Graph G1=ti->G;
	int u, v;
	int other, res;
 
 	long long int numOfOperations = 10000000000;
 	long long int numOfOperations_addEdge = numOfOperations * 0.25; 		// 25% for add edge
  	long long int numOfOperations_addVertex = numOfOperations * 0.25; 	// 25% for add vertex
  	long long int numOfOperations_removeVertex = numOfOperations *0.1 ; 	// 10% for remove vertex
  	long long int numOfOperations_removeEdge = numOfOperations * 0.1; 	// 10% for remove edge
  	long long int numOfOperations_containsVertex = numOfOperations *0.15; 	// 15% for contains vertex
  	long long int numOfOperations_containsEdge = numOfOperations * 0.15; 	// 15% for contains edge

	long long int total = numOfOperations_addEdge + numOfOperations_addVertex + numOfOperations_removeVertex + numOfOperations_removeEdge + numOfOperations_containsVertex + numOfOperations_containsEdge; 
	
	while(total > 0)
	{
		gettimeofday(&tv2,NULL);
		difference = my_difftime (&tv1, &tv2);

		if(difference->secs >= seconds)
			break;

		int other=rand()%6;	
	        if(other == 0) 
		{
			if(numOfOperations_addEdge > 0)
   			{	      
		l1:		u = (rand() % (vertexID));		//vertex IDs are from 1
				v = (rand() % (vertexID));
				if(u == v || u == 0 || v == 0)			//simple graph without self loops
					goto l1;
				res = G1.AddE(u,v); 
		         	numOfOperations_addEdge = numOfOperations_addEdge - 1;				        
				total = total - 1;
 				ops++;				
      			}
      		}
      		else if(other == 1)
       		{
     			if(numOfOperations_addVertex > 0)
        		{			
				v = vertexID;		//vertices do not come again
				vertexID++;
				res = G1.AddV(v);
			        numOfOperations_addVertex = numOfOperations_addVertex - 1;
				ops++;				
			        total = total - 1;
        		} 
       		} 
	     	else if(other == 2)
     		{
       			if(numOfOperations_removeVertex > 0)
       			{		        
			l2:	v = rand() % (vertexID);		//dont decrement the total vertex count
				if(v == 0)
					goto l2;
				res = G1.RemoveV(v);
			        numOfOperations_removeVertex = numOfOperations_removeVertex - 1;
				ops++;				
			        total = total - 1;
        		} 
       		}
		else if(other == 3)
		{
			if(numOfOperations_removeEdge > 0)
			{
		l3:		u = (rand() % (vertexID));		//vertex IDs are from 1
				v = (rand() % (vertexID));
				if(u == v || u == 0 || v == 0)	
					goto l3;
				res = G1.RemoveE(u,v); 
		         	numOfOperations_removeEdge = numOfOperations_removeEdge - 1;				        
				ops++;				
				total = total - 1;
			}
		}
		else if(other == 4)
		{
			if(numOfOperations_containsVertex > 0)
			{
		l4:		u = (rand() % (vertexID));		//vertex IDs are from 1
				if(u == 0)	
					goto l4;
				res = G1.ContainsV(u); 
		         	numOfOperations_containsVertex = numOfOperations_containsVertex - 1;				        
				ops++;				
				total = total - 1;
			}
		}
		else if(other == 5)
		{
			if(numOfOperations_containsEdge > 0)
			{
		l5:		u = (rand() % (vertexID));		//vertex IDs are from 1
				v = (rand() % (vertexID));
				if(u == v || u == 0 || v == 0)		
					goto l5;
				res = G1.ContainsE(u,v); 
		         	numOfOperations_containsEdge = numOfOperations_containsEdge - 1;				        
				ops++;				
				total = total - 1;
			}
		}
	} 		//end of while loop
}

int main(int argc, char*argv[])
{
	vertexID = 1;
	int i;
Graph sg;
	if(argc < 3)
	{
		cout << "Enter 3 command line arguments - #threads, #vertices initially, #time in seconds" << endl;
		return 0;
	}

	NTHREADS = atoi(argv[1]);
	int n = atoi(argv[2]); 		// initial number of vertices
	seconds = atoi(argv[3]);
			// number of operations each thread going to perform 1k,10k,50k,100k,1k^2
			
 	//ops = 0;
//strcpy(out,argv[4]);
	
        pthread_mutex_init(&lock, NULL);
	//create initial vertices
	vertexID = n+ 1;	
	sg.initGraph(n);
//sg.PrintGraph();
	cout << "Number of Threads: " << NTHREADS << endl;
	cout << "Initial graph with " << n << " created." << endl;
 pthread_t *thr = new pthread_t[NTHREADS];
	// Make threads Joinable for sure.
    	pthread_attr_t attr;
   	pthread_attr_init (&attr);
   	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
   	
	gettimeofday(&tv1,NULL);
	cout << "timer started . . ." << endl;
        for (i=0;i < NTHREADS;i++)
       	{
       	      tinfo *t =(tinfo*) malloc(sizeof(tinfo));
		t->tid = i;
		t->G = sg;
       		pthread_create(&thr[i], &attr, pthread_call, (void*)t);
        }

	for (i = 0; i < NTHREADS; i++)
      	{
		pthread_join(thr[i], NULL);
	}
	cout << seconds <<  " seconds elapsed" << endl;

    	cout << "Total operations: " << ops <<endl;
        gettimeofday(&tv2,NULL);
	difference = my_difftime (&tv1, &tv2);
//sg.PrintGraph();
/*
  bool cycle = sg.checkCycle();
    if(cycle == true)
      cout<<"cycle is present"<<endl;
    else
      cout<<"cycle is not present"<<endl;  
 */     
 return 0;
}

