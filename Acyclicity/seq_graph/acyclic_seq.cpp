#include <list>
#include <stack>
#include <iostream>
#include<float.h>
#include<stdint.h>
#include<stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <ctime>        // std::time
#include <random>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <sys/time.h>
#include <atomic>

//#include "cycle.h"
#include <list>
#include <stack>
#include <iostream>
#define SIZE1 100000000


using namespace std;

typedef struct node{
	int val; // data
	struct node *vnext; // pointer to the next vertex
	struct node *enext; // pointer to the next adjancy list
}slist_t;

typedef struct {
    int     secs;
    int     usecs;
}TIME_DIFF;

TIME_DIFF * my_difftime (struct timeval * start, struct timeval * end){
	TIME_DIFF * diff = (TIME_DIFF *) malloc ( sizeof (TIME_DIFF) );
	if (start->tv_sec == end->tv_sec) {
        	diff->secs = 0;
        	diff->usecs = end->tv_usec - start->tv_usec;
    	}
   	else {
        	diff->usecs = 1000000 - start->tv_usec;
        	diff->secs = end->tv_sec - (start->tv_sec + 1);
        	diff->usecs += end->tv_usec;
        	if (diff->usecs >= 1000000) {
        	    diff->usecs -= 1000000;
	            diff->secs += 1;
	        }
	}
        return diff;
}
             
class listNode{
 public:
        slist_t *Head, *Tail;
        // for initilization of the list
        listNode(){
                Head = (slist_t*) malloc(sizeof(slist_t));
                Head ->val = INT_MIN;
                Head ->vnext = NULL;
                Head ->enext = NULL;
                Tail = (slist_t*) malloc(sizeof(slist_t));
                Tail ->val = INT_MAX;
                Tail ->vnext = NULL;
                Tail ->enext = NULL;
                Head->vnext=Tail;
        }
        slist_t* createV(int key){
                slist_t* EHead = (slist_t*) malloc(sizeof(slist_t));
                EHead ->val = INT_MIN;
                EHead ->vnext = NULL;
                EHead ->enext = NULL;
                slist_t *ETail = (slist_t*) malloc(sizeof(slist_t));
                ETail ->val = INT_MAX;
                ETail ->vnext = NULL;
                ETail ->enext = NULL;
                EHead->enext=ETail;
                slist_t * temp = (slist_t*) malloc(sizeof(slist_t));
                temp ->val = key;
                temp ->vnext = NULL;
                temp ->enext = NULL;
                temp ->enext = EHead;
                return temp;
        }
        slist_t* createE(int key){
                slist_t * temp = (slist_t*) malloc(sizeof(slist_t));
                temp ->val = key;
                temp ->vnext = NULL;
                temp ->enext = NULL;
                return temp;
        }
        void locateV(slist_t ** n1, slist_t ** n2, int key){
                 
                slist_t* pred = Head; 
                slist_t* curr = pred->vnext; 
                while(curr->val < key){
                        pred = curr;
                        curr = curr->vnext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                }
         void locateE(slist_t* Ehead, slist_t ** n1, slist_t ** n2, int key){
                slist_t* pred = Ehead; 
                slist_t* curr = pred->enext; 
                while( curr->val < key){ //curr->enext != NULL &&
                        pred = curr;
                        curr = curr->enext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                 }
                
                       
        bool ContainsV(slist_t ** n, int key){
                slist_t* pred, *curr;
                locateV(&pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return success, true
                }
                else
                        return false; // key not present
                
        }
        bool ContainsE(slist_t *Ehead, slist_t ** n, int key){
                slist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return success, true
                }
                else
                        return false; // key not present
                
        }
        
        bool AddV(int key){
                slist_t* pred, *curr;
                locateV(&pred, &curr, key);
                slist_t *newv = createV(key);
                if(curr->val != key){
                        newv ->vnext = curr;   
                        pred ->vnext = newv;
                        return true;
                }
                else
                 return false;
               }
               
       bool AddE(slist_t *Ehead, int key){
                slist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                slist_t *newe = createE(key);
                if(curr->val != key){
                        newe ->enext = curr;   
                        pred ->enext = newe;
                        return true;
                }
                else
                 return false;
          }  
   
  
        bool RemV(int key){
                slist_t* pred, *curr;
                locateV(&pred, &curr, key);
                
                if(curr->val == key){
                        pred ->vnext = curr->vnext;
                        curr->vnext=NULL;
                        free(curr);
                        return true;
                }
                else
                 return false;
               }
               
       bool RemE(slist_t *Ehead, int key){
                slist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                if(curr->val == key){
                        pred ->enext = curr->enext;
                        curr->enext = NULL;
                        free(curr);
                       
                        return true;
                }
                else
                 return false;
          }  
      /* cycle detection from the vertex u to v. The vertices whchi are reachable from v to u */  
      bool isReachable(slist_t *u, slist_t *v, int n){
        slist_t * temp, *it;
        if(u->val == v->val) // base case
          return true;
        bool *  visited = new bool[n+1];
        for(int i = 0; i <=n; i++) 
                visited[i] = false;
        list<int> que;
        visited[u->val] = true;
        que.push_back(u->val);
        while(!que.empty()){
            int s = que.front();
                que.pop_front();
            bool flag1 = ContainsV(&temp, s);  
          if(flag1 == true){
               temp = temp->enext; // Ehead
                for(it = temp->enext; it->enext != NULL; it = it ->enext){
                        if(it->val == v->val) 
                                return true; // reachable
                        if(!visited[it->val]){
                                visited[it->val] = true;
                                que.push_back(it->val);
                        }
                    }    
                 }
                }
        return false;                
     }
     
     /* cycle detection for whole graph, from Head to Tail*/
      bool testCycle(int n){
        slist_t * temp=Head->vnext, *temp1, *it;
         while(temp->vnext != NULL){
            temp1 = temp->vnext;
           while(temp1->vnext != NULL){
              if(isReachable(temp1, temp, n)) {
               //cout<<" cycle:"<<temp1->val<<" "<<temp->val<<endl;
                return true; // cycle is present
               } 
             temp1 = temp1->vnext;   
          }
          temp = temp->vnext;
         }
  return false;                
 }
 
 
};
 class Graph{
    listNode lt;
 public:      
   
      Graph(){
        //lt.init();
      }
        
       // lt.init();
        bool AddGV(int key){
           return lt.AddV(key);
        }
        bool RemoveGV(int key){
                 return lt.RemV(key);
        /*
               // DIE  
              if(lt.RemV(key)){
                  slist_t *temp1 = lt.Head->vnext;
                  while(temp1 != lt.Tail){
                    lt.Remove(temp1->enext, NULL, key);
                    temp1 = temp1->vnext;
                }
                return true;
               } 
               else
                  return false; 
               */  
        }
        bool ContainsGV(int key){
                 slist_t *g1;
                 if(lt.ContainsV(&g1, key)){
                        //(*n) = &g1;                  
                   return true;
                   }
                 else
                    return false;  
        }
        
        bool RemoveGE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.ContainsV(&u, key1);
          bool flag2 = lt.ContainsV(&v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.ContainsV(&u, key1);  
            if(flag1 == false){
            return false;
            }
          return lt.RemE(u->enext, key2);  
         }
        bool ContainsGE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.ContainsV(&u, key1);
          bool flag2 = lt.ContainsV(&v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          return lt.ContainsE(u->enext, &v, key2);  
        }
         bool AddGE(int key1, int key2, int n){
          slist_t *u,*v;
          bool flag1 = lt.ContainsV(&u, key1);
          bool flag2 = lt.ContainsV(&v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.ContainsV(&u, key1);  
          if(flag1 == false)
            return false;
          //return lt.Add(u->enext,NULL, key2);  // with out cycle detection
          
          // cycle detection  and then delete the edge
          bool status = lt.AddE(u->enext, key2);  
            
            if(status == true){
             bool c = lt.isReachable(v, u,  n);
              if(c == true){
                status =  RemoveGE(key1, key2);
      
           }
           }
           return status; 
        }
    void PrintGraph(){
	slist_t *temp1 = lt.Head;
	slist_t *temp2;
	while(temp1 != NULL){
		cout << temp1->val << "->";
		temp2 = temp1->enext;
		while(temp2 != NULL)
		{
			cout << temp2->val << " ";
			temp2 = temp2->enext;
		}
		cout << endl;
		temp1 = temp1->vnext;
	   }
        }
        void initGraph(int n){
          int i=1;
          for(;i<=n;i++){
            AddGV(i);
          }
        
        }
       bool checkCycle(int n){
         return lt.testCycle(n);
       } 
     
   };     
   



