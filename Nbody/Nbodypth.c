#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <math.h>
#include <pthread.h>
#include <time.h>


const int X_RESN = 800;
const int Y_RESN = 800;

const double G = 6.6;
const int NUMB = 500;
const int MAXX = 500;
const int MINX  = 300;
const int MAXY = 500;
const int MINY  = 300;
const int MAXW = 100;
const int MINW  = 50;
const int ITERATION = 100;
const double T = 0.1;

struct Body{
    /* position */
    double x,y;
    /* velocity */
    double vx,vy;
    /* weight */
    double w;
}Nbody[505];


int i,j,k;
double vx[505];
double vy[505];


Window win;
GC gc;
Display *display;

pthread_mutex_t mutex;
int startx;

void forcecal(void* para)
{	
    startx = (int)para;

    double deltaX, deltaY;
    double distance;
    double F;
	// I don't know why but here should minors some value to avoid segmentation fault.
    while(startx < NUMB - 8){
	/* Update speed for each point.*/
		for(j=0;j<NUMB;j++)
		{
		if (j==startx) continue;
		deltaX = Nbody[j].x - Nbody[startx].x;
		deltaY = Nbody[j].y - Nbody[startx].y;
		distance = sqrt((deltaX * deltaX) + (deltaY * deltaY));
        //printf("%d\n",Nbody[startx].x);
		if(distance == 0) continue;
		F = G * Nbody[j].w / (distance*distance);
		if(distance > 5)
			{
			vx[startx] = vx[startx] + T * F * deltaX/distance;
			vy[startx] = vy[startx] + T * F * deltaY/distance;
			}
		}		
		/* retrieve and update the next job */
    pthread_mutex_lock(&mutex);
    startx++;
    pthread_mutex_unlock(&mutex);
    }
}

int main (void)
{
	    XInitThreads();

        Window          win;                            /* initialization for a window */
        unsigned
        int             width, height,                  /* window size */
                        x, y,                           /* window position */
                        border_width,                   /*border width in pixels */
                        display_width, display_height,  /* size of screen */
                        screen;                         /* which screen */

        char            *window_name = "Mandelbrot Set", *display_name = NULL;
        //GC              gc;
        unsigned
        long            valuemask = 0;
        XGCValues       values;
        //Display         *display;
        XSizeHints      size_hints;
        
        XSetWindowAttributes attr[1];
       
        /* connect to Xserver */

        if (  (display = XOpenDisplay (display_name)) == NULL ) {
           fprintf (stderr, "drawon: cannot connect to X server %s\n",
                                XDisplayName (display_name) );
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

        XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

        XMapWindow (display, win);
        XSync(display, 0);

        int scr = DefaultScreen(display);
		int pm = XCreatePixmap(display,win,X_RESN,Y_RESN,DefaultDepth(display,scr));

        /*Initialization with random weight and position with 0 inital speed*/
        srand(time(NULL));
        for(i=0;i<NUMB;i++)
        {
            Nbody[i].x = rand() % (MAXX-MINX)+MINX;
            Nbody[i].y = rand() % (MAXY-MINY)+MINY;
            Nbody[i].vx = 0;
            Nbody[i].vy = 0;
            Nbody[i].w = rand() % (MAXW-MINW)+MINW;
            //printf("%f\n", Nbody[i].w);
        }
        printf("The pthread begins\n");

        int nthread, n;
        nthread = 4;
        n = 0;

        pthread_t tid[nthread];
        pthread_mutex_init(&mutex, NULL);

        for(k=0;k<ITERATION;k++)
        {
            printf("The iter %d begins\n", k);
            XSetForeground(display,gc,0);
            XFillRectangle(display,pm,gc,0,0,X_RESN,Y_RESN);
          
            for (i = 0; i < nthread; i++)
            {
                int startx = i;
                pthread_create(&tid[i], NULL, &forcecal, (void*)(startx));
            }

            for (i = 0; i < nthread; i++) 
            {
                pthread_join(tid[i], NULL);
            }
            /* update position for each point.*/

            for(i=0;i<NUMB;i++)
            {
                //printf("%f\n", vx[i]);
                Nbody[i].x = Nbody[i].x + vx[i] * T;
                Nbody[i].y = Nbody[i].y + vy[i] * T;
                Nbody[i].vx = vx[i];
                Nbody[i].vy = vy[i];
            } 
            /*Draw the points. */ 

            XSetForeground(display, gc, WhitePixel(display,scr));

            for(i=0;i<NUMB - 8;i++)
            {
                if(Nbody[i].y <= 800 && Nbody[i].x <= 800)         
                XDrawPoint(display, pm, gc, (int)Nbody[i].y, (int)Nbody[i].x);            
            }
            XCopyArea(display,pm,win,gc,0,0,X_RESN,Y_RESN,0,0);         
            XFlush(display);
            //sleep(0.1);   
        }
        XFreePixmap(display,pm);
		XCloseDisplay(display);
		//sleep(10);
	return 0;
}
