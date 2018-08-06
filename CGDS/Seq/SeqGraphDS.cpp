/*
 * File:SeqGraphDS.cpp
 *  
 *
 * Author(s):
 *   Bapi Chatterjee <bapchatt@in.ibm.com>
 *   Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *   
 * Description:
 *   Sequential implementation of a graph
 * Copyright (c) 2018.
 * last Updated: 31/07/2018
 *
*/
#include<iostream>
#include<float.h>
#include<stdint.h>
#include<stdio.h>
#include <stdlib.h>
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

// ofstream coutt("getpath.txt");
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
	
}vlist_t;


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
vlist_t* createV(int key){
          elist_t *EHead = createE(INT_MIN); //create Edge Head
          elist_t *ETail = createE(INT_MAX); // create Edge Tail
          EHead ->enext=ETail; 
          vlist_t * newv = (vlist_t*) malloc(sizeof(vlist_t));
          newv ->val = key;
          newv ->vnext=NULL;
    
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
          //Tail ->ecount=0;
          //Tail ->visitedArray = NULL;
          Head = (vlist_t*) malloc(sizeof(vlist_t));
          Head ->val = INT_MIN;
          Head ->enext=NULL;
          //Head ->ecount=0;
         // Head ->visitedArray = NULL;
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
    
     // add a new vertex in the vertex-list
 bool AddV(int key){
 
      vlist_t* predv, *currv;

        locateVPlus(Head, &predv, &currv, key); // find the location, <pred, curr>
    
        if(currv->val == key){
           return false; // key already present
        }       
        else{
           vlist_t *newv = createV(key); // create a new vertex node
           newv->vnext=currv;  
           predv->vnext = newv;// adds in the list

               return true;
           }

        
  }
    
// Deletes the vertex from the vertex-list
 bool RemoveV(int key){
 //cout<<" r:"<<key;
        vlist_t* predv, *currv, *succv;
        elist_t * prede, *curre;

      	locateVPlus(Head, &predv, &currv, key);
	if(currv->val != key)
        	return false; // key is not present
	 predv->vnext = currv->vnext;

	// delete incomming edges 
	vlist_t *temp1 = Head->vnext;
          while(temp1 != Tail){
              locateEPlus(temp1->enext, &prede, &curre, key);
                if(curre->val == key)
                        prede->enext = curre->enext;
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
void locateEPlus(elist_t *startE, elist_t ** n1, elist_t ** n2, int key){
       elist_t *succe, *curre, *prede;
        prede = startE;
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
                  return false; // either of the vertex is not present
               }   
                locateEPlus(u->enext, &prede, &curre, key2);
                if(curre->val == key2){
                     return false; // edge already present
                   }       
                   elist_t *newe = createE(key2);// create a new edge node
                   newe->enext=curre;  // connect newe->next to curr
                   newe->pointv=v; // points to its vertex
                   prede->enext = newe;

                      return true;
     }    
        
// Deletes an edge from the edge-list if present
 bool RemoveE(int key1, int key2){
               elist_t* prede, *curre, *succe;
               vlist_t *u,*v;
               bool flag = ContainsVPlus(&u, &v, key1, key2);
               if(flag == false){             
                  return false; // either of the vertex is not present
               }   
             locateEPlus(u->enext, &prede, &curre, key2);
                if(curre->val != key2)
                     return false; // edge not present
                prede->enext = curre->enext;

           return true;          
                    
 }    

 
//Contains ENode       
 bool ContainsE(int key1, int key2){
          elist_t *curre, *prede;
          vlist_t *u,*v;
          bool flag = ContainsVPlus(&u, &v, key1, key2);
          if(flag == false){             
               return false; // either of the vertex is not present
          }
          locateEPlus(u->enext, &prede, &curre, key2);
                if(curre->val == key2)
                     return true; // edge is present   
                 else
                   return false; // edge is not present    

    }             

 void initGraph(int n){
     int i,j;
     for( i=1;i<=n;i++){
         AddV(i);
       }
     /*  
     for( i=1;i<=n;i++){
         for( j=i+1; j<=n; j++)
          if(i!=j)
          AddE(i,j);
       }
      */   
   } 
      

};
