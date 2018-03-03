/*
 * File:acyclic_lf.cpp
 *  
 *
 * Author(s):
 *   Dr. Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *   
 * Description:
 *   non-blocking implementation of a graph
 * Copyright (c) 2017.
 * last Updated: 17/10/2017
 *
*/

#ifndef LLIST_H_ 
#define LLIST_H_

#include<iostream>
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
#include <list>
//#include <stdatomic.h>


#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
#endif


using namespace std;

      
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




inline long is_marked_ref(long i){
  return (long) (i & 0x1L);
}

inline long is_marked_ref2(long i){
  return (long) (i & 0x2L);
}

inline long unset_mark(long i){
  i &= ~0x1L;
  return i;
}

inline long unset_mark2(long i){
  i &= ~0x2L;
  return i;
}

inline long set_mark(long i){
  i |= 0x1L;
  return i;
}
inline long set_mark2(long i){
  i |= 0x2L;
  return i;
}

inline long get_unmarked_ref(long w){
  return w & ~0x1L;
}
inline long get_unmarked_ref2(long w){
  return w & ~0x2L;
}

inline long get_marked_ref(long w){
  return w | 0x1L;
}
inline long get_marked_ref2(long w){
  return w | 0x2L;
}


typedef struct node{
	int val; // data
	atomic<struct node *> vnext; // pointer to the next entry
	atomic<struct node *>enext; // pointer to the next adjancy list
}slist_t;

typedef struct cyclevisit1{
         int key;
         bool vit;
      }cyclevisit;

class list1{
 public:
 
    slist_t *Head, *Tail;
  // for initilization of the list
 void init(){
  Head = (slist_t*) malloc(sizeof(slist_t));
  Head ->val = INT_MIN;
  Head ->vnext.store(NULL,memory_order_seq_cst);
  Head ->enext.store(NULL,memory_order_seq_cst);
  Tail = (slist_t*) malloc(sizeof(slist_t));
  Tail ->val = INT_MAX;
  Tail ->vnext.store(NULL,memory_order_seq_cst);
  Tail ->enext.store(NULL,memory_order_seq_cst);
  Head->vnext.store(Tail,memory_order_seq_cst);
  cout<<"init!!"<<endl;
}
        slist_t* createV(int key){
                slist_t* EHead = (slist_t*) malloc(sizeof(slist_t));
                EHead ->val = INT_MIN;
                EHead ->vnext.store(NULL,memory_order_seq_cst);
                EHead ->enext.store(NULL,memory_order_seq_cst);
                slist_t *ETail = (slist_t*) malloc(sizeof(slist_t));
                ETail ->val = INT_MAX;
                ETail ->vnext.store(NULL,memory_order_seq_cst);
                ETail ->enext.store(NULL,memory_order_seq_cst);
                EHead->enext.store(ETail,memory_order_seq_cst);
                slist_t * temp = (slist_t*) malloc(sizeof(slist_t));
                temp ->val = key;
                temp ->vnext.store(NULL,memory_order_seq_cst);
                temp ->enext.store(NULL,memory_order_seq_cst);
                temp ->enext.store(EHead,memory_order_seq_cst);
                return temp;
        }
        slist_t* createE(int key){
                slist_t * temp = (slist_t*) malloc(sizeof(slist_t));
                temp ->val = key;
                temp ->vnext.store(NULL,memory_order_seq_cst);
                temp ->enext.store(NULL,memory_order_seq_cst);
                return temp;
        }
       
        
         void locate(slist_t* head, slist_t* tail, slist_t ** n1, slist_t ** n2, int key){
         slist_t *succ, *curr, *pred;
         if(head == Head){ // locate in the vertex list
         retry:
	do{
		slist_t *t = Head;
		slist_t *t_next = t->vnext.load(memory_order_seq_cst);
		/* Find pred and curr */
		do{
			if (!is_marked_ref((long) t_next)) {
				pred = t;
				succ = t_next;
			}
			t = (slist_t *) get_unmarked_ref((long)t_next);
			if (!t->vnext) break;
			t_next = t->vnext.load(memory_order_seq_cst);
		}while (is_marked_ref((long) t_next) || (t->val < key));
		
		curr = t;
		/* Check that nodes are adjacent */
		if(succ == curr) {
			if (curr->vnext.load(memory_order_seq_cst) && is_marked_ref((long)curr->vnext.load(memory_order_seq_cst)))
				goto retry;
			else {
			(*n1) = pred;
			(*n2) = curr;
			return;
			}
		}
		/* Remove one or more marked nodes */
		if(atomic_compare_exchange_strong_explicit(& pred->vnext, &succ, curr,  memory_order_seq_cst, memory_order_seq_cst)) {
			if (curr->vnext.load(memory_order_seq_cst) && is_marked_ref((long) curr->vnext.load(memory_order_seq_cst)))
				goto retry;
			else {
			(*n1) = pred;
			(*n2) = curr;
			return;
			}
		} 
		
	  }while (true);
	}
	else{ // locate in the edge list of the vertex 
	 retry1:
	do{
		slist_t *t = head;
		slist_t *t_next = t->enext.load(memory_order_seq_cst);
		/* Find pred and curr */
		do{
			if (!is_marked_ref2((long) t_next)) {
				pred = t;
				succ = t_next;
			}
			t = (slist_t *) get_unmarked_ref2((long)t_next);
			if (!t->enext) break;
			t_next = t->enext.load(memory_order_seq_cst);
		} while(is_marked_ref2((long) t_next) || (t->val < key));
		
		curr = t;
		/* Check that nodes are adjacent */
		if(succ == curr) {
			if (curr->enext.load(memory_order_seq_cst) && is_marked_ref2((long)curr->enext.load(memory_order_seq_cst)))
				goto retry1;
			else {
			(*n1) = pred;
			(*n2) = curr;
			return;
			}
		}
		/* Remove one or more marked nodes */
		if(atomic_compare_exchange_strong_explicit(& pred->enext, &succ, curr,  memory_order_seq_cst, memory_order_seq_cst)) {
			if (curr->enext.load(memory_order_seq_cst) && is_marked_ref2((long) curr->enext.load(memory_order_seq_cst)))
				goto retry1;
			else {
			(*n1) = pred;
			(*n2) = curr;
			return;
			}
		} 
		
	  } while(true);
	
	}
	
	
	}
  
         void locateAC(slist_t* head, slist_t ** n1, slist_t ** n2, int key){
         slist_t *succ, *curr, *pred;
        // locate in the edge list of the vertex 
	 retry1:
	do{
		slist_t *t = head;
		slist_t *t_next = t->enext.load(memory_order_seq_cst);
		/* Find pred and curr */
		do{
			if (!is_marked_ref2((long) t_next)) {
				pred = t;
				succ = t_next;
			}
			t = (slist_t *) get_unmarked_ref2((long)t_next);
			if (!t->enext) break;
			t_next = t->enext.load(memory_order_seq_cst);
		} while(is_marked_ref2((long) t_next) || (t->val < key));
		
		curr = t;
		/* Check that nodes are adjacent */
		if(succ == curr) {
			if (curr->enext.load(memory_order_seq_cst) && is_marked_ref2((long)curr->enext.load(memory_order_seq_cst)))
				goto retry1;
			else {
			(*n1) = pred;
			(*n2) = curr;
			return;
			}
		}
	        /*
		// Remove one or more marked nodes 
		if(atomic_compare_exchange_strong_explicit(& pred->enext, &succ, curr,  memory_order_seq_cst, memory_order_seq_cst)) {
			if (curr->enext.load(memory_order_seq_cst) && is_marked_ref2((long) curr->enext.load(memory_order_seq_cst)))
				goto retry1;
			else {
			(*n1) = pred;
			(*n2) = curr;
			return;
			}
		}
		*/ 
		
	  } while(true);
	
	
	
	
	}
             
         bool HWFContains(slist_t * h, slist_t *t, slist_t ** n, int key){
          slist_t *curr, *pred;
          if(h == Head){ // search in the vertex list
                pred = Head;
	        locate(h, t, &pred, &curr, key);
	        if((!curr->vnext.load(memory_order_seq_cst)) || curr->val != key)
		        return false;
	        else {
	                (*n) = curr; 
		        return true;
		     }   
          }
          else{   // search in the edge list
             pred = h;
	        locate(h, t, &pred, &curr, key);
	        if((!curr->enext.load(memory_order_seq_cst)) || curr->val != key)
		        return false;
	        else {
	                (*n) = curr; 
		        return true;
		    }   
          }
         }
        bool ContainsVAC1(slist_t ** n, int key){
          slist_t *curr, *pred;
                //pred = Head;
	        locate(Head, Tail, &pred, &curr, key);
	        if((!curr->vnext. load(memory_order_seq_cst)) || curr->val != key)
		        return false;
	        else {
	                (*n) = curr; 
		        return true;
		     }   
          }
       bool ContainsEAC1(slist_t *Ehead, slist_t **n, int key ){
                slist_t* pred, *curr;
                locateAC(Ehead, &pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return success, true
                }
                else
                        return false; // key not present
        
        }
        bool Add(slist_t * h, slist_t *t, int key){
                slist_t* pred, *curr;
                if(h == Head){ // vertex add
                slist_t *newv = createV(key); // create a new vertex node
               while(true){
               locate(h, t, &pred, &curr, key);
                if(curr->val == key){
                     return true;
                   }       
               else{
                      newv->vnext.store(curr,memory_order_seq_cst);  
                      if(pred->vnext.compare_exchange_strong(curr, newv, memory_order_seq_cst)) 
                              return true;
                   }
                 }
               }
               else{ // edge add
               slist_t *newe = createE(key), *temp;// create a new edge node
                while(true){
               locate(h, t, &pred, &curr, key);
                if(curr->val == key){
                     return false;
                   }       
               else{
                      newe->enext.store(curr,memory_order_seq_cst);  
                      // set the status to transit 
                      temp = newe->enext.load(memory_order_seq_cst);
                      if(atomic_compare_exchange_strong_explicit(&newe->enext, &temp, (slist_t*)get_unmarked_ref2((long)temp), memory_order_seq_cst, memory_order_seq_cst)){
                      // add newe to the list
                      if(pred->enext.compare_exchange_strong(curr, newe, memory_order_seq_cst))
                              return true;
                   }
                  } 
                 }
          }
   }

        // change the status to added        
         bool AddAE(slist_t *h, int key){
                 slist_t* pred, *curr, *succ;
                while(true){
                   locateAC(h, &pred, &curr, key);
                   if(curr->val == key){
                     return false;
                   }       
                   else{
                      succ = curr->enext.load(memory_order_seq_cst);
                      if(atomic_compare_exchange_strong_explicit(&curr->enext, &succ, (slist_t*)get_unmarked_ref((long)succ), memory_order_seq_cst, memory_order_seq_cst))
                              return true;
                   }
                 }
      } //End of Remove method
      
    bool Remove(slist_t *h, slist_t *t, int key){
        slist_t* pred, *curr, *succ;
        //locate(h, t, &pred, &curr, key);
        if(h==Head){// vertex delete
        pred = h;
            do {
		locate(h, t, &pred, &curr, key);
		if(curr->val != key)
			return false;
		succ = curr->vnext.load(memory_order_seq_cst);
		if(!is_marked_ref((long) succ))
			if(atomic_compare_exchange_strong_explicit(&curr->vnext, &succ,(slist_t*)get_marked_ref((long)succ),  memory_order_seq_cst, memory_order_seq_cst))
				break;
	} while(true);
	if (!atomic_compare_exchange_strong_explicit(&pred->vnext, &curr, succ, memory_order_seq_cst, memory_order_seq_cst))
		locate(h, t, &pred, &curr, curr->val);
		//curr = harris_search(set, curr->val, &pred);
	return true;
  
        }
         else{
          pred = h;
           do {
		locate(h, t, &pred, &curr, key);
		if(curr->val != key)
			return false;
		succ = curr->enext.load(memory_order_seq_cst);
		if(!is_marked_ref((long) succ))
			if(atomic_compare_exchange_strong_explicit(&curr->enext, &succ,(slist_t*)get_marked_ref2((long)succ), memory_order_seq_cst, memory_order_seq_cst)) // logical deletion
				break;
	} while(true);
	if (!atomic_compare_exchange_strong_explicit(&pred->enext, &curr, succ, memory_order_seq_cst, memory_order_seq_cst)) 
		locate(h, t, &pred, &curr, curr->val);
		
	return true;
        }// End of else 
      } //End of Remove method
       
       
       
       void locateV(slist_t ** n1, slist_t ** n2, int key){
                 
                slist_t* pred = Head; 
                slist_t* curr = pred->vnext; 
                while(curr->vnext != NULL && curr->val < key){
                        pred = curr;
                        curr = curr->vnext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                }
         void locateE(slist_t* Ehead, slist_t ** n1, slist_t ** n2, int key){
                slist_t* pred = Ehead; 
                slist_t* curr = pred->enext; 
                while( curr->enext != NULL && curr->val < key){ //curr->enext != NULL &&
                        pred = curr;
                        curr = curr->enext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                 }
                
                       
        bool ContainsVAC(slist_t ** n, int key){
                slist_t* pred, *curr;
                locateV(&pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return success, true
                }
                else
                        return false; // key not present
                
        }
        bool ContainsEAC(slist_t *Ehead, slist_t ** n, int key){
                slist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return success, true
                }
                else
                        return false; // key not present
                
        }
        /* cycle detection from the vertex u to v. The vertices whchi are reachable from v to u */  
      
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
        if (!is_marked_ref((long) u) && !is_marked_ref((long) v)){
        list<int> que;
        cyclevisit item;
        item.key = u->val;
        item.vit = true;
        visited.push_back(item);
       // visited[u->val] = true;
        que.push_back(u->val);
        while(!que.empty()){
            int s = que.front();
                que.pop_front();
            bool flag1 = HWFContains(Head, Tail, &temp, s);  
          if(flag1 == true){
               temp = temp->enext.load(memory_order_seq_cst); // Ehead
                for(it = temp->enext.load(memory_order_seq_cst); it->enext.load(memory_order_seq_cst) != NULL; it = it ->enext){
                 if(!is_marked_ref2((long) it)){  
                   bool flag2 = HWFContains(Head, Tail, &temp2, it->val);  
                   if(flag2 == true){ // check for vertex is present in the list
                   
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
                   }  
                 }
                }
              }  
        return false;                
     }
     
     bool isReachable1(slist_t *u, slist_t *v){
        slist_t * temp, *temp2, *it;
        if(u->val == v->val) // base case
          return true;
        list <cyclevisit>  visited;
        //bool *visited = new bool[n+1];
        //for(int i = 0; i <=n; i++) 
        //        visited[i] = false;
       // if (!is_marked_ref((long) u) && !is_marked_ref((long) v)){
        list<int> que;
        cyclevisit item;
        item.key = u->val;
        item.vit = true;
        visited.push_back(item);
       // visited[u->val] = true;
        que.push_back(u->val);
        while(!que.empty()){
            int s = que.front();
                que.pop_front();
            bool flag1 = ContainsVAC(&temp, s);  
          if(flag1 == true){
               temp = temp->enext.load(memory_order_seq_cst); // Ehead
                for(it = temp->enext.load(memory_order_seq_cst); it->enext.load(memory_order_seq_cst) != NULL; it = it ->enext){
                // if(!is_marked_ref((long) it)){  
                   bool flag2 = ContainsVAC(&temp2, it->val);  
                   if(flag2 == true){ // check for vertex is present in the list
                   
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
                  //  }  
                  // }  
                 }
                }
              }  
        return false;                
     }
     /* cycle detection for whole graph, from Head to Tail*/
      bool testCycle(){
        slist_t * temp=Head->vnext.load(memory_order_seq_cst), *temp1, *it, *curr;
         while(temp->vnext.load(memory_order_seq_cst) != NULL){
            temp1 = temp->vnext.load(memory_order_seq_cst);
           // slist_t *temp2 = temp;
            
           while(temp1->vnext.load(memory_order_seq_cst) != NULL){
            // cout<<temp1->val<<" ";
              if(isReachable1(temp1, temp)) {
                // cout<<" reachable:"<<temp1->val<<" "<<temp->val<<endl;
               if(ContainsEAC(temp->enext.load(memory_order_seq_cst), &curr, temp1->val )){
                        cout<<" cycle:"<<temp->val<<" "<<temp1->val<<endl;
                return true; // cycle is present
               } 
              } 
             temp1 = temp1->vnext;   
          }
          temp = temp->vnext;
         }
  return false;                
 }
 
 /*        bool isReachableMultiThread(slist_t *u, slist_t *v, long n){
        slist_t * temp, *it;
        if(u->val == v->val) // base case
          return true;
        bool *  visited = new bool[n];
        //vector <bool> visited;
        for(long i = 0; i <=n; i++) 
                visited[i] = false;
        list<long> que;
        if (!is_marked_ref((long) u) && !is_marked_ref((long) v)){
        visited[u->val] = true;
        que.push_back(u->val);
        
        while(!que.empty()){
            long s = que.front();
                que.pop_front();
                //cout<<" s:"<<s;
            bool flag1 = HWFContains(Head, Tail, &temp, s);  
          if((flag1 == true ) && !is_marked_ref((long) u)){
               
               temp = temp->enext.load(memory_order_seq_cst); // Ehead
                for(it = temp->enext.load(memory_order_seq_cst); it->enext.load(memory_order_seq_cst) != NULL; it = it ->enext.load(memory_order_seq_cst)){
                      if(!is_marked_ref((long) it)){                     
                        if(it->val == v->val) 
                                return true; // reachable
                        if(!visited[it->val]){
                                visited[it->val] = true;
                                que.push_back(it->val);
                        }
                    }
                    }    
                 }
                }
          }      
        return false;                
     }
     int c=0;
     // cycle detection for whole graph, from Head to Tail
      bool isCycleMultiThread1(long n){
        slist_t * temp=Head->vnext.load(memory_order_seq_cst), *temp1, *it;
         while(temp!=Tail){
            temp1 = temp->vnext.load(memory_order_seq_cst);
           while(temp1 != Tail){
              
              if(isReachableMultiThread(temp1, temp, n)){
               
              if(ContainsE(temp->enext, &curr, temp1->val )){
               cout<<" cycle:"<<temp->val<<" "<<temp1->val<<endl;
                return true; // cycle is present
               } 
              }  
             temp1 = temp1->vnext.load(memory_order_seq_cst);   
          }
          temp = temp->vnext.load(memory_order_seq_cst);
         }
         //cout<<"# no cycle:"<<c<<endl;
         return false;                
 } 
       */
        
};

       
 class Graph{
    list1 lt;
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
              /*  if(lt.Remove(lt.Head, lt.Tail, key)){ // if true then delete all incoming edges
                  slist_t *temp1 = lt.Head->vnext.load(memory_order_seq_cst);
                  while(temp1 != lt.Tail){
                    lt.Remove(temp1->enext.load(memory_order_seq_cst), NULL, key);
                    temp1 = temp1->vnext.load(memory_order_seq_cst);
                }
                return true;
               } 
               else
                  return false; 
                */
        }
        bool ContainsV(int key){
                 slist_t *g1;
                 
                 if(lt.HWFContains(lt.Head, lt.Tail, &g1, key)){
                        //(*n) = &g1;                  
                   return true;
                   }
                 else
                    return false;  
        }
       int c2=0;
        bool AddE(int key1, int key2){
          slist_t *u,*v;
          bool flag1 = lt.HWFContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.HWFContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.HWFContains(lt.Head, lt.Tail, &u, key1);  
          if(flag1 == false)
            return false;
            
             bool status = lt.Add(u->enext,NULL, key2);// add the edge  
          if(status == true){
          
              
             bool c1 = lt.isReachable(v, u);// check for cycle
              if(c1==true) { 
                status = RemoveE(key1, key2); // remove the edge
                }
              else{
              status = lt.AddAE(u->enext, key2); // change the status to added 
              }  
              
              
           }
           return status; 
        //  return lt.Add(u->enext,NULL, key2);  
        }
        bool RemoveE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.HWFContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.HWFContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.HWFContains(lt.Head, lt.Tail, &u, key1);  
          if(flag1 == false)
            return false;
            
          return lt.Remove(u->enext,NULL, key2);  
         }
        bool ContainsE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.HWFContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.HWFContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          return lt.HWFContains(u->enext,NULL, &v, key2);  
        
        }
        
        void PrintGraph(){
	slist_t *temp1 = lt.Head;
	slist_t *temp2;
	while(temp1 != NULL){
		cout << temp1->val << "->";
		temp2 = temp1->enext.load(memory_order_seq_cst);
		while(temp2 != NULL)
		{
			cout << temp2->val << " ";
			temp2 = temp2->enext.load(memory_order_seq_cst);
		}
		cout << endl;
		temp1 = temp1->vnext.load(memory_order_seq_cst);
	   }
        }
        
         bool InitAddE(int key1, int key2){
          slist_t *u,*v;
          bool flag1 = lt.HWFContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.HWFContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
                
             
         return lt.Add(u->enext,NULL, key2);  
        }
        void create_initial_vertices(int n){
          int i,j;
          for(i=1;i<=n;i++){
            AddV(i);
          }
          /*
          for(i=1;i<=n;i++){
            for(j=i+1; j<=n; j++)
              InitAddE(i, j);
          }
          */
        
        }
       
         bool checkCycle(){
         return lt.testCycle();
       } 
   };     
   


#endif

