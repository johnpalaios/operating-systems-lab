#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

int fork_procs(struct tree_node* node,int *res);
void leaf(struct tree_node* node);
void notleaf(struct tree_node* node,int *j);
pid_t makechild (struct tree_node* node,int *k);

int r(char *c,int a,int b)
{
        if(*c == '+') return a + b;
        if(*c == '*') return a * b;
        exit(-1);
}

pid_t makechild(struct tree_node* node,int *k)
{
        int pfd[2];
        if(pipe(pfd) < 0) {
        perror("pipe");
        exit(1);
        }
        pid_t pid;
        /* Fork root of process tree */
        pid = fork();
        if (pid < 0) {
                perror("main: fork");
                exit(1);
        }
        if (pid == 0) {
                /* Child */
                int value;
                if(node->nr_children > 0)
                { // an einai parent
                 fork_procs(node,&value);
                 if( write(pfd[1],&value
                        ,sizeof(value))
                         != sizeof(value))
                        { perror("write to pipe error");}
                  printf("%s : Wrote to its parent : %d\n",
                         node->name,value);
                }
                if(node->nr_children == 0)
                {// an einai leaf
                 value = atoi(node->name);
                 if( write(pfd[1],&value
                        ,sizeof(value))
                         != sizeof(value))
                        { perror("write to pipe error");}
                  printf("%s : Wrote to its parent : %d\n",
                         node->name,value);
                 leaf(node);
                 exit(12);
                }
        }
        else {
          int value;
        if(read(pfd[0],&value
                        ,sizeof(value))
                        != sizeof(value))
                        { perror("read from pipe error");}
                printf("Parent with the pid : %d read : %d\n"
                ,getpid(),value);
         *k = value;
         }
        return pid;

}

int fork_procs(struct tree_node* node,int*res)
{
        int n = node->nr_children;
        change_pname(node->name);
        //printf("%s : Awake\n",node->name);
        if(n == 0) leaf(node);
        change_pname(node->name);
        printf("%s : Awake\n",node->name);
        int status[n];
        pid_t c[n];
        int c1,c2;
        c[1] = makechild(node->children+1,&c2);
        c[0] = makechild(node->children+0,&c1);
        *res = r(node->name,c2,c1);
        printf("Result from %d was this %d\n"
                ,getpid(),*res);
        c[0] = wait(&status[0]);
        explain_wait_status(c[0],status[0]);
        c[1] = wait(&status[1]);
        explain_wait_status(c[1],status[1]);
        //*res = r(node->name,c2,c1);
        return *res;
}

void leaf(struct tree_node* node)
{
        change_pname(node->name);
        printf("%s : Awake\n",node->name);
        sleep(SLEEP_PROC_SEC);
        printf("%s Exit\n",node->name);
        exit(13);
}

int main(int argc,char* argv[])
{
        struct tree_node *root;
        root =  get_tree_from_file(argv[1]);
        //print_tree(root); // for debug
        //printf("\n");
        int pfdd[2];
        if(pipe(pfdd) < 0) {
        perror("pipe");
        exit(1);
        }
        pid_t pid;
        int status;
        /* Fork root of process tree */
        pid = fork();
        if (pid < 0) {
                perror("main: fork");
                exit(1);
        }
        if (pid == 0) {
                /* Child */
                int res;
                fork_procs(root,&res);
                printf("This is the result : %d\n"
                        ,res);
                if(write(pfdd[1],&res
                        ,sizeof(res))
                         != sizeof(res))
                        { perror("write to pipe error");}
                close(pfdd[1]);
                //printf("%s Exit\n",root->name);
                exit(1);
        }

        /* for ask2-{fork, tree} */
         sleep(SLEEP_TREE_SEC);

        /* Print the process tree root at pid */
         show_pstree(pid);

        int res;
        if(read(pfdd[0],&res
                        ,sizeof(res))
                        != sizeof(res))
                        { perror("read from pipe error");}

        close(pfdd[0]);

        /* Wait for the root of the process tree to terminate */
        pid = wait(&status);
        explain_wait_status(pid, status);
        return 0;
}


