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
std :: list<int> adj[SIZE1];
typedef struct node{
	int val; // data
	struct node *vnext; // pointer to the next vertex
	struct node *enext; // pointer to the next adjancy list
}slist_t;

typedef struct cyclevisit1{
         int key;
         bool vit;
      }cyclevisit;
      
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

       

     
               
class listNode{
 public:
        slist_t *Head, *Tail;
        // for initilization of the list
        void init(){
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
        void locate(slist_t* head, slist_t* tail, slist_t ** n1, slist_t ** n2, int key){
                  //         cout<<"loc begins"<<endl;
                slist_t* pred = head; 
                slist_t* curr = pred->vnext; 
                if(head->vnext == tail){
                        (*n1) = head;
                        (*n2) = tail;
                }
                if(head == Head){
                while(curr != tail && curr->val < key){
                        pred = curr;
                        curr = curr->vnext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                }
                else
                {       curr = pred->enext; 
                        while(curr != tail && curr->val < key){
                        pred = curr;
                        curr = curr->enext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                }
        }
        bool Contains(slist_t * h, slist_t *t, slist_t ** n, int key){
           //cout<<"con begins"<<endl;
                slist_t* pred, *curr;
               
                locate(h, t, &pred, &curr, key);
                 //if(h==Head){
                /*if(Head->vnext == Tail){
                        return false;
                }*/
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return success, true
                }
                else
                        return false; // key not present
                
        }
        bool Add(slist_t * h, slist_t *t, int key){
                // allcate new node
                slist_t* pred, *curr;
                locate(h, t, &pred, &curr, key);
                if(h == Head){
                slist_t *newv = createV(key);
                if(curr->val != key){
                        newv ->vnext = curr;   
                        pred ->vnext = newv;
                        return true;
                }
                else
                 return false;
               }
               else{
               slist_t *newe = createE(key);
                if(curr->val != key){
                        newe ->enext = curr;   
                        pred ->enext = newe;
                        return true;
                }
                else
                 return false;
          }  
   }

    bool Remove(slist_t *h, slist_t *t, int key){
        slist_t* pred, *curr;
        locate(h, t, &pred, &curr, key);
        if(h==Head){
                if(curr->val == key){
                        pred ->vnext = curr->vnext;
                        return true;
                }
                else
                        return false;
        }
         else{
         if(curr->val == key){
                        pred ->enext = curr->enext;
                        return true;
                }
                else
                        return false;
         }
        } 
    
        void print(){
                slist_t* pred = Head; 
                slist_t* curr = pred->vnext; 
                if(Head->vnext==Tail){
                        cout<<"Head("<<Head->val<<")->";
                        cout<<Tail->val<<"Tail"<<endl;
                        return;
                }
                cout<<"Head("<<Head->val<<")->";      
                while(curr != Tail){
                        cout<<curr->val<<"->";
                        pred = curr;
                        curr = curr->vnext;
                }
                cout<<Tail->val<<"Tail"<<endl;
         return;
        }
        
         bool checkVisited(list <cyclevisit> l, int k){
                list <cyclevisit> :: iterator it;
                for(it = l.begin(); it != l.end(); ++it){
                     if(it->key == k && it->vit == true){
                       return true;
                    }
                 }      
        return false;
      }
      bool isReachable(slist_t *u, slist_t *v){
        slist_t * temp, *temp2, *it;
        if(u->val == v->val) // base case
          return true;
        list <cyclevisit>  visited;
        //bool *visited = new bool[n+1];
        //for(int i = 0; i <=n; i++) 
        //        visited[i] = false;
      //  if (!is_marked_ref((long) u) && !is_marked_ref((long) v)){
        list<int> que;
        cyclevisit item;
        item.key = u->val;
        item.vit = true;
        visited.push_back(item);
        //cout<<u->val;
       // visited[u->val] = true;
        que.push_back(u->val);
        while(!que.empty()){
            int s = que.front();
                que.pop_front();
            bool flag1 = Contains(Head, Tail, &temp, s);  
          if(flag1 == true){
               temp = temp->enext; // Ehead
                for(it = temp->enext; it->enext != NULL; it = it ->enext){
                   bool flag2 = Contains(Head, Tail, &temp2, it->val);  
                   if(flag2 == true ){ // check for vertex is present in the list
                        if(it->val == v->val) 
                                return true; // reachable
                        bool flag3 = checkVisited(visited, it->val);        
                        //if(!visited[it->val]){
                        if(flag3 == false){
                                cyclevisit item1;
                                item1.key = it->val;
                                item.vit = true;
                                visited.push_back(item1);
                               // visited[it->val] = true;
                                que.push_back(it->val);
                        }
                    } 
                    }  
                   //}  
                 //}
                }
              }  
        return false;                
     }
     
     
     
     /* cycle detection for whole graph, from Head to Tail*/
      bool isCycle(){
        slist_t * temp=Head->vnext, *temp1, *it, *curr;
         while(temp!=Tail){
            temp1 = temp->vnext;
           while(temp1 != Tail){
              
              if(isReachable(temp1, temp))
               if(Contains(temp->enext, NULL,  &curr, temp1->val )){
               cout<<"cycle"<<endl;
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
        lt.init();
      }
        
       // lt.init();
        bool AddV(int key){
           return lt.Add(lt.Head,lt.Tail,key);
        }
        bool RemoveV(int key){
                return lt.Remove(lt.Head, lt.Tail, key);
        /*
                if(lt.Remove(lt.Head, lt.Tail, key)){
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
        bool ContainsV(int key){
                 slist_t *g1;
                 if(lt.Contains(lt.Head, lt.Tail, &g1, key)){
                        //(*n) = &g1;                  
                   return true;
                   }
                 else
                    return false;  
        }
        
       
        //void locateV(slist_t ** n1, slist_t ** n2, int key);
        bool AddE(int key1, int key2){
          slist_t *u,*v;
          bool flag1 = lt.Contains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.Contains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.Contains(lt.Head, lt.Tail, &u, key1);  
          if(flag1 == false)
            return false;
          //return lt.Add(u->enext,NULL, key2);  
          bool status = lt.Add(u->enext,NULL, key2);  
           
            if(status == true){
         
              bool c = lt.isReachable(v, u);
              if(c == true){
                status =  RemoveE(key1, key2);
               
           }
           }
           return status; 
        }

        bool RemoveE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.Contains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.Contains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.Contains(lt.Head, lt.Tail, &u, key1);  
          if(flag1 == false)
            return false;
          return lt.Remove(u->enext,NULL, key2);  
         }
        bool ContainsE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.Contains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.Contains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          return lt.Contains(u->enext,NULL, &v, key2);  
        
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
            AddV(i);
          }
        
        }
       bool checkCycle(){
         return lt.isCycle();
       } 
     
   };     
   



