#include "const.h"
#include "models.h"
#include "display.h"

#define legal(x, n) ( (x)>=0 && (x)<(n) )

#define start_time clock_gettime(CLOCK_MONOTONIC, &start);
#define end_time clock_gettime(CLOCK_MONOTONIC, &finish); 
#define time_elapsed_ns (long long)(finish.tv_sec-start.tv_sec)*1000000000 + finish.tv_nsec - start.tv_nsec
#define time_elapsed_s (double)(finish.tv_sec-start.tv_sec) + (double)(finish.tv_nsec - start.tv_nsec)/1000000000
#define NOT_FIRE_PLACE i

int iteration,iter_cnt,x,y;
TemperatureField *field;
TemperatureField *tempField, *swapField;

int dx[4] = {0, -1, 0, 1};
int dy[4] = {1, 0, -1, 0};

void temperature_iterate(TemperatureField *field, int x)
{
	++iter_cnt;
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

int main(int argc, char **argv)
{

    if (argc<4)
    {
	    printf("Usage: %s x y iteration\n", argv[0]);
    }
    sscanf(argv[1], "%d", &x);
    sscanf(argv[2], "%d", &y);
    sscanf(argv[3], "%d", &iteration);

    field = malloc(sizeof(TemperatureField));
    tempField = malloc(sizeof(TemperatureField));
    newField(field, x, y,0,0);
    newField(tempField, x, y,0,0);
    initField(field);
    XWindow_Init(field);

    struct timespec start, finish;
    start_time

    int iter;
    for (iter=0; iter<iteration; iter++)
    {
	temperature_iterate(field, x);
	swapField = field;
	field = tempField;
	tempField = swapField;
	XRedraw(field);
    }

    /*Print result and exit multi-threads*/
    printf("Finished in %d iterations.\n", iter_cnt);
    end_time;
    printf("%lf\n", time_elapsed_s);
    return 0;
}
