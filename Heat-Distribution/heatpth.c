#include "const.h"
#include "models.h"
#include "display.h"
#include <pthread.h>
#include <stdio.h>

#define legal(x, n) ( (x)>=0 && (x)<(n) )
#define start_time clock_gettime(CLOCK_MONOTONIC, &start);
#define end_time clock_gettime(CLOCK_MONOTONIC, &finish); 
#define time_elapsed_ns (long long)(finish.tv_sec-start.tv_sec)*1000000000 + finish.tv_nsec - start.tv_nsec
#define time_elapsed_s (double)(finish.tv_sec-start.tv_sec) + (double)(finish.tv_nsec - start.tv_nsec)/1000000000
#define NOT_FIRE_PLACE i

int iteration, threads;

TemperatureField *field;
TemperatureField *tempField, *swapField;

pthread_t *threadPool;
pthread_mutex_t *subThreadWakeUp, *subThreadFinished;
int *threadID, terminate;

double *error;
double  EPSILON;


int dx[4] = {0, -1, 0, 1};
int dy[4] = {1, 0, -1, 0};

int x, y, iter_cnt;

int min(int x, int y){ if (x<y) return x; return y; }

void* iterateLine(void* data)
{   
    int threadID = *((int*)data);
    while (1)
    {
    	/*Lock the thread calculating now, then set the size, start and end */
	    pthread_mutex_lock(&subThreadWakeUp[threadID]);
	    if (terminate) break;
	    int blockSize = field->x/threads + !!(field->x%threads);
	    int lineStart = blockSize * threadID;
	    int lineEnd = min(blockSize*(threadID+1), field->x);
	    error[threadID]=0;

	    int i, j, d;
	    for (i=lineStart; i<lineEnd; ++i) 
		for (j=0; j<field->y; ++j)
		{
			tempField->t[i][j] = 0;
			for (d=0; d<4; ++d)
				if ( legal(i+dx[d], field->x) && legal(j+dy[d], field->y) )
					tempField->t[i][j] += field->t[i+dx[d]][j+dy[d]];
				else
					tempField->t[i][j] += ROOM_TEMP;
			tempField->t[i][j] /= 4;
			if (NOT_FIRE_PLACE)
				error[threadID] += fabs(tempField->t[i][j] - field->t[i][j]);
		}
		/* add the thread finished job to finished*/
	    pthread_mutex_unlock(&subThreadFinished[threadID]);
    }
    pthread_exit(NULL);
}

double temperature_iterate()
{
	++iter_cnt; //Just a counter hold the iteration number.
	refreshField(field, 0, 0, field->x, field->y, field->x, field->y);
	int i;

	/* unlock threads in WakeUp and lock threads in Finish*/
	for (i=0; i<threads; ++i)
		pthread_mutex_unlock(&subThreadWakeUp[i]);
	for (i=0; i<threads; ++i)
		pthread_mutex_lock(&subThreadFinished[i]);

	double sumError = 0;
	for (i=0; i<threads; ++i)
		sumError += error[i];

	return sumError;
}

int main(int argc, char **argv)
{
    struct timespec start, finish;
    start_time

    /*Reading parameter*/
    if (argc<5)
    {
	    printf("Usage: %s x y iteration INCREMENT_TIME, INCREMENT threads EPSILON\n", argv[0]);
    }
    sscanf(argv[1], "%d", &x);
    sscanf(argv[2], "%d", &y);
    sscanf(argv[3], "%d", &iteration);
    sscanf(argv[4], "%d", &threads);
    sscanf(argv[5], "%lf", &EPSILON);

    field = malloc(sizeof(TemperatureField));
    tempField = malloc(sizeof(TemperatureField));
    threadPool = malloc(sizeof(pthread_t)*threads);
    subThreadWakeUp = malloc(sizeof(pthread_mutex_t)*threads);
    subThreadFinished = malloc(sizeof(pthread_mutex_t)*threads);
    threadID = malloc(sizeof(int)*threads);
    error = malloc(sizeof(double)*threads);
    terminate = 0;
    field->x = y;
    field->y = x;



    /*Initial mutex and lock up*/
    int i;
    for (i=0; i<threads; ++i)
    {
	pthread_mutex_init(&subThreadWakeUp[i], NULL);
	pthread_mutex_init(&subThreadFinished[i], NULL);
	pthread_mutex_lock(&subThreadWakeUp[i]);
	pthread_mutex_lock(&subThreadFinished[i]);
	threadID[i] = i;
	pthread_create(&threadPool[i], NULL, iterateLine, &threadID[i]);
    }

    int iter;
    newField(field, x, x, 0, 0);
    newField(tempField, x, x, 0, 0);
    initField(field);
    XWindow_Init(field);

    /* Main iteration, lines is computed parallel*/
	for (iter=0; iter<iteration; iter++)
    {	
	   double error = temperature_iterate();
	   if (error<EPSILON)
	   {
		printf("Finished. iteration=%d, error=%lf\n", iter, error);
		break;
	   }
	   swapField = field;
	   field = tempField;
	   tempField = swapField;	
	   if(iter % 100 == 0) XRedraw(field);
	}
    
	/*Delete field and unlock mutex*/
    deleteField(field);
    deleteField(tempField);
    free(threadPool);
    for (i=0; i<threads; ++i)
    {
	    terminate = 1;
	    pthread_mutex_unlock(&subThreadWakeUp[i]);
    }
    
    /*Print result and exit multi-threads*/
    printf("Finished in %d iterations.\n", iter_cnt);
    end_time;
    printf("%lf\n", time_elapsed_s);

    sleep(30);
    pthread_exit(NULL);
    return 0;
}
