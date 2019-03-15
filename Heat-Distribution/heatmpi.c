#include "models.h"
#include "display.h"
#include <mpi.h>

#define legal(x, n) ( (x)>=0 && (x)<(n) )
#define start_time clock_gettime(CLOCK_MONOTONIC, &start);
#define end_time clock_gettime(CLOCK_MONOTONIC, &finish); 
#define time_elapsed_ns (long long)(finish.tv_sec-start.tv_sec)*1000000000 + finish.tv_nsec - start.tv_nsec
#define time_elapsed_s (double)(finish.tv_sec-start.tv_sec) + (double)(finish.tv_nsec - start.tv_nsec)/1000000000
#define NOT_FIRE_PLACE i;

int job;

int dx[4] = {0, -1, 0, 1};
int dy[4] = {1, 0, -1, 0};

int iteration,x,y,iter_cnt;
TemperatureField *field;
TemperatureField *tempField, *swapField;

void temperature_iterate(TemperatureField *field, int x)
{
	++iter_cnt; //Just a counter hold the iteration number.
	int i, j, d;
	for (i=0; i<field->x; ++i)
		for (j=0; j<field->y; ++j)
		{
			int cnt = 0;
			tempField->t[i][j] = 0;
			for (d=0; d<4; ++d)
				if ( legal(i+dx[d], field->x) && legal(j+dy[d], field->y) )
				{
					tempField->t[i][j] += field->t[i+dx[d]][j+dy[d]];
					++cnt;
				}
			tempField->t[i][j] /= cnt;
		}
	for (i=0;i<7*x/10;i++)
	{
		if(3*x/10 <i)
			tempField->t[0][i] = 100.0f;
	}
}

int main(int argc, char **argv){

    XInitThreads();

    struct timespec start, finish;
    start_time

	int i;
	int size, rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc<4) {
	    printf("Usage: %s x iteration\n", argv[0]);
    }
    sscanf(argv[1], "%d", &x);
    sscanf(argv[1], "%d", &y);
    sscanf(argv[3], "%d", &iteration);

    field = malloc(sizeof(TemperatureField));
    tempField = malloc(sizeof(TemperatureField));
    newField(field, x, x, 0, 0);
    newField(tempField, x, x, 0, 0);
    initField(field);

	if (rank == 0) {
        XWindow_Init(field);
	}
	
    job = x / size;
	if (x % size != 0) job++;

    int startx = rank * job;

    int iter;
	for (iter = 0; iter < iteration; iter++) {
		temperature_iterate(field, startx);
        MPI_Allgather(&(tempField->t[startx][0]), job*field->y, MPI_FLOAT, &(field->t[0][0]), job*field->y, MPI_FLOAT, MPI_COMM_WORLD);

		if (rank == 0) {
            for(i = x * 0.3; i < x * 0.7; i++)
			    field->t[0][i] = FIRE_TEMP;
            if(iter % 100 == 0) XRedraw(field);
		}
	} 

    if (rank == 0) {
    /*Print result and exit multi-threads*/
    printf("Finished in %d iterations.\n", iter_cnt);
    end_time;
    printf("%lf\n", time_elapsed_s);
	}
    
    MPI_Finalize();
    return 0;
}
