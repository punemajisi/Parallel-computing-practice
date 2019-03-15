/* MPI Parallel Mandelbrot program */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

#define         X_RESN  800       /* x resolution */
#define         Y_RESN  800       /* y resolution */
#define         ITERRATION 1000
#define         OFFSETX 0     /* offset of the oringinal point */
#define         OFFSETY 0
#define         FACTOR 200     /* scale facotr for detail. */
#define         LENGTH 4

typedef struct complextype
        {  
        float real, imag;
        } Compl;


int main(int argc, char *argv[])
{
	XInitThreads();

    int i, j, k;
    int size, rank;

    double startTime, endTime;
    startTime = MPI_Wtime();

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int rowData[X_RESN + 1]; //the array saves the row data buff for communication
    int row= 0;

    MPI_Status status;

    if (rank == 0)
    {

    	Window          win;                            /* initialization for a window */
        unsigned
        int             width, height,                  /* window size */
                        x, y,                           /* window position */
                        border_width,                   /*border width in pixels */
                        display_width, display_height,  /* size of screen */
                        screen;                         /* which screen */

        char            *window_name = "Mandelbrot Set", *display_name = NULL;
        GC              gc;
        unsigned
        long            valuemask = 0;
        XGCValues       values;
        Display         *display;
        XSizeHints      size_hints;
        Pixmap          bitmap;
        XPoint          points[800];
        FILE            *fp, *fopen ();
        char            str[100];

        XSetWindowAttributes attr[1];

       /* Mandlebrot variables */

       
        if (  (display = XOpenDisplay (display_name)) == NULL ) 
        {
           fprintf (stderr, "drawon: cannot connect to X server %s\n",
                                XDisplayName (display_name) );
           MPI_Finalize();
        }
        
        /* get screen size */

        screen = DefaultScreen (display);
        display_width = DisplayWidth (display, screen);
        display_height = DisplayHeight (display, screen);

        /* set window size */

        width = X_RESN;
        height = Y_RESN;

        /* set window position */

        x = 0;
        y = 0;

        /* create opaque window */

        border_width = 4;
        win = XCreateSimpleWindow (display, RootWindow (display, screen),
                                x, y, width, height, border_width, 
                                BlackPixel (display, screen), WhitePixel (display, screen));

        size_hints.flags = USPosition|USSize;
        size_hints.x = x;
        size_hints.y = y;
        size_hints.width = width;
        size_hints.height = height;
        size_hints.min_width = 300;
        size_hints.min_height = 300;
        
        XSetNormalHints (display, win, &size_hints);
        XStoreName(display, win, window_name);

        /* create graphics context */

        gc = XCreateGC (display, win, valuemask, &values);

        XSetBackground (display, gc, WhitePixel (display, screen));
        XSetForeground (display, gc, BlackPixel (display, screen));
        XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

        attr[0].backing_store = Always;
        attr[0].backing_planes = 1;
        attr[0].backing_pixel = BlackPixel(display, screen);

        XChangeWindowAttributes(display, win,
        						 CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

        XMapWindow (display, win);
        XSync(display, 0);

        /* Assign rows to slaves */
        int count=0;

        for(i = 1; i < size; i++)
        {
           MPI_Send(&row, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
           row++;
           count++;
        }

        do
        {
        	/* Receve back data from slave process*/
        	MPI_Recv(&rowData, X_RESN+1, MPI_INT, MPI_ANY_SOURCE, 2, 
                      MPI_COMM_WORLD, &status);      

        	int backRank = status.MPI_SOURCE;
        	count--;

        	// Send next data(in if) or send stop flag(in else), distinguish by TAG.
        	if(row < Y_RESN)
        	{
        		MPI_Send(&row, 1, MPI_INT, backRank, 1, MPI_COMM_WORLD);
        		row++; 
        		count++;
        	}
        	else
        	{
        		MPI_Send(&row, 1, MPI_INT, backRank, 3, MPI_COMM_WORLD);
        	}

        	/* Draw points */	
        	int rowNum = rowData[X_RESN];
        	for(j=0; j< X_RESN; j++)
        	{
        		if(rowData[j]== 100) 
        		{
        		//XLockDisplay(display);
        		XDrawPoint(display,win,gc,rowNum,j);       		
				//XUnlockDisplay(display);
				}
        	}
        } while(count>0);

        XFlush(display);
        sleep (30);
    }
    else
    {

    	float   lengthsq, temp;
        Compl   z,c;

    	/*Receive the next operating row number */

    	MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

       	while(status.MPI_TAG == 1) //If tags is row number
	    {
	    	/* set row number at rowData[X_RESN] */
			rowData[X_RESN] = row;

			/* Calculate points */
			for(i = 0; i< X_RESN; i++)
			{
			 	z.real = z.imag = 0.0;

			    /* scale factors for 800 x 800 window */
			    c.real = ((float)row - 400.0)/FACTOR + OFFSETX;
			    c.imag = ((float)i - 400.0)/FACTOR + OFFSETY;
			    k = 0;
			    
			    do  /* iterate for pixel color */
			    { 
			        temp = z.real*z.real - z.imag*z.imag + c.real;
			        z.imag = 2.0*z.real*z.imag + c.imag;
			        z.real = temp;
			        lengthsq = z.real*z.real+z.imag*z.imag;
			        k++;

	          	} while(lengthsq < 4.0 && k < ITERRATION);

	          	/*store result Y array in rowData */
	          	rowData[i] = k;
       		}

       		/*Send rowData back to the master */
           	MPI_Send(&rowData, X_RESN + 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

           	/*Receive the next command */
           	MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	    }
    }

    endTime = MPI_Wtime();
    double totaltime = endTime - startTime;
    printf("Run time is: %fs\n", totaltime);


    /* Program Finished */

    MPI_Finalize();
	return 0;
}