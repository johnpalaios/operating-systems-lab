#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

void fork_procs(struct tree_node* node);
void leaf(struct tree_node* node);
void notleaf(struct tree_node* node);
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
                if(node->nr_children > 0)
                {
                 notleaf(node);
                }
                if(node->nr_children == 0)
                {
                 leaf(node);
                 exit(12);
                }
        }
       return pid;
}

void fork_procs(struct tree_node* node)
{
        int n = node->nr_children;
        change_pname(node->name);
        printf("%s : Awake\n",node->name);
        if(n == 0) leaf(node);
        //printf("%.*s: Sleeping...\n",(int)sizeof(node->name),node->name);
        int i = 0;
        int status[n];
        pid_t c[n];
        for(i = 0;i<n;++i)
        {
         c[i] = makechild(node->children+i);
        }
        for(i = 0;i<n;++i)
        {
          c[i] = wait(&status[i]);
          explain_wait_status(c[i],status[i]);
        }
}

void leaf(struct tree_node* node)
{
        change_pname(node->name);
        printf("%s : Awake\n" , node->name);
        sleep(SLEEP_PROC_SEC);
        printf("%s : Exit\n",node->name);
        exit(13);
}

void notleaf(struct tree_node* node)
{
        fork_procs(node);
        printf("%s: Exit\n",node->name);
        exit(14);
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
                exit(1);
        }
        if (pid == 0) {
                /* Child */
                fork_procs(root);
                exit(1);
        }

        /* for ask2-{fork, tree} */
        sleep(SLEEP_TREE_SEC);

        /* Print the process tree root at pid */
        show_pstree(pid);

        /* for ask2-signals */
        /* kill(pid, SIGCONT); */

        /* Wait for the root of the process tree to terminate */
        pid = wait(&status);
        explain_wait_status(pid, status);


        return 0;
}

