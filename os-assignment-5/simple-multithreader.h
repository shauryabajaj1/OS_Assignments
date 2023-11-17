#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <chrono>
#include<pthread.h>


struct AssistArgs1D {
    int start;
    int end;
    std::function<void(int)> Lexp;
    int id;
};

struct AssistArgs2D {
    int start;
    int end;
    int row;
    int col;
    std::function<void(int,int)> Lexp;
    int id;
};

int count1D = 1;
int count2D = 1;


int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */

void demonstration(std::function<void()> && lambda) {
  lambda();
}

int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5
  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main

void* parallel_for_assist1D(void *para){
    auto arguments = static_cast<AssistArgs1D*>(para);
    for (int i = arguments->start; i < arguments->end; i++) {
        arguments->Lexp(i);
    }
    return nullptr;
}


void parallel_for(int ini, int fin, std::function<void(int)> &&givenFnc, int numThreads){
  auto start_time = std::chrono::high_resolution_clock::now();

  if (!givenFnc) {  //Error handling
    perror("Invalid Lamda Function"); }

  if(numThreads <= 0){
    printf("Error : Number of Threads are 0\n");
    exit(0);
  }

  int itrsper = (fin-ini)/numThreads;

  pthread_t threads[numThreads];
  AssistArgs1D assistArgs[numThreads];

  for(int m = 0; m<numThreads; m++){
      if(m == numThreads - 1){
        AssistArgs1D& curr = assistArgs[m];
        curr.end = fin;
        curr.start = m*itrsper;
        curr.Lexp = givenFnc;
        curr.id = m;
      }
      else{
        AssistArgs1D& curr = assistArgs[m];
        curr.end = (m+1)*itrsper;
        curr.start = m*itrsper;
        curr.Lexp = givenFnc;
        curr.id = m;
      }
  }

  
  for(int m = 0; m<numThreads ; m++){
      int result = pthread_create(&threads[m], NULL, parallel_for_assist1D, &assistArgs[m]);
      if (result != 0) {              //Error Handling       
        perror("Error in Thread Creation\n"); }
  }

  for (int m = 0; m < numThreads; m++) {
      int result= pthread_join(threads[m], nullptr);
      if (result != 0) {             //Error Handling          
        perror("Error in Thread Creation\n"); }
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  printf("Time Taken for 1D Call %d: %ld\n",count1D,duration_time.count());
  count1D++;
}


void* parallel_for_assist2D(void *para){
    auto arguments = static_cast<AssistArgs2D*>(para);
    int thisCol = arguments->col; int thisRow = arguments->row;
    for (int i = arguments->start; i < arguments->end ; i++) {
        int col = i%thisRow;
        int row = (i-col)/thisRow;
        arguments->Lexp(row, col);
    }
    return nullptr;
}


void parallel_for(int ini1, int fin1 , int ini2 , int fin2, std::function<void(int,int)> &&givenFnc, int numThreads){

  auto start_time = std::chrono::high_resolution_clock::now();
  
  if (!givenFnc) {  //Error handling
    perror("Invalid Lamda Function"); }

  if(numThreads <= 0){
    printf("Error : number of Threads are 0\n");
    exit(0);
  }

  int itrsper = ((fin1 - ini1)*(fin2 - ini2))/numThreads;
  int row = fin1 - ini1;
  int col = fin2 - ini2;

  pthread_t threads[numThreads];
  AssistArgs2D assistArgs[numThreads];
  
  for(int m = 0; m<numThreads; m++){
      AssistArgs2D curr;
      if(m == numThreads -1){
        curr.start = m*itrsper;
        curr.end = row*col;
        curr.col = col;
        curr.row = row;
        curr.Lexp = givenFnc;
        curr.id = m;
      }
      else{
        curr.start = m*itrsper;
        curr.end = (m+1)*itrsper;
        curr.col = col;
        curr.row = row;
        curr.Lexp = givenFnc;
        curr.id = m;
      }
      assistArgs[m] = curr;      
  }

    for(int m = 0; m<numThreads ; m++){
      int result = pthread_create(&threads[m], NULL, parallel_for_assist2D, &assistArgs[m]);
      if (result != 0) {     //Error Handling
        perror("Error in Thread Creation\n"); }

  }

  for (int m = 0; m < numThreads; m++) {
      int result= pthread_join(threads[m], nullptr);
      if (result != 0) {     //Error Handling      
        perror("Error in Thread Creation\n"); }
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  printf("Time Taken for 2D Call %d: %ld\n",count2D,duration_time.count());  
  count2D++;

}

