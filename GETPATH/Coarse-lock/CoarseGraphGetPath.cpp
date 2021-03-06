/*
 * File:CoarseGraphGetPath.cpp
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

using namespace std;

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

ofstream coutt("getpath.txt");
 // freopen("error.txt", "w", stderr );

// ENode structure
typedef struct ENode{
	int val; // data
	struct VNode *pointv; // pointer to its vertex
	struct ENode *enext; // pointer to the next ENode
}elist_t;

// VNode structure
typedef struct VNode{
	int val; // data
	struct VNode *vnext; // pointer to the next VNode
	struct ENode *enext; // pointer to the EHead
	int ecount; // counter for edge operations
	int *visitedArray; // size same as # threads
}vlist_t;

// BFSNode structure
typedef struct BFSNode{
	struct VNode *n; 
	struct BFSNode *p; 
	struct BFSNode *next; 
}bfslist_t;

thread_local int cnt = 0; // thread local variable , used in the visited list
pthread_mutex_t lock;
//class BFSTree;
class slist{
 public:
 vlist_t *Head, *Tail;

// creation of new ENode
 elist_t* createE(int key){
          elist_t * newe = (elist_t*) malloc(sizeof(elist_t));
          newe ->val = key;
          newe ->pointv = NULL;
          newe ->enext = NULL;
         return newe;
        }

// creation of new VNode
vlist_t* createV(int key, int n){
          elist_t *EHead = createE(INT_MIN); //create Edge Head
          elist_t *ETail = createE(INT_MAX); // create Edge Tail
          EHead ->enext=ETail; 
          vlist_t * newv = (vlist_t*) malloc(sizeof(vlist_t));
          newv ->val = key;
          newv ->vnext=NULL;
          newv->ecount=0; // init ecounter to zero
          newv->visitedArray = new int[n];// same as # threads
          newv ->enext=EHead;
          EHead ->pointv=newv;
          ETail ->pointv=Tail;
         return newv;
}
 
 // init of sentinals Head and Tail
void init(){
          Tail = (vlist_t*) malloc(sizeof(vlist_t));
          Tail ->val = INT_MAX;
          Tail ->vnext=NULL;
          Tail ->enext=NULL;
          Tail ->ecount=0;
          Tail ->visitedArray = NULL;
          Head = (vlist_t*) malloc(sizeof(vlist_t));
          Head ->val = INT_MIN;
          Head ->enext=NULL;
          Head ->ecount=0;
          Head ->visitedArray = NULL;
          Head->vnext=Tail; // Head next is Tail
 }

int i;
 // Find pred and curr for VNode(key)     
void locateVPlus(vlist_t *startV, vlist_t ** n1, vlist_t ** n2, int key){
       vlist_t *succv, *currv, *predv;
        predv = startV;
        currv = startV->vnext;
        while(true){
         if(currv->val >= key){
          (*n1) = predv;
          (*n2) = currv;
          return;
         }
         predv = currv;
         currv = currv->vnext;
        
     }  
 } 
    
     // adds a new vertex in the vertex-list
 bool AddV(int key, int n){
      vlist_t* predv, *currv;
        locateVPlus(Head, &predv, &currv, key); // find the location, <pred, curr>
        if(currv->val == key){
           return false; // key already present
        }       
        else{
           vlist_t *newv = createV(key, n); // create a new vertex node
           newv->vnext=currv;  
           predv->vnext = newv;// adds in the list
               return true;
           }
  }
    
// Deletes the vertex from the vertex-list
 bool RemoveV(int key){
        vlist_t* predv, *currv, *succv;
        elist_t * prede, *curre;
      	locateVPlus(Head, &predv, &currv, key);
	if(currv->val != key)
        	return false; // key is not present
	 predv->vnext = currv->vnext;
	// delete all its incomming edges 
	vlist_t *temp1 = Head->vnext;
          while(temp1 != Tail){
              locateEPlus(&temp1, &prede, &curre, key);
                if(curre->val == key){
                       prede->enext = curre->enext;
                      }
                  temp1 = temp1->vnext;      
           }
   return true;
}
   
// Contains VNode   
bool ContainsV(int key){
          vlist_t *currv, *predv;
          locateVPlus(Head, &predv, &currv, key); 
	  if((currv->vnext) && currv->val == key )
	        return true;
	    
	  else 
	        return false;
             
    }    
 
   // Contains++       
 bool ContainsVPlus(vlist_t ** n1, vlist_t ** n2, int key1, int key2){
          vlist_t *curr1, *pred1, *curr2, *pred2;
          if(key1 < key2){
             locateVPlus(Head, &pred1, &curr1, key1); //first look for key1 
             if((!curr1->vnext) || curr1->val != key1)
	        return false; // key1 is not present in the vertex-list
	     locateVPlus(curr1, &pred2, &curr2, key2); // looking for key2 only if key1 present
             if((!curr2->vnext) || curr2->val != key2)
	        return false; // key2 is not present in the vertex-list
         }
        else{
             locateVPlus(Head, &pred2, &curr2, key2); //first look for key2 
             if((!curr2->vnext) || curr2->val != key2)
	        return false; // key2 is not present in the vertex-list
	                
	     locateVPlus(curr2, &pred1, &curr1, key1); // looking for key1 only if key2 present
             if((!curr1->vnext) || curr1->val != key1)
	        return false; // key1 is not present in the vertex-list
        }
     (*n1) = curr1; 
     (*n2) = curr2; 
    return true;    
 }	
    

 // Find pred and curr for ENode(key)     
void locateEPlus(vlist_t **startE, elist_t ** n1, elist_t ** n2, int key){
       elist_t *succe, *curre, *prede;
        prede = (*startE)->enext;
        curre = prede->enext;
        while(true){
         if(curre->val >= key){
          (*n1) = prede;
          (*n2) = curre;
          return;
         }
         prede = curre;
         curre =curre->enext;
        }  
    } 

// add a new edge in the edge-list    
 bool AddE(int key1, int key2){
               elist_t* prede, *curre;
               vlist_t *u,*v;
               bool flag = ContainsVPlus(&u, &v, key1, key2);
               if(flag == false){             
                  return (char*) vntp; // either of the vertex is not present
               }   
                locateEPlus(&u, &prede, &curre, key2);
                if(curre->val == key2){
                     return (char*) ep; // edge already present
                   }       
                   elist_t *newe = createE(key2);// create a new edge node
                   newe->enext=curre;  // connect newe->next to curr
                   newe->pointv=v; // points to its vertex
                   prede->enext = newe;
                   u->ecount = u->ecount + 1;
                      return (char*) eadd;
     }    
        
// Deletes an edge from the edge-list if present
 bool RemoveE(int key1, int key2){
               elist_t* prede, *curre, *succe;
               vlist_t *u,*v;
               bool flag = ContainsVPlus(&u, &v, key1, key2);
               if(flag == false){             
                  return (char*) vntp; // either of the vertex is not present
               }   
             locateEPlus(&u, &prede, &curre, key2);
                if(curre->val != key2)
                     return (char*) entp; // edge not present
                prede->enext = curre->enext;
               u->ecount = u->ecount + 1; 
           return (char*)er;          
                    
 }    

 
//Contains ENode       
 bool ContainsE(int key1, int key2){
          elist_t *curre, *prede;
          vlist_t *u,*v;
          bool flag = ContainsVPlus(&u, &v, key1, key2);
          if(flag == false){             
               return (char*) entp; // either of the vertex is not present
          }
          locateEPlus(&u, &prede, &curre, key2);
                if(curre->val == key2)
                     return (char*) ep; // edge is present   
                 else
                   return (char*) entp; // edge is not present    

    }             
 // init of the graph having n verteces + C(n,2)/4 edges   
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
bfslist_t* createBFSNode(vlist_t *n, bfslist_t *p, bfslist_t *next){
          bfslist_t * newbfs = (bfslist_t*) malloc(sizeof(bfslist_t));
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
    bfslist_t *bfsNode = createBFSNode(u,NULL,NULL);
    Insert(bfsHead, bfsTail, bfsNode);
    que.push(bfsNode);
    while(!que.empty()){ // run until que is not empty
       bfslist_t *currentVNode = que.front(); // get the front
       que.pop(); // delete from the front
       eHead = currentVNode->n->enext; // Ehead
         for(elist_t * itNode = eHead->enext; itNode->enext != NULL; itNode = itNode ->enext){ // iterate through all adjacency vertices 
                vlist_t *adjVNode = itNode->pointv; // get the vertex
                if(adjVNode && adjVNode->vnext){ // check for vertex is present in the list
                       if(adjVNode == v){ // found the key
                          bfslist_t *bfsNode1 = createBFSNode(adjVNode,currentVNode,NULL);
                          Insert(bfsHead, bfsTail, bfsNode1);
                          return true; // reachable
                         }
                                // if not visted, push in to the queue
                                if(!checkVisited(tid, adjVNode, cnt) ){ 
                                   adjVNode->visitedArray[tid] = cnt;
                                   bfslist_t *bfsNode2 = createBFSNode(adjVNode,currentVNode,NULL);
                                   Insert(bfsHead, bfsTail, bfsNode2);
                                   que.push(bfsNode2);
                                }
                              } 
                            }  
                      } 
                return false;                
       }
  // get path operations     
bool GetPath(int key1, int key2, int tid){
  vlist_t *u,*v;
  bool flag = ContainsVPlus(&u, &v, key1, key2);
  if(flag == false){             
      return (char*) vntp; // either of the vertex is not present
    }   
        vlist_t *  bTail = createV(INT_MAX,1);
      vlist_t *  bHead = createV(INT_MIN,1);
   bfslist_t *BFSHead = createBFSNode(bHead, NULL, NULL); 
 bfslist_t *BFSTail = createBFSNode(bTail,BFSHead, NULL);
 BFSHead->next = BFSTail;
 BFSTail->next = BFSHead;
  bool status = Scan(tid, u, v, *BFSHead, *BFSTail); 
  if(status == true){
    //PrintPath(BFSTail);
    return (char*) pp; // path is present
    }
  else if(status == false)  
          return (char*) pntp; // path is not present
}

bool Scan(int tid, vlist_t *u, vlist_t *v, bfslist_t &bfsHead, bfslist_t &bfsTail){
  vlist_t *  bTail = createV(INT_MAX,1);
      vlist_t *  bHead = createV(INT_MIN,1);
  bfslist_t *oldbfsTreeHead = createBFSNode(bHead, NULL, NULL); 
 bfslist_t *oldbfsTreeTail = createBFSNode(bTail,oldbfsTreeHead, NULL);
 oldbfsTreeHead->next = oldbfsTreeTail;
 oldbfsTreeTail->next = oldbfsTreeHead;
  bfslist_t *newbfsTreeHead = createBFSNode(bHead, NULL, NULL); 
 bfslist_t *newbfsTreeTail = createBFSNode(bTail,newbfsTreeHead, NULL);
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
// compare path method
bool ComparePath(bfslist_t &oldbfsTreeTail, bfslist_t &newbfsTreeTail){
  if(&oldbfsTreeTail == NULL || &newbfsTreeTail == NULL ){
    return false;
  }
  bfslist_t *oldbfsTreeIt = &oldbfsTreeTail;
  bfslist_t *newbfsTreeIt = &newbfsTreeTail;
  do{
    if(oldbfsTreeIt->n != newbfsTreeIt->n || oldbfsTreeIt->n->ecount != newbfsTreeIt->n->ecount || oldbfsTreeIt->p != newbfsTreeIt->p ){
       return false;
    }
    oldbfsTreeIt = oldbfsTreeIt->p;
    newbfsTreeIt = newbfsTreeIt->p;
  }while(oldbfsTreeIt->p != NULL && newbfsTreeIt->p !=NULL );
  
  if(oldbfsTreeIt->n != newbfsTreeIt->n || oldbfsTreeIt->n->ecount != newbfsTreeIt->n->ecount || oldbfsTreeIt->p != newbfsTreeIt->p ){
       return false;
    }
  else
    return true;  
}
// compare tree method
bool CompareTree(bfslist_t &oldbfsTreeHead, bfslist_t &newbfsTreeHead){
  if(&oldbfsTreeHead == NULL || &newbfsTreeHead == NULL ){
    return false;
  }
  bfslist_t *oldbfsTreeIt = &oldbfsTreeHead;
  bfslist_t *newbfsTreeIt = &newbfsTreeHead;
  do{
    if(oldbfsTreeIt->n != newbfsTreeIt->n || oldbfsTreeIt->n->ecount != newbfsTreeIt->n->ecount || oldbfsTreeIt->p != newbfsTreeIt->p ){
       return false;
    }
    oldbfsTreeIt = oldbfsTreeIt->next;
    newbfsTreeIt = newbfsTreeIt->next;
  }while(oldbfsTreeIt->next->n->val != INT_MAX && newbfsTreeIt->next->n->val !=INT_MAX );
  
  if(oldbfsTreeIt->n != newbfsTreeIt->n || oldbfsTreeIt->n->ecount != newbfsTreeIt->n->ecount || oldbfsTreeIt->p != newbfsTreeIt->p ){
       return false;
    }
  else
    return true;  
}

// inssert the bfsNode at the Tail position of the BST-tree
void Insert(bfslist_t &BFSHead, bfslist_t &BFSTail, bfslist_t *node){
      node->next = &BFSTail;
      BFSTail.next->next = node;
      BFSTail.next = node;
  }  
      
 void PrintBFSTree(bfslist_t &BFSHead, bfslist_t &BFSTail){
   bfslist_t * node = &BFSHead;
   while(node->n->val != INT_MAX){
    cout<<"n, p:"<< node->n->val<<" "<<node->p->n->val<<" "<<node->next<<endl;
    node = node->next;
   }
 }
 // print the path starting from Tail node
void PrintBFSPath(bfslist_t &BFSHead, bfslist_t &BFSTail){
   bfslist_t * node = BFSTail.next;
   while(node->p){ // ->n->val != INT_MIN
    coutt<<"n, p:"<< node->n->val<<" "<<node->p->n->val<<endl;
    node = node->p;
   }
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
