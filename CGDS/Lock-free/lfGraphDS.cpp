/*
 * File:lfGraphDS.cpp
 *  
 *
 * Author(s):
 *   Bapi Chatterjee <bapchatt@in.ibm.com>
 *   Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *   
 * Description:
 *   lock-free implementation of a graph
 * Copyright (c) 2018.
 * last Updated: 31/07/2018
 *
*/
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
#include<list>
#include<queue>
#include<stack>

using namespace std;

//ofstream coutt("getpath.txt");
 // freopen("error.txt", "w", stderr );
  
#define vntp "VERTEX NOT PRESENT"
#define entp "EDGE NOT PRESENT"
#define ventp "VERTEX OR EDGE NOT PRESENT"
#define ep "EDGE PRESENT"
#define eadd "EDGE ADDED"
#define er "EDGE REMOVED"
#define ef "EDGE FOUND"
#define vp "VERTEX PRESENT"
#define vadd "VERTEX ADDED"
#define vr "VERTEX REMOVED"



inline int is_marked_ref(long i){
  return (int) (i & 0x1L);
}


inline long unset_mark(long i){
  i &= ~0x1L;
  return i;
}

inline long set_mark(long i){
  i |= 0x1L;
  return i;
}

inline long get_unmarked_ref(long w){
  return w & ~0x1L;
}

inline long get_marked_ref(long w){
  return w | 0x1L;
}


// ENode structure
typedef struct ENode{
	int val; // data
	atomic<struct VNode *>pointv; // pointer to its vertex
	atomic<struct ENode *>enext; // pointer to the next ENode
}elist_t;

// VNode structure
typedef struct VNode{
	int val; // data
	atomic<struct VNode *>vnext; // pointer to the next VNode
	atomic<struct ENode *>enext; // pointer to the EHead
}vlist_t;

class slist{
 public:
 vlist_t *Head, *Tail;
// creation of new ENode
 elist_t* createE(int key){
          elist_t * newe = (elist_t*) malloc(sizeof(elist_t));
          newe ->val = key;
          newe ->pointv.store(NULL);
          newe ->enext.store(NULL);
         return newe;
        }

// creation of new VNode
vlist_t* createV(int key){
          elist_t *EHead = createE(INT_MIN); //create Edge Head
          elist_t *ETail = createE(INT_MAX); // create Edge Tail
          EHead ->enext.store(ETail); 
          vlist_t * newv = (vlist_t*) malloc(sizeof(vlist_t));
          newv ->val = key;
          newv ->vnext.store(NULL);
          newv ->enext.store(EHead);
          EHead ->pointv.store(newv);
          ETail ->pointv.store(Tail);
         return newv;
 }
 
 // init of sentinals Head and Tail
void init(){
          Tail = (vlist_t*) malloc(sizeof(vlist_t));
          Tail ->val = INT_MAX;
          Tail ->vnext.store(NULL);
          Tail ->enext.store(NULL);
          Head = (vlist_t*) malloc(sizeof(vlist_t));
          Head ->val = INT_MIN;
          Head ->enext.store(NULL);
          Head->vnext.store(Tail); // Head next is Tail
 }

int i;
 // Find pred and curr for VNode(key)     
void locateVPlus(vlist_t *startV, vlist_t ** n1, vlist_t ** n2, int key){
       vlist_t *succv, *currv, *predv;
      retry:
       while(true){
        predv = startV;
        currv = predv->vnext.load();
        while(true){
         succv = currv->vnext.load();
         while(currv->vnext.load() != NULL && is_marked_ref((long) succv) && currv->val < key ){ // 
           if(!predv->vnext.compare_exchange_strong(currv, (vlist_t *) get_unmarked_ref((long)succv), memory_order_seq_cst))
           goto retry;
           currv = (vlist_t *) get_unmarked_ref((long)succv);
           succv = currv->vnext.load(); 
         }
         if(currv->val >= key){
          (*n1) = predv;
          (*n2) = currv;
          return;
         }
         predv = currv;
         currv = succv;
        }
     }  
 } 
    
     // add a new vertex in the vertex-list
 bool AddV(int key){
      vlist_t* predv, *currv;
      while(true){
        locateVPlus(Head, &predv, &currv, key); // find the location, <pred, curr>
        if(currv->val == key){
           return false; // key already present
        }       
        else{
           vlist_t *newv = createV(key); // create a new vertex node
           newv->vnext.store(currv);  
           if(predv->vnext.compare_exchange_strong(currv, newv, memory_order_seq_cst)) {// adds in the list
               return true;
           }
          } 
      }
  }
    
// Deletes the vertex from the vertex-list
 bool RemoveV(int key){
        vlist_t* predv, *currv, *succv;
           while(true){
        	locateVPlus(Head, &predv, &currv, key);
		if(currv->val != key)
			return false; // key is not present
		succv = currv->vnext.load(); 
		if(!is_marked_ref((long) succv)){
		  if(!currv->vnext.compare_exchange_strong(succv, (vlist_t*)get_marked_ref((long)succv), memory_order_seq_cst))
		    continue;
		   if(predv->vnext.compare_exchange_strong(currv, succv, memory_order_seq_cst)){
                       break;
        	}
        	}	
	} 
   return true;
}
   
// Contains VNode   
bool ContainsV(int key){
          vlist_t *currv = Head;
          while(currv->vnext.load() && currv->val < key){
           currv =  (vlist_t *) get_unmarked_ref((long)currv->vnext.load());
          }
          vlist_t *succv = currv->vnext.load();
	  if((currv->vnext.load()) && currv->val == key && !is_marked_ref((long) succv)){
	        return true;
	     }   
	  else {
	        return false;
          }   
    }    
 
    	   

   
   // Contains++       
 bool ContainsVPlus(vlist_t ** n1, vlist_t ** n2, int key1, int key2){
          vlist_t *curr1, *pred1, *curr2, *pred2;
          if(key1 < key2){
          
             locateVPlus(Head, &pred1, &curr1, key1); //first look for key1 
             if((!curr1->vnext.load()) || curr1->val != key1)
	        return false; // key1 is not present in the vertex-list
	        
	     locateVPlus(curr1, &pred2, &curr2, key2); // looking for key2 only if key1 present
             if((!curr2->vnext.load()) || curr2->val != key2)
	        return false; // key2 is not present in the vertex-list
         }
        else{
             locateVPlus(Head, &pred2, &curr2, key2); //first look for key2 
             if((!curr2->vnext.load()) || curr2->val != key2)
	        return false; // key2 is not present in the vertex-list
	                
	     locateVPlus(curr2, &pred1, &curr1, key1); // looking for key1 only if key2 present
             if((!curr1->vnext.load()) || curr1->val != key1)
	        return false; // key1 is not present in the vertex-list

        }
     (*n1) = curr1; 
     (*n2) = curr2; 
    return true;    
 }	
    

 // Find pred and curr for ENode(key)     
void locateEPlus(elist_t *startE, elist_t ** n1, elist_t ** n2, int key){
       elist_t *succe, *curre, *prede;
       vlist_t *tv;
      
retry: while(true){
        prede = startE;
        curre = prede->enext.load();
        while(true){
         succe = curre->enext.load();
         tv = curre->pointv.load();
        /*helping: delete one or more enodes whose vertex was marked*/
retry2: while(tv && tv->vnext.load() && curre->enext.load() != NULL && is_marked_ref((long)tv->vnext.load()) && !is_marked_ref((long) succe) && curre->val < key ){ 
           if(!curre->enext.compare_exchange_strong(succe, (elist_t*)get_marked_ref((long)succe), memory_order_seq_cst))
           goto retry;
           if(!prede->enext.compare_exchange_strong(curre, succe, memory_order_seq_cst))
           goto retry;
           curre = (elist_t*)get_unmarked_ref((long)succe);
           succe = curre->enext.load();
           tv = curre->pointv.load();
          } 
        /*helping: delete one or more enodes which are marked*/
         while(curre->enext.load() != NULL && is_marked_ref((long) succe) && !is_marked_ref((long)tv->vnext.load()) &&  curre->val < key ){ 
           if(!prede->enext.compare_exchange_strong(curre, (elist_t*)get_unmarked_ref((long)succe), memory_order_seq_cst))
           goto retry;
           curre = (elist_t*)get_unmarked_ref((long)succe);
           succe = curre->enext.load(); 
           tv = curre->pointv.load();
         }
         
         if(tv && tv->vnext.load() && is_marked_ref((long) tv->vnext.load()) && curre->enext.load() != NULL && curre->val < key)
           goto retry2;
         if(curre->val >= key){
          (*n1) = prede;
          (*n2) = curre;
          return;
         }
         prede = curre;
         curre =(elist_t*)get_unmarked_ref((long)succe);
        }
       }  
    } 

// add a new edge in the edge-list    
 bool AddE(int key1, int key2){
               elist_t* prede, *curre;
               vlist_t *u,*v;
               bool flag = ContainsVPlus(&u, &v, key1, key2);
               if(flag == false){             
                  return false; // either of the vertex is not present
               }   
                while(true){
                if(is_marked_ref((long) u->vnext.load()) || is_marked_ref((long) v->vnext.load()) )
                   return false;
               
                locateEPlus(u->enext.load(), &prede, &curre, key2);
                if(curre->val == key2){
                     return false; // edge already present
                   }       
               
                   elist_t *newe = createE(key2);// create a new edge node
                   newe->enext.store(curre);  // connect newe->next to curr
                   newe->pointv.store(v); // points to its vertex
                   if(prede->enext.compare_exchange_strong(curre, newe, memory_order_seq_cst)){  // actual insertion
                      return true;
              }
            } // End of while           
     }    
        
// Deletes an edge from the edge-list if present
 bool RemoveE(int key1, int key2){
               elist_t* prede, *curre, *succe;
               vlist_t *u,*v;
               bool flag = ContainsVPlus(&u, &v, key1, key2);
               if(flag == false){             
                  return false; // either of the vertex is not present
               }   
               while(true){
              if(is_marked_ref((long) u->vnext.load()) || is_marked_ref((long) v->vnext.load()) )
                   return false;
               locateEPlus(u->enext.load(), &prede, &curre, key2);
                if(curre->val != key2){
                     return false; // edge already present
                   } 
               succe = curre->enext.load();
	       if(!is_marked_ref((long) succe)){
	         if(!curre->enext.compare_exchange_strong(succe, (elist_t*)get_marked_ref((long)succe), memory_order_seq_cst))
		    continue;
		  if(!prede->enext.compare_exchange_strong(curre, succe, memory_order_seq_cst)){
	        	break;
	          }
	      }
	}      
 }    
 // Find pred and curr for VNode(key)     
void locateCPlus(vlist_t *startV, vlist_t ** n1, vlist_t ** n2, int key){
       vlist_t  *currv, *predv;
        predv = startV;
        currv = startV->vnext.load();
        while(currv && currv->val < key){
         
         predv = currv;
         currv = (vlist_t*)get_unmarked_ref((long)currv->vnext.load());
    }
    
          (*n1) = predv;
          (*n2) = currv;
          return;
         
} 
   // Contains++       
 bool ContainsCPlus(vlist_t ** n1, vlist_t ** n2, int key1, int key2){
          vlist_t *curr1, *pred1, *curr2, *pred2;
          if(key1 < key2){
          
             locateCPlus(Head, &pred1, &curr1, key1); //first look for key1 
             if((!curr1->vnext.load()) || curr1->val != key1)
	        return false; // key1 is not present in the vertex-list
	        
	     locateCPlus(curr1, &pred2, &curr2, key2); // looking for key2 only if key1 present
             if((!curr2->vnext.load()) || curr2->val != key2)
	        return false; // key2 is not present in the vertex-list
         }
        else{
             locateCPlus(Head, &pred2, &curr2, key2); //first look for key2 
             if((!curr2->vnext.load()) || curr2->val != key2)
	        return false; // key2 is not present in the vertex-list
	                
	     locateCPlus(curr2, &pred1, &curr1, key1); // looking for key1 only if key2 present
             if((!curr1->vnext.load()) || curr1->val != key1)
	        return false; // key1 is not present in the vertex-list

        }
     (*n1) = curr1; 
     (*n2) = curr2; 
    return true;    
 }
 
//Contains ENode       
 bool ContainsE(int key1, int key2){
          elist_t *curre, *prede;
          vlist_t *u,*v;
          bool flag = ContainsCPlus(&u, &v, key1, key2);
          if(flag == false){             
               return false; // either of the vertex is not present
          }   
          curre = u->enext.load(); 
          while(curre->enext.load() && curre->val < key2){
           curre =  (elist_t*)get_unmarked_ref((long)curre->enext.load());
          }
	 if((curre->enext.load()) && curre->val == key2 && !is_marked_ref((long) curre->enext.load()) && !is_marked_ref((long) u->vnext.load()) && !is_marked_ref((long) v->vnext.load())){
	        return true;
	        }
	  else {
	        return false;
          }   
    }             

 void initGraph(int n){
     int i,j;
     for( i=1;i<=n;i++){
         AddV(i);
       }
    for( i=1;i<=n;i= i+2){
         for( j=i+1; j<=n; j=j+2){
          int u = rand()%n +1;
          int v = rand()%n +1;
          if(u!=v)
          AddE(u,v);
       }   
      } 
   } 
      
};
