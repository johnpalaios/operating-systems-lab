#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "proc-common.h"
#include "tree.h"

void fork_procs(struct tree_node* node);
void leaf(struct tree_node* node);
pid_t makechild (struct tree_node* node);

pid_t makechild(struct tree_node* node)
{
        pid_t pid;
        /* Fork root of process tree */
        pid = fork();
        if (pid < 0) {
                perror("main: fork");
                exit(1);
        }
        if (pid == 0) {
                /* Child */
               fork_procs(node);
               //wait_for_ready_children(node->nr_children);
        }
        return pid;
}
void fork_procs(struct tree_node* node)
{
        change_pname(node->name);
        printf("%s : Awake\n",node->name);
        int n = node->nr_children;
        if(n == 0)
        {
                raise(SIGSTOP);
                printf("%s : Exit\n",node->name);
                exit(420);
        }
        int i = 0;
        int status[n];
        pid_t c[n];
        for(i = 0;i<n;++i)
        {
                 c[i] = makechild(node->children+i);
        }
        //wait_for_ready_children(node->nr_children);
        raise(SIGSTOP);
        for(i = 0;i<n;++i)
        {
                kill(c[i],SIGCONT);
                c[i] = wait(&status[i]);
                explain_wait_status(c[i],status[i]);
        }
        printf("%s : Exit\n",node->name);
        exit(422);
}

int main(int argc,char* argv[])
{
        struct tree_node *root;
        root =  get_tree_from_file(argv[1]);
        //print_tree(root); // for debug
        //printf("\n");

        pid_t pid;
        int status;

        /* Fork root of process tree */
        pid = fork();
        if (pid < 0) {
                perror("main: fork");
                exit(2);
        }
        if (pid == 0) {
                /* Child */
                fork_procs(root);
        }

        /* Print the process tree root at pid */
        show_pstree(pid);

        /* for ask2-signals */
        kill(pid, SIGCONT);

        /* Wait for the root of the process tree to terminate */
        pid = wait(&status);
        explain_wait_status(pid, status);


        return 0;
}

