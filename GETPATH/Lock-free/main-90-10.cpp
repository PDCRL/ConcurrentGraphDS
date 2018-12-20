 /*
 * File:main-90-10.cpp
 *  
 *
 * Author(s):
 *   Bapi Chatterjee <bapchatt@in.ibm.com>
 *   Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *   
 * Description:
 *    implementation of a Coarse Graph GetPath
 * Copyright (c) 2018.
 * last Updated: 31/07/2018
 *
*/
#include"lfGraph_getPath.cpp"

 //ofstream couttt("graphinput.txt");
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


atomic<long> vertexID;
double seconds;
struct timeval tv1, tv2;
TIME_DIFF * difference;
int NTHREADS, ops;

typedef struct infothread{
  long tid;
  slist G;
}tinfo;


void* pthread_call(void* t)
{
        tinfo *ti=(tinfo*)t;
        long Tid = ti->tid;
        slist G1=ti->G;
	int u, v;;
	while(true)
	{
		gettimeofday(&tv2,NULL);
		difference = my_difftime (&tv1, &tv2);

		if(difference->secs >= seconds)
			break;

		int op = rand()%1000;	
	        if(op >= 0  && op < 20 ) 
		{
		l1:		u = rand()% vertexID +1;		
				v = rand()% vertexID +1;
				if(u == v || u == 0 || v == 0)			
					goto l1;
				G1.GetPath(u,v, Tid); 
 				ops++;				
      		}
      		else if(op >= 20 && op < 40 )
       		{
     				vertexID++;
				l:	v = rand() % (vertexID);		
				if(v == 0)
					goto l;
				G1.AddV(v,NTHREADS);
				ops++;				
			 
       		} 
 
	     	else if(op >= 40 && op <60 )
     		{
       			l2:	v = rand() % (vertexID)+1;		
				if(v == 0)
					goto l2;
				G1.RemoveV(v);
        			ops++;				
       		}
		else if(op >= 60 && op <80 )
		{
		l3:		u = (rand() % (vertexID))+1;		
				v = (rand() % (vertexID))+1;
				if(u == v || u == 0 || v == 0)	
					goto l3;
				G1.RemoveE(u,v); 
				ops++;				
		}
		else if(op >= 80 && op <100 )
		{
		l4:		u = (rand() % (vertexID))+1;	
				v = (rand() % (vertexID))+1;
				if(u == v || u == 0 || v == 0)	
					goto l4;
				G1.AddE(u,v); 
				ops++;				
		}
		else if(op >= 100 && op <550 )
		{
		l5:		u = (rand() % (vertexID))+1;		
				v = (rand() % (vertexID))+1;
				if(u == v || u == 0 || v == 0)		
					goto l5;
				G1.ContainsE(u,v); 
				ops++;				
		}
		else if(op >= 550 && op < 1000 )
		{
		
		l6:		v = rand() % (vertexID);	
				if(v == 0)
					goto l6;
				G1.ContainsV(v);
        			ops++;		
		}
	} 		
}

int main(int argc, char*argv[])	
{
	slist sg;
	vertexID.store(1);
	int i;
	if(argc < 3)
	{
		cout << "Enter 3 command line arguments - #threads, #vertices initially, #time in seconds" << endl;
		return 0;
	}
	NTHREADS = atoi(argv[1]);
	int initial_vertices = atoi(argv[2]); 
	seconds = atoi(argv[3]);		
   	ops = 0;
	vertexID.store(initial_vertices + 1);	
        sg.init();
	sg.initGraph(initial_vertices, NTHREADS);
	cout << "Number of Threads: " << NTHREADS << endl;
	cout << "Initial graph with " << initial_vertices << " created." << endl;
        pthread_t *thr = new pthread_t[NTHREADS];
	pthread_attr_t attr;
   	pthread_attr_init (&attr);
   	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
   	int dig,temp; 
	double duration = 0.0;
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
	return 0;
}

