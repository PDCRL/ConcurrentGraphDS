/*
 * File:acyclic_lazy.cpp
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
#include <list>
//#include <stdatomic.h>


#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
#endif
enum type1 {transit, marked, added};

using namespace std;
typedef struct node{
	int val; // data
	atomic<int> status; 
	pthread_mutex_t lock;
	atomic<struct node *> vnext; // pointer to the next entry
	atomic<struct node *>enext; // pointer to the next adjancy list
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


int x;
class list1{
 public:
 
    slist_t *Head, *Tail;
  // for initilization of the list
 void init(){
  Head = (slist_t*) malloc(sizeof(slist_t));
  Head ->val = INT_MIN;
  pthread_mutex_init(&Head->lock, NULL);   
  Head ->vnext.store(NULL,memory_order_seq_cst);
  Head ->enext.store(NULL,memory_order_seq_cst);
  Head ->status.store(added,memory_order_seq_cst);
  Tail = (slist_t*) malloc(sizeof(slist_t));
  Tail ->val = INT_MAX;
  pthread_mutex_init(&Tail->lock, NULL);   
  Tail ->vnext.store(NULL,memory_order_seq_cst);
  Tail ->enext.store(NULL,memory_order_seq_cst);
  Tail ->status.store(added,memory_order_seq_cst);
  Head->vnext.store(Tail,memory_order_seq_cst);
}
        slist_t* createV(int key){
                slist_t* EHead = (slist_t*) malloc(sizeof(slist_t));
                EHead ->val = INT_MIN;
                pthread_mutex_init(&EHead->lock, NULL);   
                EHead ->vnext.store(NULL,memory_order_seq_cst);
                EHead ->enext.store(NULL,memory_order_seq_cst);
                EHead ->status.store(added,memory_order_seq_cst);
                
                slist_t *ETail = (slist_t*) malloc(sizeof(slist_t));
                ETail ->val = INT_MAX;
                pthread_mutex_init(&ETail->lock, NULL);   
                ETail ->vnext.store(NULL,memory_order_seq_cst);
                ETail ->enext.store(NULL,memory_order_seq_cst);
                ETail ->status.store(added,memory_order_seq_cst);
                EHead->enext.store(ETail,memory_order_seq_cst);
                
                slist_t * temp = (slist_t*) malloc(sizeof(slist_t));
                temp ->val = key;
                pthread_mutex_init(&temp->lock, NULL);   
                temp ->vnext.store(NULL,memory_order_seq_cst);
                temp ->enext.store(NULL,memory_order_seq_cst);
                temp ->status.store(added,memory_order_seq_cst);
                temp ->enext.store(EHead,memory_order_seq_cst);
                return temp;
        }
        slist_t* createE(int key){
                slist_t * temp = (slist_t*) malloc(sizeof(slist_t));
                temp ->val = key;
                pthread_mutex_init(&temp->lock, NULL);   
                temp ->vnext.store(NULL,memory_order_seq_cst);
                temp ->enext.store(NULL,memory_order_seq_cst);
                temp ->status.store(transit,memory_order_seq_cst); // created with tranist
                return temp;
        }
        bool ValidateV(slist_t *pred, slist_t *curr){
                if((pred->status.load(memory_order_seq_cst) == added) && (curr->status.load(memory_order_seq_cst) == added) && (pred->vnext.load(memory_order_seq_cst) == curr))
                        return true;
                else
                        return false; 
               }
        bool ValidateE(slist_t *pred, slist_t *curr){
                if((pred->status.load(memory_order_seq_cst) != marked) && (curr->status.load(memory_order_seq_cst) != marked) && (pred->enext.load(memory_order_seq_cst) == curr))
                        return true;
                else
                        return false; 
               }  
         bool ValidateAE(slist_t *pred, slist_t *curr){
                if((pred->status.load(memory_order_seq_cst) != marked) && (curr->status.load(memory_order_seq_cst) == transit) && (pred->enext.load(memory_order_seq_cst) == curr))
                        return true;
                else
                        return false; 
               }                     
        void locate(slist_t* head, slist_t* tail, slist_t ** pred, slist_t ** curr, int key){
                  //         cout<<"loc begins"<<endl;
             //slist_t* pred = head; 
               // slist_t* curr;
               (*pred)=head;
             if(head == Head){ // locate in the vertex list
               (*curr) = (*pred)->vnext.load(memory_order_seq_cst); 
                while((*curr) != Tail && (*curr)->val < key){
                    (*pred) = (*curr);
                    (*curr) = (*curr)->vnext.load(memory_order_seq_cst);
                 }
                 pthread_mutex_lock(&(*pred)->lock);
                 pthread_mutex_lock(&(*curr)->lock);
                 if(ValidateV((*pred), (*curr))){  // vertex validate
                  // (*n1) = pred;
                  // (*n2) = curr;
                   //cout<<"curr:"<<curr->val;
                   //cin >> x;
                    return;
                  }
                else{
                     pthread_mutex_unlock(&(*curr)->lock);
                     pthread_mutex_unlock(&(*pred)->lock);  
                   }
                   
                 }
                else{   // locate in the edge list of the vertex 
                       (*curr) = (*pred)->enext.load(memory_order_seq_cst); 
                        while((*curr) != tail && (*curr)->val < key){
                        (*pred) = (*curr);
                        (*curr) = (*curr)->enext.load(memory_order_seq_cst);
                       }
                  pthread_mutex_lock(&(*pred)->lock);
                 pthread_mutex_lock(&(*curr)->lock);
                 if(ValidateE((*pred), (*curr))){ // edge validate 
                   //(*n1) = pred;
                   //(*n2) = curr;
                     // cout<<"curr:"<<curr->val;
                    return;
                  }
                else{
                     pthread_mutex_unlock(&(*curr)->lock);
                     pthread_mutex_unlock(&(*pred)->lock);  
                   }
                
                }
        }
        
       
        
         void locateAE(slist_t* head, slist_t ** pred, slist_t ** curr, int key){
                // locate in the edge list of the vertex 
                (*pred) = head;
                 (*curr) = (*pred)->enext.load(memory_order_seq_cst); 
                  while((*curr)->enext.load(memory_order_seq_cst) != NULL && (*curr)->val < key){
                        (*pred) = (*curr);
                        (*curr) = (*curr)->enext.load(memory_order_seq_cst);
                       }
                  pthread_mutex_lock(&(*pred)->lock);
                  pthread_mutex_lock(&(*curr)->lock);
                  if(ValidateE((*pred), (*curr))){ // edge validate 
                   
                    return;
                  }
                else{
                     pthread_mutex_unlock(&(*curr)->lock);
                     pthread_mutex_unlock(&(*pred)->lock);  
                   }
                
                }
        
        void locateAEStatus(slist_t* head, slist_t ** pred, slist_t ** curr, int key){
                // locate in the edge list of the vertex 
                (*pred) = head;
                 (*curr) = (*pred)->enext.load(memory_order_seq_cst); 
                  while((*curr)->enext.load(memory_order_seq_cst) != NULL && (*curr)->val < key){
                        (*pred) = (*curr);
                        (*curr) = (*curr)->enext.load(memory_order_seq_cst);
                       }
                  pthread_mutex_lock(&(*pred)->lock);
                  pthread_mutex_lock(&(*curr)->lock);
                  if(ValidateAE((*pred), (*curr))){ // edge validate 
                   
                    return;
                  }
                else{
                     pthread_mutex_unlock(&(*curr)->lock);
                     pthread_mutex_unlock(&(*pred)->lock);  
                   }
                
                }
        
       
         /* wait-free contains */
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
                 if((curr->val == key) && (curr->status.load(memory_order_seq_cst) == added)){
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
                         if((curr->val == key) && (curr->status.load(memory_order_seq_cst) == added)){
                                (*n) = curr; 
                                return true;       // found it, return success, true
                          }
                        else{
                                return false; // key not present
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
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return true;
                }
                else{
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
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
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return true;
                }
                else{
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                 return false;
              }   
          }  
          //}
   }
    bool AddAE(slist_t * h, int key){
                // allcate new node
                slist_t* pred, *curr;
                slist_t *newe = createE(key);// create a new edge node
                locateAE(h,&pred, &curr, key);
               //if(curr !=NULL){
                if(curr->val != key){
                        //newe ->status.store(transit,memory_order_seq_cst);  // make the status to transit
                        newe ->enext.store(curr,memory_order_seq_cst);  
                        pred ->enext.store(newe,memory_order_seq_cst); // adding in the list, LP
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return true;
                }
                else{
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                 return false;
              }   
            
          //}
   }
   
    bool changeStatusAE(slist_t * h, int key){
                // allcate new node
                slist_t* pred, *curr;
                locateAEStatus(h,&pred, &curr, key);
               
                if(curr->val == key){
                        curr ->status.store(added, memory_order_seq_cst);  // make the status to transit
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return true;
                }
                else{
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                 return false;
              }   
            
          //}
   }
//RemoveAE(u->enext, key2)
 bool RemoveAE(slist_t *h, int key){
        slist_t* pred, *curr;
       locateAE(h, &pred, &curr, key);

         if(curr->val == key){// edge delete
                        curr->status.store(marked, memory_order_seq_cst); // logical delete, LP
                        pred ->enext.store(curr->enext.load(memory_order_seq_cst), memory_order_seq_cst); // Physcial delete
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return true;
                }
                else{
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return false;
                  }      
     } 

    bool Remove(slist_t *h, slist_t *t, int key){
        slist_t* pred, *curr;
        if(h==Head){// vertex delete
            //  if(curr !=NULL){
             locate(h, t, &pred, &curr, key);
                if(curr->val == key){
                        curr->status.store(marked, memory_order_seq_cst); // logical delete, LP
                        pred ->vnext.store(curr->vnext.load(memory_order_seq_cst),memory_order_seq_cst); // physical delete
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return true;
                }
                else{
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return false;
                  }      
        //}
        }
         else{
          locate(h, t, &pred, &curr, key);
        // if(curr !=NULL){
         if(curr->val == key){// edge delete
                        curr->status.store(marked, memory_order_seq_cst); // logical delete, LP
                        pred ->enext.store(curr->enext.load(memory_order_seq_cst),memory_order_seq_cst); // Physcial delete
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
                        return true;
                }
                else{
                     pthread_mutex_unlock(&(curr)->lock);
                     pthread_mutex_unlock(&(pred)->lock);  
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
            bool flag1 = WFContains(Head, Tail, &temp, s);  
          if(flag1 == true){
               temp = temp->enext.load(memory_order_seq_cst); // Ehead
                for(it = temp->enext.load(memory_order_seq_cst); it->enext.load(memory_order_seq_cst) != NULL; it = it ->enext){
                 if(it->status != marked){  // either transit or added, 
                   bool flag2 = WFContains(Head, Tail, &temp2, it->val);  
                   if(flag2 == true && temp2->status != marked){ // check for vertex is present in the list,
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
                 //}
                }
              }  
        return false;                
     }
     
        /*
        bool isReachable(slist_t *u, slist_t *v, int n){
        slist_t * temp, *it, *temp1;
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
            bool flag1 = Contains(Head, Tail, &temp, s);  
          if(flag1 == true){
               temp = temp->enext.load(memory_order_seq_cst); // Ehead
               for(it = temp->enext.load(memory_order_seq_cst); it->enext.load(memory_order_seq_cst) != NULL; it = it ->enext.load(memory_order_seq_cst)){
               bool flag2 = Contains(Head, Tail, &temp1, it->val); // check corresponding vertex is present or not, if no IDE
                if(flag2 == true && it->status == added){ // process only added edges
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
        return false;                
     }
     */
     /* cycle detection for whole graph, from Head to Tail*/
      bool isCycle(){
        slist_t * temp=Head->vnext.load(memory_order_seq_cst), *temp1, *it, *curr;
         while(temp!=Tail){
            temp1 = temp->vnext.load(memory_order_seq_cst);
           while(temp1 != Tail){
              
              if(isReachable(temp1, temp))
              {
             // cout<<"reachable";
              if(WFContains(temp->enext.load(memory_order_seq_cst), NULL,  &curr, temp1->val )){
                        cout<<" cycle:"<<temp->val<<" "<<temp1->val<<endl;
                return true; // cycle is present
               } 
              }  
             temp1 = temp1->vnext.load(memory_order_seq_cst);   
          }
          temp = temp->vnext.load(memory_order_seq_cst);
         }
         return false;                
        } 
     
        
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
                 if(lt.WFContains(lt.Head, lt.Tail, &g1, key)){
                        //(*n) = &g1;                  
                   return true;
                   }
                 else
                    return false;  
        }
        //void locateV(slist_t ** n1, slist_t ** n2, int key);
        bool AddE(int key1, int key2){
          slist_t *u,*v;
          bool flag1 = lt.WFContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.WFContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.WFContains(lt.Head, lt.Tail, &u, key1);  
          if(flag1 == false)
            return false;
          bool status1 = lt.AddAE(u->enext, key2);// logical addition  
          if(status1 == true){
          
              bool c = lt.isReachable(v, u);
              if(c == true){
                 lt.RemoveAE(u->enext, key2); // delete from the list
                return false;
                }
              else{
                lt.changeStatusAE(u->enext, key2);// physical addition
                return true;
              }
              
            }      
               
           
           return status1;   
          //return lt.Add(u->enext,NULL, key2);  
        }
        bool RemoveE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.WFContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.WFContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          flag1 = lt.WFContains(lt.Head, lt.Tail, &u, key1);  
          if(flag1 == false)
            return false;
          return lt.Remove(u->enext,NULL, key2);  
         }
        bool ContainsE(int key1, int key2){
         slist_t *u,*v;
          bool flag1 = lt.WFContains(lt.Head, lt.Tail, &u, key1);
          bool flag2 = lt.WFContains(lt.Head, lt.Tail, &v, key2);
          if(flag1 == false || flag2 == false)
            return false;
          return lt.WFContains(u->enext,NULL, &v, key2);  
        
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
          for(int i=1;i<n;i++){
            AddV(i);
          }
        
        }
        bool checkCycle(){
         return lt.isCycle();
       } 
   };     
   


#endif

