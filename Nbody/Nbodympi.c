#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "mpi.h"


const int X_RESN = 800;
const int Y_RESN = 800;

const double G = 6.6;
const int NUMB = 100;
const int MAXX = 500;
const int MINX  = 300;
const int MAXY = 500;
const int MINY  = 300;
const int MAXW = 100;
const int MINW  = 50;
const int ITERATION = 1000;
const double T = 0.1;

struct Body{
    /* position */
    double x,y;
    /* velocity */
    double vx,vy;
    /* weight */
    double w;
};



int main (int argc, char *argv[]) 
{
    XInitThreads();

    Window win;
	GC gc;
Display *display;

	int i, j, k;
    int size, rank;

    double startTime, endTime;
    startTime = MPI_Wtime();

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Datatype MPIBody;
    MPI_Type_contiguous(5, MPI_DOUBLE, &MPIBody);
    MPI_Type_commit(&MPIBody);

    MPI_Status status;
    int job = NUMB /size;

    double vx[NUMB];
    double vy[NUMB];
    double deltaX, deltaY;
    double distance;
    double F;

    struct Body *local_nBody;
    local_nBody = (struct Body*)malloc(NUMB * sizeof(struct Body));
    struct Body* Nbody = (struct Body*)malloc(NUMB * sizeof(struct Body));

    if (rank == 0)
    {

    	Window          win;                            /* initialization for a window */
        unsigned
        int             width, height,                  /* window size */
                        x, y,                           /* window position */
                        border_width,                   /*border width in pixels */
                        display_width, display_height,  /* size of screen */
                        screen;                         /* which screen */

        char            *window_name = "N-body Simulation", *display_name = NULL;
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

        int scr = DefaultScreen(display);
        int pm = XCreatePixmap(display,win,X_RESN,Y_RESN,DefaultDepth(display,scr));

        /*Initialization with random weight and position with 0 inital speed*/
        srand(time(NULL));
        for(i=0;i<NUMB;i++)
        {
        	Nbody[i].x = rand() % (MAXX-MINX) + MINX;
        	Nbody[i].y = rand() % (MAXY-MINY) + MINY;
        	Nbody[i].vx = 0;
        	Nbody[i].vy = 0;
        	Nbody[i].w = rand()% (MAXW-MINW) + MINW;
        }

        for (i = 1; i < size; i++)
            MPI_Send(Nbody, NUMB, MPIBody, i, i, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Recv(Nbody, NUMB, MPIBody, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for(k=0;k<ITERATION;k++)
    {
        int startPoint = job * rank;
        
        /* Update speed for each point in job.*/
        for(i=startPoint;i<job + startPoint;i++)
        {
            for(j=0;j<NUMB;j++)
            {
                if (j==i) continue;
                deltaX = Nbody[j].x - Nbody[i].x;
                deltaY = Nbody[j].y - Nbody[i].y;
                distance = sqrt((deltaX * deltaX) + (deltaY * deltaY));
                if(distance == 0) continue;
                F = G * Nbody[j].w / (distance*distance);
                if(distance > 5)
                {
                vx[i] = vx[i] + T * F * deltaX/distance;
                vy[i] = vy[i] + T * F * deltaY/distance;
            	}
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);

        /* update position for each point in job.*/
        for(i=startPoint;i<job + startPoint;i++)
        {
            Nbody[i].x = Nbody[i].x + vx[i] * T;
            Nbody[i].y = Nbody[i].y + vy[i] * T;
            Nbody[i].vx = vx[i];
            Nbody[i].vy = vy[i];
        }

        for(i=0;i<job;i++)
        {
            local_nBody[i].x = Nbody[startPoint+i].x;
            local_nBody[i].y = Nbody[startPoint+i].y;
            local_nBody[i].vy = Nbody[startPoint+i].vy;
            local_nBody[i].vx = Nbody[startPoint+i].vx;
        }

        MPI_Gather(local_nBody, job, MPIBody, Nbody, job, MPIBody, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        /*Draw the points. */
        if(rank == 0)
        {
   //      	int scr = DefaultScreen(display);
   //      	int pm = XCreatePixmap(display,win,X_RESN,Y_RESN,DefaultDepth(display,scr));
   //      	XClearWindow(display, win);
   //      	XSetForeground(display, gc, WhitePixel(display,scr));
   //      	for(i=0;i<NUMB;i++)
   //      	{      		
			// 	XDrawPoint(display, pm, gc, Nbody[i].y, Nbody[i].x);			
   //      	}
			// XCopyArea(display,pm,win,gc,0,0,X_RESN,Y_RESN,0,0);		

            for (j = 1; j < size; j++)
            MPI_Send(Nbody, NUMB, MPIBody, j, j, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(Nbody, NUMB, MPIBody, 0, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
       
    if(rank == 0)
    {
        endTime = MPI_Wtime();
        double totaltime = endTime - startTime;
        printf("Run time is: %fs\n", totaltime);
    }

    MPI_Finalize();

	return 0;
}
