#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <X11/Xlib.h>    /* for X-window */
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define NUM_BODY 50 /* the number of bodies */
#define MAX_X 900   /* the positions (x_i,y_i) of bodies are randomly picked */
#define MIN_X 100   /* where x_i is in MIN_X to MAX_X and  y_i is in MIN_Y to MAX_Y */
#define MAX_Y 500  
#define MIN_Y 100
#define MAX_W 800   /* the weights w_i of bodies are randomly picked */
#define MIN_W 200   /* where w_i is in MIN_W to MAX_X */
#define MAX_V 20    /* the velocities (vx_i,vy_i) of bodies are randomly picked */
#define MIN_V 0        /* where vx_i,vy_i are in MIN_V to MAX_V */

#define ITERATION 100    /* run in ITERATION times */ 
#define DELTA_T 1  

#define X_RESN  1000    /* x resolution */
#define Y_RESN  600    /* y resolution */

/* Gravitational constant */
const double G_CONS=6.67259;

/* body structure */
struct Body{
    /* position */
    double x,y;
    /* velocity */
    double vx,vy;
    /* weight */
    double w;
};

int main(int argc, char* argv[]){
    /* start to record time */
    /* open file to record time */
    FILE * pFile;
    pFile=fopen("time_hw5_seq.txt","a");
    /* create display */
    Window win;                    /* initialization for a window */
    unsigned int width, height,            /* window size */
         x, y,                /* window position */
         border_width,            /* border width in pixels */
         display_width, display_height, /* size of screen */
         screen;            /* which screen */

    char *window_name="N-Body", *display_name=NULL;
    GC gc;
    unsigned long valuemask = 0;
    XGCValues values;
    Display *display;
    XSizeHints size_hints;
    Pixmap bitmap;
    XPoint points[800];
    FILE *fp, *fopen();
    char str[100];
    XSetWindowAttributes attr[1];
       
    /* create a display and connect to Xserver */
    if ((display=XOpenDisplay(display_name))==NULL){
        fprintf(stderr, "drawon: cannot connect to X server %s\n",
                XDisplayName (display_name) );
        fclose(pFile);
        exit(-1);
    }
    
    /* get display infomation */
    screen = DefaultScreen (display);
    display_width = DisplayWidth (display, screen);
    display_height = DisplayHeight (display, screen);

    /* set the attributes of window*/
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
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);
    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel(display, screen);
    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);
    XMapWindow (display, win);

    /* set color */
    Colormap screen_colormap = DefaultColormap(display, DefaultScreen(display));
    XColor grayColor;

    /* N-Body variables */
    int i,j,k;
    struct Body Nbody[NUM_BODY];    /* record the properties of all bodies */
    unsigned int bodyColor[NUM_BODY];    /* to record the color of bodies */
    int bodyR[NUM_BODY];        /* to record the radius of bodies */
    double newVx[NUM_BODY];        /* for calculate each body's velocity */
    double newVy[NUM_BODY];

    /* initialize N-body */
    srand(time(NULL));
    for(i=0;i<NUM_BODY;i++){
        Nbody[i].x=rand()%(MAX_X-MIN_X)+MIN_X;
        Nbody[i].y=rand()%(MAX_Y-MIN_Y)+MIN_Y;
        Nbody[i].vx=newVx[i]=rand()%(MAX_V-MIN_V)-(MAX_V+MIN_V)/2;
        Nbody[i].vy=newVy[i]=rand()%(MAX_V-MIN_V)-(MAX_V+MIN_V)/2;
        Nbody[i].w=rand()%(MAX_W-MIN_W)+MIN_W;
        bodyColor[i]=65000-Nbody[i].w*60;   /* heavier, darker */
        bodyR[i]=Nbody[i].w*0.05;        /* heavier, larger */
    }

    /* start to simulate N-body */
    for(k=0;k<ITERATION;k++){
    /* Calculate the position of bodies by point-to-point in each iteration */
        for(i=0;i<NUM_BODY;i++){
            for(j=0;j<NUM_BODY;j++) {
                if(j==i){ /* there is no need to calculate the effect from itself */
                    continue;
                }
                double delta_x=Nbody[j].x-Nbody[i].x;
                double delta_y=Nbody[j].y-Nbody[i].y;
                double distance=sqrt((delta_x * delta_x)+ (delta_y * delta_y));
                if(distance==0){ /* if two bodies have the same position, skip the force calculation */
                    continue;
                }
                double force=G_CONS*Nbody[j].w/(distance*distance);
                newVx[i]=newVx[i]+DELTA_T*force*delta_x/distance;
                newVy[i]=newVy[i]+DELTA_T*force*delta_y/distance;
            }
        }
        /* update the new data */
        for(i=0;i<NUM_BODY;i++){
            Nbody[i].x=Nbody[i].x+newVx[i]*DELTA_T;
            Nbody[i].y=Nbody[i].y+newVy[i]*DELTA_T;
            Nbody[i].vx=newVx[i];
            Nbody[i].vy=newVy[i];
        }
        /* display new position */ 
        for(i=0;i<NUM_BODY;i++){
            grayColor.red=grayColor.green=grayColor.blue=bodyColor[i];
            XAllocColor(display,screen_colormap,&grayColor);
            XSetForeground(display,gc,grayColor.pixel);
            XFillArc(display,win,gc,(int)(Nbody[i].x-bodyR[i]),(int)(Nbody[i].y-bodyR[i]),
                    (int)(2*bodyR[i]),(int)(2*bodyR[i]),0,360*64);
            grayColor.red=grayColor.green=grayColor.blue=0;
            XAllocColor(display,screen_colormap,&grayColor);
            XSetForeground(display,gc,grayColor.pixel);
            XDrawArc(display,win,gc,(int)(Nbody[i].x-bodyR[i]),(int)(Nbody[i].y-bodyR[i]),
                    (int)(2*bodyR[i]),(int)(2*bodyR[i]),0,360*64);
        }
        XFlush(display);
    }
    return 0;
}
