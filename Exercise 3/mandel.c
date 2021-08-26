 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "mandel-lib.h"


#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define MANDEL_MAX_ITERATION 100000
// defined perror_thread
#define perror_pthread(ret, message) \
                do { errno = ret; perror(message); } while (0)

/***************************
 * Compile-time parameters *
 ***************************/

void  catchctrlc(int sig)
{
     // signal is ignored
     signal(sig, SIG_IGN);
     // reset the color of the characters
     reset_xterm_color(1);
     // exit
     exit(-1);
}
/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 50;
int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;

/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

// these are the threads we must create
// we set it as global
int nthreads;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
        /*
         * x and y traverse the complex plane.
         */
        double x, y;

        int n;
        int val;

        /* Find out the y value corresponding to this line */
        y = ymax - ystep * line;

        /* and iterate for all points on this line */ 
        for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

                /* Compute the point's color value */
                val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
                if (val > 255)
                        val = 255;

                /* And store it in the color_val[] array */
                val = xterm_color(val);
                color_val[n] = val;
        }
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
        int i;

        char point ='@';
        char newline='\n';

        for (i = 0; i < x_chars; i++) {
                /* Set the current color, then output the point */
                set_xterm_color(fd, color_val[i]);
                if (write(fd, &point, 1) != 1) {
                        perror("compute_and_output_mandel_line: write point");
                        exit(1);
                }
        }

        /* Now that the line is done, output a newline character */
        if (write(fd, &newline, 1) != 1) {
                perror("compute_and_output_mandel_line: write newline");
                exit(1);
        }
}
// we create a global array of semaphores
#define u 100000
sem_t sem[u];

void *compute_and_output_mandel_line(void * lin)
{
        /*
         * A temporary array, used to hold color 
         * values for the line being drawn
         */

        int line = *(int *) lin;
        int i;
        for (i = line;i < y_chars;i += nthreads)
        {
                int color_val[x_chars];

                //with bigger critical section
                //sem_wait(&sem[i % nthreads]);

                compute_mandel_line(i, color_val);

                // the semaphore should wait the sem
                // that is between 0 -> nthreads
                // (the sem that is responsible for
                //  computing that line)
                sem_wait(&sem[i % nthreads]);

                output_mandel_line(1, color_val);

                // we use sem_post to signal the
                // next semaphore
                sem_post(&sem[((i % nthreads) + 1) % nthreads]);
        }
        return 0;
}

int main(int argc,char* argv[])
{
        //take the number of threads !
        /*printf("Give us the number of threads you want us to create : \n");
        if(scanf("%d",&nthreads)==1) 
        {
                printf("%d\n",nthreads);
        }
        else printf("Couldn't read the number of threads\n");
        // took it*/

        //for the 2nd question
        //nthreads=2;

        if( argc == 1) {
                printf("Too few arguments.Please tell me how many threads to create\n");
                return 0;
        }
        nthreads =atoi( argv[1]);

        signal(SIGINT, catchctrlc);

        // now we check if the number
        // of threads is valid
        if (nthreads < 1 || nthreads > (y_chars - 1))
        {
                printf("The number of threads is invalid\n");
                return -1;
        }


        // an array of thread id's
        pthread_t thrid[nthreads];

        // sem_init(sem_t *sem,int pshared,unsigned int val);
        // sem : the semaphore to be initialized
        // pshared : if pshared == 0 it is shared between threads
        // val : the value assigned to the new semaphore
        // sem[0] == 1 && if (i > 1) => sem[i] == 0
        sem_init(&sem[0],0,1);
        if(nthreads > 1) {
                int i;
                for (i = 1;i < nthreads;i++) {
                sem_init(&sem[i],0,0);
                }
        }

        xstep = (xmax - xmin) / x_chars;
        ystep = (ymax - ymin) / y_chars;

        /*
         * draw the Mandelbrot Set, one line at a time.
         * Output is sent to file descriptor '1', i.e., standard output.
         */

        int ret;
        int i;
        for(i = 0;i < nthreads; i++)
        {
                ret = pthread_create(&thrid[i],NULL
                        ,compute_and_output_mandel_line,&i);
                if (ret) {
                perror_pthread(ret, "pthread_create");
                exit(1);
                }
        }

        /*for (line = 0; line < y_chars; line++) {
        
                compute_and_output_mandel_line(1, line);
        }*/
        for (i = 0;i < nthreads; i++)
        {
                ret = pthread_join(thrid[i],NULL);
                if(ret) perror_pthread(ret,"pthread_join");
        }
        for(i = 0;i < nthreads; i++)
        {
                // destroy semaphoressssss
                sem_destroy(&sem[i]);
        }

        reset_xterm_color(1);
        return 0;
}

