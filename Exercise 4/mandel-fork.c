/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

/*TODO header file for m(un)map*/

#include "mandel-lib.h"
#include <sys/mman.h>
#include <signal.h>
#include <semaphore.h>


#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/

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
int n;
void compute_and_output_mandel_line(int line,sem_t *sem)
{
        /*
         * A temporary array, used to hold color values for the line being drawn
         */ 
        int i; 
        for (i = line;i < y_chars;i += n)
        {
                int color_val[x_chars];

                //with bigger critical section
                //sem_wait(&sem[i % nthreads]);

                compute_mandel_line(i, color_val);

                // the semaphore should wait the sem
                // that is between 0 -> nthreads
                // (the sem that is responsible for
                //  computing that line)
                sem_wait(&sem[i % n]);

                output_mandel_line(1, color_val);

                // we use sem_post to signal the
                // next semaphore 
                sem_post(&sem[((i % n) + 1) % n]);
        }
        raise(SIGSTOP);
}

/*
 * Create a shared memory area, usable by all descendants of the calling
 * process.
 */
void *create_shared_memory_area(unsigned int numbytes)
{
        int pages;
        void *addr;

        if (numbytes == 0) {
                fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
                exit(1);
        }

        /*
         * Determine the number of pages needed, round up the requested number of
         * pages
         */
        pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

        /* Create a shared, anonymous mapping for this number of pages */
        /* TODO:  
                addr = mmap(...)
        */
        addr = mmap(NULL,pages,PROT_EXEC| PROT_READ|PROT_WRITE
                                        ,MAP_SHARED|MAP_ANONYMOUS , 0,0);
        return addr;
}
void destroy_shared_memory_area(void *addr, unsigned int numbytes) {
        int pages;

        if (numbytes == 0) {
                fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
                exit(1);
        }

        /*
         * Determine the number of pages needed, round up the requested number of
         * pages
         */
        pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

        if (munmap(addr, pages * sysconf(_SC_PAGE_SIZE)) == -1) {
                perror("destroy_shared_memory_area: munmap failed");
                exit(1);
        }
}

int main(int argc,char **argv)
{
        if( argc == 1) {
                printf("Too few arguments.Please tell me how many threads to create\n");
                return 0;
        }
        n =atoi( argv[1]);

        //signal(SIGINT, catchctrlc);

        // now we check if the number
        // of threads is valid
        if (n < 1 || n > (y_chars - 1))
        {
                printf("The number of threads is invalid\n");
                return -1;
        }

        xstep = (xmax - xmin) / x_chars;
        ystep = (ymax - ymin) / y_chars;

        /*
         * draw the Mandelbrot Set, one line at a time.
         * Output is sent to file descriptor '1', i.e., standard output.
         */
        /*for (line = 0; line < y_chars; line++) {
                compute_and_output_mandel_line(1, line);
        }*/
        sem_t *sem = create_shared_memory_area(sizeof(int) * n);
        sem_init(&sem[0],0,1);

        sem_init(&sem[0],1,1);
        if(n > 1) {
                int i;
                for (i = 1;i < n;i++) {
                sem_init(&sem[i],1,0);
                }
        }
        pid_t procid[n];
        int status[n];
        int i;
        for(i = 0;i < n; i++)
        {
                procid[i] = fork();
                if (procid[i] < 0) {
                        perror("main: fork");
                        exit(1);
                }
                if (procid[i] == 0) {
                        /* Child */
                        compute_and_output_mandel_line(i,sem);
                        exit(i);
                }
        }

        //raise(SIGSTOP);
        printf("goes here\n");
        for (i = 0;i < n; i++)
        {
                kill(procid[i],SIGCONT);
             //   procid[i] = wait(&status[i]);
             //   explain_wait_status(procid[i],status[i]);
        }
        destroy_shared_memory_area(sem,sizeof(int) * n);
        reset_xterm_color(1);
        return 0;
}
               
