/*
 * File:cgds_HoH.cpp
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
 * last Updated: 14/10/2017
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
//#include <stdatomic.h>


#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
#endif


using namespace std;
typedef struct node{
	int val; // data
	//atomic<bool> marked;
	pthread_mutex_t lock;
	atomic<struct node *> vnext; // pointer to the next entry
	atomic<struct node *>enext; // pointer to the next adjancy list
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


int x;
class list{
 public:
 
    slist_t *Head, *Tail;
  // for initilization of the list
 void init(){
  Head = (slist_t*) malloc(sizeof(slist_t));
  Head ->val = INT_MIN;
  Head ->vnext.store(NULL,memory_order_seq_cst);
  Head ->enext.store(NULL,memory_order_seq_cst);
  //Head ->marked.store(false,memory_order_seq_cst);
  Tail = (slist_t*) malloc(sizeof(slist_t));
  Tail ->val = INT_MAX;
  Tail ->vnext.store(NULL,memory_order_seq_cst);
  Tail ->enext.store(NULL,memory_order_seq_cst);
  //Tail ->marked.store(false,memory_order_seq_cst);
  Head->vnext.store(Tail,memory_order_seq_cst);
}
        slist_t* createV(int key){
                slist_t* EHead = (slist_t*) malloc(sizeof(slist_t));
                EHead ->val = INT_MIN;
                EHead ->vnext.store(NULL,memory_order_seq_cst);
                EHead ->enext.store(NULL,memory_order_seq_cst);
                //EHead ->marked.store(false,memory_order_seq_cst);
                
                slist_t *ETail = (slist_t*) malloc(sizeof(slist_t));
                ETail ->val = INT_MAX;
                ETail ->vnext.store(NULL,memory_order_seq_cst);
                ETail ->enext.store(NULL,memory_order_seq_cst);
                //ETail ->marked.store(false,memory_order_seq_cst);
                EHead->enext.store(ETail,memory_order_seq_cst);
                
                slist_t * temp = (slist_t*) malloc(sizeof(slist_t));
                temp ->val = key;
                temp ->vnext.store(NULL,memory_order_seq_cst);
                temp ->enext.store(NULL,memory_order_seq_cst);
                //temp ->marked.store(false,memory_order_seq_cst);
                temp ->enext.store(EHead,memory_order_seq_cst);
                return temp;
        }
        slist_t* createE(int key){
                slist_t * temp = (slist_t*) malloc(sizeof(slist_t));
                temp ->val = key;
                temp ->vnext.store(NULL,memory_order_seq_cst);
                temp ->enext.store(NULL,memory_order_seq_cst);
                //temp ->marked.store(false,memory_order_seq_cst);
                return temp;
        }
                       
        void locate(slist_t* head, slist_t* tail, slist_t ** pred, slist_t ** curr, int key){
                  //         cout<<"loc begins"<<endl;
             //slist_t* pred = head; 
               // slist_t* curr;
               (*pred)=head;
             if(head == Head){ // locate in the vertex list
               pthread_mutex_lock(&(*pred)->lock); 
               (*curr) = (*pred)->vnext.load(memory_order_seq_cst); 
               pthread_mutex_lock(&(*curr)->lock);
                while((*curr) != Tail && (*curr)->val < key){
                    pthread_mutex_unlock(&(*pred)->lock);    
                    (*pred) = (*curr);
                    (*curr) = (*curr)->vnext.load(memory_order_seq_cst);
                    pthread_mutex_lock(&(*curr)->lock);
                 }
                 //pthread_mutex_lock(&(*pred)->lock);
                 //pthread_mutex_lock(&(*curr)->lock);
                // if(ValidateV((*pred), (*curr))){  // vertex validate
                  // (*n1) = pred;
                  // (*n2) = curr;
                   //cout<<"curr:"<<curr->val;
                   //cin >> x;
                  //  return;
                  //}
                //else{
                 //    pthread_mutex_unlock(&(*pred)->lock);
                 //    pthread_mutex_unlock(&(*curr)->lock);  
                 //  }
                 return;
                   
                 }
                else{   // locate in the edge list of the vertex 
                         pthread_mutex_lock(&(*pred)->lock); 
                       (*curr) = (*pred)->enext.load(memory_order_seq_cst); 
                        pthread_mutex_lock(&(*curr)->lock);
                        while((*curr) != tail && (*curr)->val < key){
                        pthread_mutex_unlock(&(*pred)->lock);    
                        (*pred) = (*curr);
                        (*curr) = (*curr)->enext.load(memory_order_seq_cst);
                         pthread_mutex_lock(&(*curr)->lock);
                       }
                 // pthread_mutex_lock(&(*pred)->lock);
                 //pthread_mutex_lock(&(*curr)->lock);
                 //if(ValidateE((*pred), (*curr))){ // edge validate 
                   //(*n1) = pred;
                   //(*n2) = curr;
                     // cout<<"curr:"<<curr->val;
                    return;
                 // }
               // else{
                   //  pthread_mutex_unlock(&(*pred)->lock);
                 //    pthread_mutex_unlock(&(*curr)->lock);  
                  // }
                
                }
        }
                 /* blocking contains */
        bool Contains(slist_t * h, slist_t *t, slist_t ** n, int key){
           //cout<<"con begins"<<endl;
                slist_t* pred= (slist_t*) malloc(sizeof(slist_t));
                pred->val=0;
                pred ->vnext.store(NULL,memory_order_seq_cst);
                pred ->enext.store(NULL,memory_order_seq_cst);
                //pred ->marked.store(false,memory_order_seq_cst);
                slist_t *curr= (slist_t*) malloc(sizeof(slist_t));
                curr->val=0;
                curr ->vnext.store(NULL,memory_order_seq_cst);
                curr ->enext.store(NULL,memory_order_seq_cst);
                //curr ->marked.store(false,memory_order_seq_cst);
                locate(h, t, &pred, &curr, key);
                 //if(h==Head){
                /*if(Head->vnext == Tail){
                        return false;
                }*/
                if(curr->val == key){
                        (*n) = curr; 
                         pthread_mutex_unlock(&pred->lock);
                        pthread_mutex_unlock(&curr->lock);  
                        return true;       // found it, return success, true
                }
                else{
                         pthread_mutex_unlock(&pred->lock);
                     pthread_mutex_unlock(&curr->lock);  
                        return false; // key not present
                     }   
                
        }
         /* wait-free contains */
         /*
               bool WFContains(slist_t * h, slist_t *t, slist_t ** n, int key){
           //cout<<"con begins"<<endl;
                slist_t* pred = h; 
                slist_t* curr;
             if(h == Head){ // search in the vertex list
               curr = pred->vnext.load(memory_order_seq_cst); 
                while(curr != Tail && curr->val < key){
                    pred = curr;
                    curr = curr->vnext.load(memory_order_seq_cst);
                 }
                 if((curr->val == key) && (curr->marked.load(memory_order_seq_cst) == false)){
                        (*n) = curr; 
                        return true;       // found it, return success, true
                }
                 else{
                        return false; // key not present
                 }
                   
                 }
                else{   // search in the edge list
                       curr = pred->enext.load(memory_order_seq_cst); 
                        while(curr != t && curr->val < key){
                            pred = curr;
                            curr = curr->enext.load(memory_order_seq_cst);
                         }
                         if((curr->val == key) && (curr->marked.load(memory_order_seq_cst) == false)){
                                (*n) = curr; 
                                return true;       // found it, return success, true
                          }
                        else{
                                return false; // key not present
                        }
                
                }
                
        }
        */
        
         bool HoHContains(slist_t * h, slist_t *t, slist_t ** n, int key){
          slist_t *curr, *pred;
          if(h == Head){ // search in the vertex list
                pred = Head;
	        locate(h, t, &pred, &curr, key);
	        if(curr->val != key){
	                pthread_mutex_unlock(&(pred)->lock);
                        pthread_mutex_unlock(&(curr)->lock);  
		        return false;
		        }
	        else {
	                (*n) = curr; 
	                pthread_mutex_unlock(&(pred)->lock);
                        pthread_mutex_unlock(&(curr)->lock);  	                
		        return true;
		     }   
          }
          else{   // search in the edge list
             pred = h;
	        locate(h, t, &pred, &curr, key);
	        if( curr->val != key){
	                pthread_mutex_unlock(&(pred)->lock);
                        pthread_mutex_unlock(&(curr)->lock);  
		        return false;
		   }     
	        else {
	                (*n) = curr; 
	                pthread_mutex_unlock(&(pred)->lock);
                        pthread_mutex_unlock(&(curr)->lock);  	  	                
		        return true;
		    }   
          }
         }

        bool Add(slist_t * h, slist_t *t, int key){
                // allcate new node
                slist_t* pred, *curr;
               // locate(h, t, &pred, &curr, key);
                //cout<<"pred->val:"<<pred->val<<" curr->val:"<<curr->val<<endl;
                if(h == Head){ // vertex add
                slist_t *newv = createV(key); // create a new vertex node
                //cout<<curr->val<<" ";
              //  if(curr != NULL){
               
               locate(h, t, &pred, &curr, key);
               
                if(curr->val != key){
                        newv ->vnext.store(curr,memory_order_seq_cst);  
                        pred ->vnext.store(newv,memory_order_seq_cst);// actual addition in the vertex list, LP
                        pthread_mutex_unlock(&pred->lock);
                        pthread_mutex_unlock(&curr->lock);  
                        return true;
                }
                else{
                   pthread_mutex_unlock(&pred->lock);
                   pthread_mutex_unlock(&curr->lock);                       
                 return false;
                }
               }
               //}
               else{ // edge add
               slist_t *newe = createE(key);// create a new edge node
                locate(h, t, &pred, &curr, key);
               //if(curr !=NULL){
                if(curr->val != key){
                        newe ->enext.store(curr,memory_order_seq_cst);  
                        pred ->enext.store(newe,memory_order_seq_cst); // adding in the list, LP
                        pthread_mutex_unlock(&pred->lock);
                        pthread_mutex_unlock(&curr->lock);  
                        return true;
                }
                else{
                   pthread_mutex_unlock(&pred->lock);
                   pthread_mutex_unlock(&curr->lock);                       
                 return false;
              }   
          }  
          //}
   }

    bool Remove(slist_t *h, slist_t *t, int key){
        slist_t* pred, *curr;
        //locate(h, t, &pred, &curr, key);
        if(h==Head){// vertex delete
            //  if(curr !=NULL){
             locate(h, t, &pred, &curr, key);
                if(curr->val == key){
                        //curr->marked.store(true, memory_order_seq_cst); // logical delete, LP
                        pred ->vnext.store(curr->vnext.load(memory_order_seq_cst),memory_order_seq_cst); // physical delete
                        pthread_mutex_unlock(&pred->lock);
                        pthread_mutex_unlock(&curr->lock); 
                        return true;
                }
                else{
                        pthread_mutex_unlock(&pred->lock);
                        pthread_mutex_unlock(&curr->lock); 
                        return false;
                  }      
        //}
        }
         else{
          locate(h, t, &pred, &curr, key);
        // if(curr !=NULL){
         if(curr->val == key){// edge delete
                        //curr->marked.store(true, memory_order_seq_cst); // logical delete, LP
                        pred ->enext.store(curr->enext.load(memory_order_seq_cst),memory_order_seq_cst); // Physcial delete
                        pthread_mutex_unlock(&pred->lock);
                        pthread_mutex_unlock(&curr->lock); 
                        return true;
                }
                else{
                        pthread_mutex_unlock(&pred->lock);
                        pthread_mutex_unlock(&curr->lock); 
                        return false;
                  }      
         
         }
         //}
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
                 g1 ->vnext.store(NULL,memory_order_seq_cst);
                g1 ->enext.store(NULL,memory_order_seq_cst);
               // g1 ->marked.store(false,memory_order_seq_cst);
                 
                 if(lt.HoHContains(lt.Head, lt.Tail, &g1, key)){
                        //(*n) = &g1;                  
                   return true;
                   }
                 else
                    return false;  
        }
        //void locateV(slist_t ** n1, slist_t ** n2, int key);
        bool AddE(int key1, int key2){
          slist_t *u,*v;
          bool flag1 = lt.HoHContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.HoHContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.HoHContains(lt.Head, lt.Tail, &u, key1);  
          if(flag1 == false)
            return false;
          return lt.Add(u->enext,NULL, key2);  
        }
        bool RemoveE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.HoHContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.HoHContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.HoHContains(lt.Head, lt.Tail, &u, key1);  
          if(flag1 == false)
            return false;
          return lt.Remove(u->enext,NULL, key2);  
         }
        bool ContainsE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.HoHContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.HoHContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          return lt.HoHContains(u->enext,NULL, &v, key2);  
        
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
        void initGraph(int n){
          int i=0;
          for(;i<n;i++){
            AddV(i);
          }
       }
   };     
   


#endif

