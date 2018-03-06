/*
 * File:cgds_coarse.cpp
 *  
 *
 * Author(s):
 *   Dr. Sathya Peri <sathya_p@iith.ac.in>
 *   Muktikanta Sa   <cs15resch11012@iith.ac.in>
 *   Nandini Singhal <cs15mtech01004@iith.ac.in>
 *   
 * Description:
 *   Sequential implementation of a graph
 * Copyright (c) 2017.
 * last Updated: 13/10/2017
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


using namespace std;
#ifndef LLIST_H_ 
#define LLIST_H_


#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
#endif



typedef struct node{
	int val; // data
	struct node *vnext; // pointer to the next vertex
	struct node *enext; // pointer to the next adjancy list
}slist_t;

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



class list{
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
                EHead ->enext = NULL;
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
                slist_t* pred= (slist_t*) malloc(sizeof(slist_t));
                pred->val=0;
                pred->vnext=NULL;
                pred->enext=NULL;
                slist_t *curr= (slist_t*) malloc(sizeof(slist_t));
                curr->val=0;
                curr->vnext=NULL;
                 curr->enext=NULL;
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
       // void print();
      // friend class Graph;
        
};

       // slist_t * g=(slist_t*) malloc(sizeof(slist_t));
       // g->val=0;
       // g->vnext=NULL;
       // g->enext=NULL;
 class Graph{
    list lt;
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
        }
        bool ContainsV(int key){
                 slist_t *g1=(slist_t*) malloc(sizeof(slist_t));
                 g1->val=0;
                 g1->vnext=NULL;
                 g1->enext=NULL;
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
          return lt.Add(u->enext,NULL, key2);  
            
            
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
          int i=0;
          for(;i<n;i++){
            AddV(i);
          }
        
        }
   };     
   


#endif

