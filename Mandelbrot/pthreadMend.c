/* Pthread Parallel Mandelbrot program */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <math.h>
#include <pthread.h>
#include <time.h>


#define         X_RESN  800       /* x resolution */
#define         Y_RESN  800       /* y resolution */
#define         ITERRATION  2000
#define         OFFSETX 0    /* offset of the oringinal point */
#define         OFFSETY 0
#define         FACTOR 200     /* scale facotr for detail. */
#define         LENGTH 4

typedef struct complextype
        {
        float real, imag;
        } Compl;

struct timespec {
    time_t tv_sec;        
    long int tv_nsec;    
};

Window win;
GC gc;
Display *display;

pthread_mutex_t mutex;
int startx;

struct timespec begin, end;
double timediff;


void *mandelbrot(void* para)
{
    int startx = (int)para;
        
	/* Mandlebrot variables */
    int  j, k;
    Compl   z, c;
    float   lengthsq, temp;
    while(startx < X_RESN)
    {
        /* Calculate and draw points */
        for(j=0; j < Y_RESN; j++) 
        {
    		z.real = z.imag = 0.0; 
    		c.real = ((float) j - 400.0)/FACTOR + OFFSETX;           /* scale factors for 800 x 800 window */
    		c.imag = ((float) startx - 400.0)/FACTOR + OFFSETY;
    		k = 0;

    		do
            {                                             /* iterate for pixel color */
        		temp = z.real*z.real - z.imag*z.imag + c.real;
        		z.imag = 2.0*z.real*z.imag + c.imag;
        		z.real = temp;
        		lengthsq = z.real*z.real+z.imag*z.imag;
        		k++;
        	} while(lengthsq < LENGTH && k < ITERRATION);

    		if (k == ITERRATION)
            {
                XDrawPoint(display, win, gc, j, startx);
                XFlush (display);
            }
        }
        //retrieve and update the next job
        pthread_mutex_lock(&mutex);
        startx ++;
        pthread_mutex_unlock(&mutex);
    }
}

int main (void)
{
        XInitThreads();

        //Window          win;                            /* initialization for a window */
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

       /* Mandlebrot variables */
        int i, j, k;
        Compl   z, c;
        float   lengthsq, temp;
       
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

        int nthread, n;
        nthread = 8;

        clock_gettime(CLOCK_MONOTONIC, &begin);

        /*Parallel Mandlebrot variables */
        pthread_t tid[nthread];
        pthread_mutex_init(&mutex, NULL);

        //Jobs * Jobdis = malloc(sizeof(Jobs));
        int startx = i;
        while(n < nthread)
        {       
        	pthread_create(&tid[n], NULL, &mandelbrot, (void *)(startx)); 
            //Int startx change to void pointer to startx. Here will result in warning.    	
            n++;
        }

        for (n = 0; n < nthread; n++) {
        pthread_join(tid[i], NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        double totaltime = end.tv_sec - begin.tv_sec + (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
        printf("Run time is: %fs\n", totaltime);

        XFlush (display);
        sleep (10);

        //pthread_exit(NULL);
        /* Program Finished */
	
	return (1);
}
