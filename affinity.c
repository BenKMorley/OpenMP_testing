#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define N 729
#define reps 100

double a[N][N], b[N][N], c[N];
int jmax[N];

void init1(void);
void init2(void);
void loop1(int, int);
void loop2(int, int);
void valid1(void);
void valid2(void);
int max_index(int*, int);
int *initializer_sizes(int, int);
int *initializer_lower(int, int);

int main(int argc, char *argv[]) {

  int num_threads = atoi(argv[1]);
  /* The user is expected to pass in the number of threads they wish to use as
  an argument to the program */
  omp_set_num_threads(num_threads);

  double start1,start2,end1,end2;
  int r;

  init1();

  start1 = omp_get_wtime(); 

  for (r=0; r<reps; r++){
    int num_remaining = N;

    /* local_sizes[i] gives the number of remaining to be executed in thread i's local set */ 
    int *local_sizes = initializer_sizes(N, num_threads);

    /* local_lower[i] gives the value of i at the start of the next chunk for thread i */
    int *local_lower = initializer_lower(N, num_threads);

    #pragma omp parallel default(none) shared(num_remaining, local_sizes, local_lower, num_threads)
    {
      /* The following variables are initialized in the parallel region and so are private 
      variables */
      int chunksize;
      int lower, upper;
      int thread_num = omp_get_thread_num();

      /* The following 2 variables are to be used as dummy variables */
      int index;
      int temp;

      while(num_remaining >= 0){ /* While there are iterations left to be executed */

        /* It's important for avoidance of race conditions that the code which reads from
        and edits the shared variables is within a critical region */
        #pragma omp critical
        {
          /* Check if the local set is empty or not */
          if(local_sizes[thread_num] > 0){

            /* Initialise next chunk to (1/p) the size of the local set */
            temp = local_sizes[thread_num]/num_threads;

            /* Need to check if this is zero first, to avoid zero chunksize */
            if(temp == 0){
              chunksize = 1;
            }

            else{
              chunksize = temp;
            }

            /* Determine the range of iterables to be executed on the loop */
            lower = local_lower[thread_num];
            upper = lower + chunksize;

            /* Update the shared variables for the next chunk */
            local_lower[thread_num] = upper;
            local_sizes[thread_num] -= chunksize;
            num_remaining -= chunksize;
          }

          /* In the case that this threads local set is empty */
          else{

            /* Determine which local set is the "most loaded" */
            index = max_index(local_sizes, num_threads);

            /* Because of the possibility of multiple threads making it through
            the while loop, need to check that there are iterations left */
            if(num_remaining == 0){
              num_remaining = -1; /* This is a flag */
            }

            else{ /* Execute a chunk of the most loaded threads local set */

              /* Initialise next chunk to (1/p) the size of the local set */
              temp = local_sizes[index]/num_threads;

              /* Need to check if this is zero first, to avoid zero chunksize */
              if(temp == 0){
                chunksize = 1;
              }

              else{
                chunksize = temp;
              }

              /* Determine the range of iterables to be executed on the loop */
              lower = local_lower[index];
              upper = lower + chunksize;

              /* Update the shared variables for the next chunk */
              local_lower[index] = upper;
              local_sizes[index] -= chunksize;
              num_remaining -= chunksize;
            } 
          }
        } /* End of critical region */

        /* Check to make sure there are threads left to execute */
        if(num_remaining >= 0){
          /*printf("Thread %d is about to execute the loop from %d to %d\n", thread_num, lower, upper);*/

          /* Execute the loop from lower to upper, where the lower bound is 
          inclusive and the upper bound is exclusive */
          loop1(lower, upper);
        }
      }
    }
  }


  end1  = omp_get_wtime();  

  printf("\n");

  valid1(); 

  printf("Total time for %d reps of loop 1 = %f\n",reps, (float)(end1-start1)); 

  init2();

  start2 = omp_get_wtime(); 

  for (r=0; r<reps; r++){
    int num_remaining = N;

    /* local_sizes[i] gives the number of remaining to be executed in thread i's local set */ 
    int *local_sizes = initializer_sizes(N, num_threads);

    /* local_lower[i] gives the value of i at the start of the next chunk for thread i */
    int *local_lower = initializer_lower(N, num_threads);

    #pragma omp parallel default(none) shared(num_remaining, local_sizes, local_lower, num_threads)
    {
      /* The following variables are initialized in the parallel region and so are private 
      variables */
      int chunksize;
      int lower, upper;
      int thread_num = omp_get_thread_num();

      /* The following 2 variables are to be used as dummy variables */
      int index;
      int temp;

      while(num_remaining >= 0){ /* While there are iterations left to be executed */

        /* It's important for avoidance of race conditions that the code which reads from
        and edits the shared variables is within a critical region */
        #pragma omp critical
        {
          /* Check if the local set is empty or not */
          if(local_sizes[thread_num] > 0){

            /* Initialise next chunk to (1/p) the size of the local set */
            temp = local_sizes[thread_num]/num_threads;

            /* Need to check if this is zero first, to avoid zero chunksize */
            if(temp == 0){
              chunksize = 1;
            }

            else{
              chunksize = temp;
            }

            /* Determine the range of iterables to be executed on the loop */
            lower = local_lower[thread_num];
            upper = lower + chunksize;

            /* Update the shared variables for the next chunk */
            local_lower[thread_num] = upper;
            local_sizes[thread_num] -= chunksize;
            num_remaining -= chunksize;
          }

          /* In the case that this threads local set is empty */
          else{

            /* Determine which local set is the "most loaded" */
            index = max_index(local_sizes, num_threads);

            /* Because of the possibility of multiple threads making it through
            the while loop, need to check that there are iterations left */
            if(num_remaining == 0){
              num_remaining = -1; /* This is a flag */
            }

            else{ /* Execute a chunk of the most loaded threads local set */

              /* Initialise next chunk to (1/p) the size of the local set */
              temp = local_sizes[index]/num_threads;

              /* Need to check if this is zero first, to avoid zero chunksize */
              if(temp == 0){
                chunksize = 1;
              }

              else{
                chunksize = temp;
              }

              /* Determine the range of iterables to be executed on the loop */
              lower = local_lower[index];
              upper = lower + chunksize;

              /* Update the shared variables for the next chunk */
              local_lower[index] = upper;
              local_sizes[index] -= chunksize;
              num_remaining -= chunksize;
            } 
          }
        } /* End of critical region */

        /* Check to make sure there are threads left to execute */
        if(num_remaining >= 0){
          /*printf("Thread %d is about to execute the loop from %d to %d\n", thread_num, lower, upper);*/

          /* Execute the loop from lower to upper, where the lower bound is 
          inclusive and the upper bound is exclusive */
          loop2(lower, upper);
        }
      }
    }
  }

  end2  = omp_get_wtime(); 

  printf("\n");

  valid2();

  printf("Total time for %d reps of loop 2 = %f\n", reps, (float)(end2-start2));
}

char* translate_schedule(omp_sched_t x){
  if(x == 1) return "static";
  if(x == 2) return "dynamic";
  if(x == 3) return "guided";
  if(x == 4) return "auto";

  return "none of the above";
}

/* Function to find the index of the max element of a one-dimensional input 
array of size size */
int max_index(int *array, int size){
  int max_idx, max_val, new_val;

  if(size == 1){
    printf("Warning: max index called on length 1 array");
  }

  max_idx = 0;
  max_val = array[0];

  for(int i=1; i<size; i++){
    new_val = array[i];

    if(new_val > max_val){
      max_val = new_val;
      max_idx = i;
    }
  }

  return max_idx;
}

void init1(void){
  int i,j; 

  for (i=0; i<N; i++){ 
    for (j=0; j<N; j++){ 
      a[i][j] = 0.0; 
      b[i][j] = 3.142*(i+j); 
    }
  }
}

void init2(void){ 
  int i,j, expr; 

  for (i=0; i<N; i++){ 
    expr =  i%( 3*(i/30) + 1); 
    if ( expr == 0) { 
      jmax[i] = N;
    }
    else {
      jmax[i] = 1; 
    }
    c[i] = 0.0;
  }

  for (i=0; i<N; i++){ 
    for (j=0; j<N; j++){ 
      b[i][j] = (double) (i*j+1) / (double) (N*N); 
    }
  }
} 

/* This array will contain the starting number of iterations for each thread */
int *initializer_sizes(int n, int p){
  int *my_array;
  int divisor = n/p;
  int rem = n - p * divisor;

  my_array = (int *) malloc(p * sizeof(int));

  /* Initialize each threads values in turn */
  for(int t=0; t<p; t++){

    if(rem == 0){
      my_array[t] = divisor;
    }

    else{
      my_array[t] = divisor + 1;
      rem--;
    }
  }
  return my_array;
}

int *initializer_lower(int n, int p){
  int *my_array;
  int divisor = n/p;
  int rem = n - p * divisor;
  int counter = 0;

  my_array = (int *) malloc(p * sizeof(int));

  /* Initialize each threads values in turn */
  for(int t=0; t<p; t++){
    my_array[t] = counter;

    if(rem == 0){
      counter += divisor;
    }

    else{
      counter += divisor + 1;
      rem--;
    }
  }

  return my_array;
}

void loop1(int lower, int upper) { 
  int i,j;
  for (i=lower; i<upper; i++){
    for (j=N-1; j>i; j--){
      a[i][j] += cos(b[i][j]);
    }
  }
}

void loop2(int lower, int upper) {
  int i,j,k; 
  double rN2; 

  rN2 = 1.0 / (double) (N*N);  

  /*#pragma omp parallel default(none)*/
  for (i=lower; i<upper; i++){
    for (j=0; j < jmax[i]; j++){
      for (k=0; k<j; k++){ 
	c[i] += (k+1) * log (b[i][j]) * rN2;
      } 
    }
  }
}

void valid1(void) { 
  int i,j; 
  double suma; 
  
  suma= 0.0; 
  for (i=0; i<N; i++){ 
    for (j=0; j<N; j++){ 
      suma += a[i][j];
    }
  }
  printf("Loop 1 check: Sum of a is %lf\n", suma);
}

void valid2(void) { 
  int i; 
  double sumc; 
  
  sumc= 0.0; 
  for (i=0; i<N; i++){ 
    sumc += c[i];
  }
  printf("Loop 2 check: Sum of c is %f\n", sumc);
}
