 /*
 * File: lfGraph_getPath.cpp
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
#define pp "PATH PRESENT"
#define pntp "PATH NOT PRESENT"

using namespace std;

 ofstream coutt("getpath.txt");
 // freopen("error.txt", "w", stderr );
  
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
	atomic <int> ecount; // counter for edge operations
	int *visitedArray; // size same as # threads
}vlist_t;

// BFSNode structure
typedef struct BFSNode{
        int ecount;
	struct VNode *n; 
	struct BFSNode *p; 
	struct BFSNode *next; 
}bfslist_t;

thread_local int cnt = 0; // thread local variable , used in the visited list

//class BFSTree;
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
vlist_t* createV(int key, int n){
          elist_t *EHead = createE(INT_MIN); //create Edge Head
          elist_t *ETail = createE(INT_MAX); // create Edge Tail
          EHead ->enext.store(ETail); 
          vlist_t * newv = (vlist_t*) malloc(sizeof(vlist_t));
          newv ->val = key;
          newv ->vnext.store(NULL);
          newv->ecount.store(0); // init ecounter to zero
          newv->visitedArray = new int[n];// same as # threads
          for(int i=0; i<n; i++)
            newv->visitedArray[i] = 0;
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
          Tail ->ecount.store(0);
          Tail ->visitedArray = NULL;
          Head = (vlist_t*) malloc(sizeof(vlist_t));
          Head ->val = INT_MIN;
          Head ->enext.store(NULL);
          Head ->ecount.store(0);
          Head ->visitedArray = NULL;
          Head->vnext.store(Tail); // Head next is Tail
 }

 // Find pred and curr for VNode(key)     
void locateVPlus(vlist_t *startV, vlist_t ** n1, vlist_t ** n2, int key){
       vlist_t *succv, *currv, *predv;
      retry:
       while(true){
        predv = startV;
        currv = predv->vnext.load();
        while(true){
         succv = currv->vnext.load();
         while(currv->vnext.load() != NULL && is_marked_ref((long) succv) && currv->val < key ){ 
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
 bool AddV(int key, int n){
      vlist_t* predv, *currv;
      while(true){
        locateVPlus(Head, &predv, &currv, key); // find the location, <pred, curr>
        if(currv->val == key){
           return false; // key already present
        }       
        else{
           vlist_t *newv = createV(key, n); // create a new vertex node
           newv->vnext.store(currv);  
           if(predv->vnext.compare_exchange_strong(currv, newv, memory_order_seq_cst)) {// added in the vertex-list
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
		  if(!currv->vnext.compare_exchange_strong(succv, (vlist_t*)get_marked_ref((long)succv), memory_order_seq_cst)) // logical deletion
		    continue;
		   if(predv->vnext.compare_exchange_strong(currv, succv, memory_order_seq_cst)){ // physical deletion
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

   // ContainsV+       
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
    

 // Find pred and curr for ENode(key)    in the edge-list 
void locateEPlus(vlist_t **startE, elist_t ** n1, elist_t ** n2, int key){
       elist_t *succe, *curre, *prede;
       vlist_t *tv;
retry: while(true){
        prede = (*startE)->enext;
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
           (*startE)->ecount.fetch_add(1, memory_order_relaxed);
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
 char * AddE(int key1, int key2){
               elist_t* prede, *curre;
               vlist_t *u,*v;
               bool flag = ContainsVPlus(&u, &v, key1, key2);
               if(flag == false){             
                  return (char*)vntp; // either of the vertex is not present
               }   
                while(true){
                if(is_marked_ref((long) u->vnext.load()) || is_marked_ref((long) v->vnext.load()) )
                   return (char*)vntp;
                locateEPlus(&u, &prede, &curre, key2);
                if(curre->val == key2){
                     return (char*)ep; // edge already present
                   }       
                   elist_t *newe = createE(key2);// create a new edge node
                   newe->enext.store(curre);  // connect newe->next to curr
                   newe->pointv.store(v); // points to its vertex
                   if(prede->enext.compare_exchange_strong(curre, newe, memory_order_seq_cst)){  // insertion
                      u->ecount.fetch_add(1, memory_order_relaxed);
                      return (char*)eadd;
              }
            } // End of while           
     }    
        
// Deletes an edge from the edge-list if present
 char* RemoveE(int key1, int key2){
               elist_t* prede, *curre, *succe;
               vlist_t *u,*v;
               bool flag = ContainsVPlus(&u, &v, key1, key2);
               if(flag == false){             
                  return (char*)vntp; // either of the vertex is not present
               }   
               while(true){
              if(is_marked_ref((long) u->vnext.load()) || is_marked_ref((long) v->vnext.load()) )
                   return (char*)vntp;
               locateEPlus(&u, &prede, &curre, key2);
                if(curre->val != key2){
                     return (char*)entp; // edge already present
                   } 
               succe = curre->enext.load();
	       if(!is_marked_ref((long) succe)){
	         if(!curre->enext.compare_exchange_strong(succe, (elist_t*)get_marked_ref((long)succe), memory_order_seq_cst)) //  logical deletion
		    continue;
		  u->ecount.fetch_add(1, memory_order_relaxed); // increament the counter
		  if(!prede->enext.compare_exchange_strong(curre, succe, memory_order_seq_cst)){ // physical deletion
			break;
	          }
	      }
	}
	return (char*)er;      
 }    
 
 // Find pred and curr for VNode(key), used for contains edge     
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
          return ;
        
} 
   // ContainsC++       
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
 char* ContainsE(int key1, int key2){
          elist_t *curre, *prede;
          vlist_t *u,*v;
          bool flag = ContainsCPlus(&u, &v, key1, key2);
          if(flag == false){             
               return (char*)vntp; // either of the vertex is not present
          }   
          curre = u->enext.load(); 
          while(curre->enext.load() && curre->val < key2){
           curre =  (elist_t*)get_unmarked_ref((long)curre->enext.load());
          }
	 if((curre->enext.load()) && curre->val == key2 && !is_marked_ref((long) curre->enext.load()) && !is_marked_ref((long) u->vnext.load()) && !is_marked_ref((long) v->vnext.load())){
	        return (char*)ep;
	        }
	  else {
	        return (char*)ventp;
          }   
    }             


 void initGraph(int n, int NT){
     int i,j;
     for( i=1;i<=n;i++){
         AddV(i, NT);
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

// create BFSNode
bfslist_t* createBFSNode(int ecount, vlist_t *n, bfslist_t *p, bfslist_t *next){
          bfslist_t * newbfs = (bfslist_t*) malloc(sizeof(bfslist_t));
          newbfs->ecount = ecount;
          newbfs ->n = n;
          newbfs ->p = p;
          newbfs ->next = next;
         return newbfs;
        } 
        
// check for visited  
bool checkVisited(int tid, VNode *v, int cValue){
        return v->visitedArray[tid] == cValue;
   }

 // get the path from key1 to key2     
 bool TreeCollect(int tid, vlist_t *u, vlist_t *v, bfslist_t &bfsHead, bfslist_t &bfsTail){
    elist_t * eHead;
    queue <bfslist_t*> que;
    cnt = cnt + 1;
    u->visitedArray[tid] = cnt;
    bfslist_t *bfsNode = createBFSNode(u->ecount,u,NULL,NULL);
    Insert(bfsHead, bfsTail, bfsNode);
    que.push(bfsNode);
    while(!que.empty()){ // run until que is not empty
       bfslist_t *currentVNode = que.front(); // get the front
       que.pop(); // delete from the front
       eHead = currentVNode->n->enext.load(); // Ehead
         for(elist_t * itNode = eHead->enext.load(); itNode->enext.load() != NULL; itNode = (elist_t*)get_unmarked_ref((long)itNode ->enext.load())){ // iterate through all adjacency vertices 
             if(!is_marked_ref((long) itNode->enext.load())){  
                vlist_t *adjVNode = itNode->pointv.load(); // get the vertex
                if(adjVNode && adjVNode->vnext.load()){ // check for vertex is present in the list
		   if (!is_marked_ref((long) adjVNode->vnext.load()) ){ // check the vertex is marked or not
                       if(adjVNode == v){ // found the key
                          bfslist_t *bfsNode1 = createBFSNode(adjVNode->ecount, adjVNode,currentVNode,NULL);
                          Insert(bfsHead, bfsTail, bfsNode1);
                          return true; // reachable
                         }
                                // if not visted, push in to the queue
                                if(!checkVisited(tid, adjVNode, cnt) ){ 
                                   adjVNode->visitedArray[tid] = cnt;
                                   bfslist_t *bfsNode2 = createBFSNode(adjVNode->ecount, adjVNode,currentVNode,NULL);
                                   Insert(bfsHead, bfsTail, bfsNode2);
                                   que.push(bfsNode2);
                                }
                              } 
                            }  
                           }  
                         }
                      } 
                return false;                
       }
   
   /*Get Path definition*/    
char* GetPath(int key1, int key2, int tid){
  vlist_t *u,*v;
  bool flag = ContainsCPlus(&u, &v, key1, key2);
  if(flag == false){             
      return (char*)vntp; // either of the vertex is not present
    }   
  if(is_marked_ref((long) u->vnext.load()) || is_marked_ref((long) v->vnext.load()) )
    return (char*)vntp; // either of the vertex is marked
      vlist_t *  bTail = createV(INT_MAX,1);
      vlist_t *  bHead = createV(INT_MIN,1);
      
 bfslist_t *BFSHead = createBFSNode(1, bHead, NULL, NULL); 
 bfslist_t *BFSTail = createBFSNode(1, bTail,BFSHead, NULL);
 BFSHead->next = BFSTail;
 BFSTail->next = BFSHead;
  bool status = Scan(tid, u, v, *BFSHead, *BFSTail); 
  if(status == true){
    //PrintPath(BFSHead, BFSTail);
    return (char*) pp;
    }
  else if(status == false)  
          return (char*) pntp;
}

/*Scan method*/
bool Scan(int tid, vlist_t *u, vlist_t *v, bfslist_t &bfsHead, bfslist_t &bfsTail){
  vlist_t *  bTail = createV(INT_MAX,1);
      vlist_t *  bHead = createV(INT_MIN,1);
  bfslist_t *oldbfsTreeHead = createBFSNode(1, bHead, NULL, NULL); 
 bfslist_t *oldbfsTreeTail = createBFSNode(1, bTail,oldbfsTreeHead, NULL);
 oldbfsTreeHead->next = oldbfsTreeTail;
 oldbfsTreeTail->next = oldbfsTreeHead; 
  bfslist_t *newbfsTreeHead = createBFSNode(1, bHead, NULL, NULL); 
 bfslist_t *newbfsTreeTail = createBFSNode(1, bTail,newbfsTreeHead, NULL);
 newbfsTreeHead->next = newbfsTreeTail;
 newbfsTreeTail->next = newbfsTreeHead;
 bool flag1 = TreeCollect(tid, u, v, *oldbfsTreeHead, *oldbfsTreeTail);
 while(true){
   bool flag2 = TreeCollect(tid, u, v, *newbfsTreeHead, *newbfsTreeTail);
   if(flag1 == true && flag2 == true && ComparePath(*oldbfsTreeTail , *newbfsTreeTail)){
     bfsHead = *newbfsTreeHead;
     bfsTail = *newbfsTreeTail;
     coutt<<"Path("<<u->val<<","<<v->val<<"):";
     PrintPath(bfsTail);
    // PrintBFSPath(bfsHead,bfsTail);
     return true;
   }
   if(flag1 == false && flag2 == false && CompareTree(*oldbfsTreeHead, *newbfsTreeHead)){
     return false;
   }
   flag1 = flag2;
   oldbfsTreeHead = newbfsTreeHead;
   oldbfsTreeTail = newbfsTreeTail;
 }
 return false;
}

bool ComparePath(bfslist_t &oldbfsTreeTail, bfslist_t &newbfsTreeTail){
  if(&oldbfsTreeTail == NULL || &newbfsTreeTail == NULL ){
    return false;
  }
  bfslist_t *oldbfsTreeIt = &oldbfsTreeTail;
  bfslist_t *newbfsTreeIt = &newbfsTreeTail;
  do{
    if(oldbfsTreeIt->n != newbfsTreeIt->n || oldbfsTreeIt->ecount != newbfsTreeIt->ecount || oldbfsTreeIt->p != newbfsTreeIt->p ){
       return false;
    }
    oldbfsTreeIt = oldbfsTreeIt->p;
    newbfsTreeIt = newbfsTreeIt->p;
  }while(oldbfsTreeIt->p != NULL && newbfsTreeIt->p !=NULL );
  
  if(oldbfsTreeIt->n != newbfsTreeIt->n || oldbfsTreeIt->ecount != newbfsTreeIt->ecount || oldbfsTreeIt->p != newbfsTreeIt->p ){
       return false;
    }
  else
    return true;  
}

/* compare tree*/
bool CompareTree(bfslist_t &oldbfsTreeHead, bfslist_t &newbfsTreeHead){
  if(&oldbfsTreeHead == NULL || &newbfsTreeHead == NULL ){
    return false;
  }
  bfslist_t *oldbfsTreeIt = &oldbfsTreeHead;
  bfslist_t *newbfsTreeIt = &newbfsTreeHead;
  do{
    if(oldbfsTreeIt->n != newbfsTreeIt->n || oldbfsTreeIt->ecount != newbfsTreeIt->ecount || oldbfsTreeIt->p != newbfsTreeIt->p ){
       return false;
    }
    oldbfsTreeIt = oldbfsTreeIt->next;
    newbfsTreeIt = newbfsTreeIt->next;
  }while(oldbfsTreeIt->next->n->val != INT_MAX && newbfsTreeIt->next->n->val !=INT_MAX );
  
  if(oldbfsTreeIt->n != newbfsTreeIt->n || oldbfsTreeIt->ecount != newbfsTreeIt->ecount || oldbfsTreeIt->p != newbfsTreeIt->p ){
       return false;
    }
  else
    return true;  
}

void Insert(bfslist_t &BFSHead, bfslist_t &BFSTail, bfslist_t *node){
      node->next = &BFSTail;
      BFSTail.next->next = node;
      BFSTail.next = node;
  }
      
void PrintBFSTree(bfslist_t &BFSHead, bfslist_t &BFSTail){
   bfslist_t * node = &BFSHead;
   while(node->n->val != INT_MAX){
    node = node->next;
   }
}
 
void PrintBFSPath(bfslist_t &BFSHead, bfslist_t &BFSTail){
   bfslist_t * node = BFSTail.next;
   while(node->p){ 
    coutt<<" n, p:"<< node->n->val<<" "<<node->p->n->val;
    node = node->p;
   }
   coutt<<endl;
 }

void PrintPath(bfslist_t &bNodeTail){
  stack <bfslist_t*> stk;
  bfslist_t *bNode = bNodeTail.next;
  while(bNode){
   stk.push(bNode);
   bNode = bNode->p;
  }
  coutt<<"start=>";
  while(!stk.empty()){
   bfslist_t * node = stk.top();
   stk.pop();
   coutt<< node->n->val<<"->";
  } 
  coutt<<"end"<<endl;
  }
  
};

