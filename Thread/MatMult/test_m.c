#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>

struct timeval start, fin;

int SIZE, NTHREADS;
int **A, **B, **C, **_C;

void init()
{
    int i, j;

    A = (int**)malloc(SIZE * sizeof(int *));
    for(i = 0; i < SIZE; i++)
        A[i] = malloc(SIZE * sizeof(int));

    B = (int**)malloc(SIZE * sizeof(int *));
    for(i = 0; i < SIZE; i++)
        B[i] = malloc(SIZE * sizeof(int));

    C = (int**)malloc(SIZE * sizeof(int *));
    for(i = 0; i < SIZE; i++)
        C[i] = malloc(SIZE * sizeof(int));

    _C = (int**)malloc(SIZE * sizeof(int *));
    for(i = 0; i < SIZE; i++)
        _C[i] = malloc(SIZE * sizeof(int));

    srand(time(NULL));

    for(i = 0; i < SIZE; i++) {
        for(j = 0; j < SIZE; j++) {
            A[i][j] = rand()%100;
            B[i][j] = rand()%100;
        }
    }
}

void seq_mm()
{
	for(int i=0; i < SIZE; i++){
		for(int j=0; j < SIZE; j++){
			//_C[i][j] = 0;
			for(int k=0; k < SIZE; k++)
				_C[i][j] += A[i][k]*B[k][j];
		}
	}
}

void mm(int tid)
{
    int i, j, k;
    int start = tid * SIZE/NTHREADS;
    int end = (tid+1) * (SIZE/NTHREADS) - 1;

    for(i = start; i <= end; i++) {
        for(j = 0; j < SIZE; j++) {
            C[i][j] = 0;
            for(k = 0; k < SIZE; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void *worker(void *arg)
{
    int tid = (int)arg;
    mm(tid);
}

int main(int argc, char* argv[])
{

    FILE *fp;
    fp = fopen("result.csv","a+");
    fprintf(fp,"# of size(thread),Exec(seq),Exec(multi)\n");
    pthread_t* threads;
    int rc, i;
/*
    if(argc != 3)
    {
        printf("Usage: %s <size_of_square_matrix> <number_of_threads>\n", argv[0]);
        exit(1);
    }

    SIZE = atoi(argv[1]);
    NTHREADS = atoi(argv[2]);
 */
  int step = 50;
  for(int tc = 10; tc <= 2500; tc+=step)
  {
    printf("\n=============== SIZE %d  ==================\n",tc);
    if(tc >= 100 && tc <= 1000) step = 100;
    else if(tc> 1000) step = 500;
    SIZE = tc;
    NTHREADS = tc;    
    init();
    threads = (pthread_t*)malloc(NTHREADS * sizeof(pthread_t));



    gettimeofday(&start, NULL);
    for(i = 0; i < NTHREADS; i++) {
        rc = pthread_create(&threads[i], NULL, worker, (void *)i);
        assert(rc == 0);
    }

    for(i = 0; i < NTHREADS; i++) {
        rc = pthread_join(threads[i], NULL);
        assert(rc == 0);
    } 

    gettimeofday(&fin, NULL);


    double el_time = (double)(fin.tv_sec - start.tv_sec) + (double)(fin.tv_usec-start.tv_usec)*0.000001;
    printf("[Thread]Elapsed time: %lf sec.\n", el_time); 
 

    gettimeofday(&start, NULL);
    seq_mm();
    gettimeofday(&fin, NULL);
    
    double _el_time = (double)(fin.tv_sec - start.tv_sec) + (double)(fin.tv_usec-start.tv_usec)*0.000001;
    printf("[Seq]Elapsed time: %lf sec.\n", _el_time);

    int err = 0;
    for(int i = 0; i < SIZE; i++)
	    for(int j = 0; j < SIZE; j++)
		err += _C[i][j]-C[i][j];
   
   fprintf(fp,"%d,%lf,%lf\n",SIZE,_el_time,el_time); 

   printf("[std]Error: %lf\n",(double) (err/SIZE));    
  }
   fclose(fp);
    
/*
    for(int i = 0; i < SIZE; i++){
	    for(int j = 0; j < SIZE; j++)
		    printf("%d ", _C[i][j]);
	    printf("\n");
    }

    printf("\n");
		  
    for(int i = 0; i < SIZE; i++){
	    for(int j = 0; j < SIZE; j++)
		    printf("%d ", C[i][j]);
	    printf("\n");
    }
*/    

    for(i = 0; i < SIZE; i++)
        free((void *)A[i]);
    free((void *)A);

    for(i = 0; i < SIZE; i++)
        free((void *)B[i]);
    free((void *)B);

    for(i = 0; i < SIZE; i++)
        free((void *)C[i]);
    free((void *)C);

    for(i = 0; i < SIZE; i++)
        free((void *)_C[i]);
    free((void *)_C);

    free(threads);

    return 0;
}
