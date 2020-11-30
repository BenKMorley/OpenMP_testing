#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define N 729
#define reps 100

double a[N][N], b[N][N], c[N];
int jmax[N];

void init1(void);
void init2(void);
void loop1(void);
void loop2(void);
void valid1(void);
void valid2(void);
char* translate_schedule(omp_sched_t);

int main(int argc, char *argv[]) {
  double start1,start2,end1,end2;
  int r;

  omp_sched_t my_schedule;
  int chunksize;

  omp_get_schedule(&my_schedule, &chunksize);

  printf("The schedule is :%s, with chunksize %d\n", translate_schedule(my_schedule), chunksize);

  init1();

  start1 = omp_get_wtime(); 

  for (r=0; r<reps; r++){
    loop1();
  }

  end1  = omp_get_wtime();  

  valid1(); 

  printf("Total time for %d reps of loop 1 = %f\n",reps, (float)(end1-start1)); 

  init2(); 

  start2 = omp_get_wtime(); 

  for (r=0; r<reps; r++){ 
    loop2();
  }

  end2  = omp_get_wtime(); 

  valid2(); 

  printf("Total time for %d reps of loop 2 = %f\n",reps, (float)(end2-start2));
}

char* translate_schedule(omp_sched_t x){
  if(x == 1) return "static";
  if(x == 2) return "dynamic";
  if(x == 3) return "guided";
  if(x == 4) return "auto";

  return "none of the above";
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

void loop1(void) { 
  int i,j;

  /* schedule(runtime) is used so that the exact OpenMP schedule implemented 
  can be controlled using the OpenMP environmental variables in BASH. This
  allows this script to be used for all inbuit OpenMP scheduling methods */
  #pragma omp parallel for default(none) shared(a, b) private(i, j) schedule(runtime)
  for (i=0; i<N; i++){
    for (j=N-1; j>i; j--){
      a[i][j] += cos(b[i][j]);
    }
  }
}

void loop2(void) {
  int i,j,k; 
  double rN2; 

  rN2 = 1.0 / (double) (N*N);  

  /* schedule(runtime) is used so that the exact OpenMP schedule implemented 
  can be controlled using the OpenMP environmental variables in BASH. This
  allows this script to be used for all inbuit OpenMP scheduling methods */
  #pragma omp parallel for default(none) shared(b, c, rN2, jmax) private(i, j, k) schedule(runtime)
  for (i=0; i<N; i++){
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
