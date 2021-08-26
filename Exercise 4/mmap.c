/*
 * mmap.c
 *
 * Examining the virtual memory of processes.
 *
 * Operating Systems course, CSLab, ECE, NTUA
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <sys/wait.h>

#include "help.h"
#include <inttypes.h>

#define RED     "\033[31m"
#define RESET   "\033[0m"


char *heap_private_buf;
char *heap_shared_buf;

char *file_shared_buf;

uint64_t buffer_size;


/*
 * Child process' entry point.
 */
void child(void)
{
        uint64_t pa;

        /*
         * Step 7 - Child
         */
        if (0 != raise(SIGSTOP))
                die("raise(SIGSTOP)");
        /*
         * TODO: Write your code here to complete child's part of Step 7.
         */

        printf("This is the child's map:\n");
        show_maps();
        /*
         * Step 8 - Child
         */
        if (0 != raise(SIGSTOP))
                die("raise(SIGSTOP)");
        /*
         * TODO: Write your code here to complete child's part of Step 8.
         */
        uint64_t va1 = (uint64_t)heap_private_buf;
        pa = get_physical_address(va1);
        printf("[CHILD] This is the physical address : 0x%" PRIx64 "\n",pa);

        /*
         * Step 9 - Child
         */
        if (0 != raise(SIGSTOP))
                die("raise(SIGSTOP)");
        /*
         * TODO: Write your code here to complete child's part of Step 9.
         */
        heap_private_buf = "THE CHILD WAS HERE";
        uint64_t va2 = (uint64_t)heap_private_buf;
        pa = get_physical_address(va2);
        printf("[CHILD] This is the physical address : 0x%" PRIx64 "\n",pa);

        /*
         * Step 10 - Child
         */
        if (0 != raise(SIGSTOP))
                die("raise(SIGSTOP)");
        /*
         * TODO: Write your code here to complete child's part of Step 10.
         */
        heap_shared_buf = "This is STEP 10 and child was here";
        uint64_t va3 = (uint64_t)heap_shared_buf;
        pa = get_physical_address(va3);
        printf("[CHILD] This is the physical address : 0x%" PRIx64 "\n",pa);

        /*
         * Step 11 - Child
         */
        if (0 != raise(SIGSTOP))
                die("raise(SIGSTOP)");
        /*
         * TODO: Write your code here to complete child's part of Step 11.
         */
        if (mprotect( heap_shared_buf,buffer_size, PROT_READ )) {
        perror("Couldn’t mprotect");
        exit(errno);
        }
        printf("[CHILD]\n");
        show_maps();
        /*
         * Step 12 - Child
         */
        /*
         * TODO: Write your code here to complete child's part of Step 12.
         */
       munmap(heap_private_buf, buffer_size);
       munmap(heap_shared_buf, buffer_size);
       munmap(file_shared_buf, buffer_size);
}

/*
 * Parent process' entry point.
 */
void parent(pid_t child_pid)
{
        uint64_t pa;
        int status;

        /* Wait for the child to raise its first SIGSTOP. */
        if (-1 == waitpid(child_pid, &status, WUNTRACED))
                die("waitpid");

        /*
         * Step 7: Print parent's and child's maps. What do you see?
         * Step 7 - Parent
         */
        printf(RED "\nStep 7: Print parent's and child's map.\n" RESET);
        press_enter();

        printf("This is the parent's map:\n");
        show_maps();

        /*
         * TODO: Write your code here to complete parent's part of Step 7.
         */

        if (-1 == kill(child_pid, SIGCONT))
                die("kill");
        if (-1 == waitpid(child_pid, &status, WUNTRACED))
                die("waitpid");


        /*
         * Step 8: Get the physical memory address for heap_private_buf.
         * Step 8 - Parent
         */
        printf(RED "\nStep 8: Find the physical address of the private heap "
                "buffer (main) for both the parent and the child.\n" RESET);
        press_enter();

        uint64_t va1 = (uint64_t)heap_private_buf;
        pa = get_physical_address(va1);
        printf("[PARENT] This is the physical address : 0x%" PRIx64 "\n",pa);
        /*
         * TODO: Write your code here to complete parent's part of Step 8.
         */

        if (-1 == kill(child_pid, SIGCONT))
                die("kill");
        if (-1 == waitpid(child_pid, &status, WUNTRACED))
                die("waitpid");


        /*
         * Step 9: Write to heap_private_buf. What happened?
         * Step 9 - Parent
         */
        printf(RED "\nStep 9: Write to the private buffer from the child and "
                "repeat step 8. What happened?\n" RESET);
        press_enter();

        /*
         * TODO: Write your code here to complete parent's part of Step 9.
         */

        uint64_t va2 = (uint64_t)heap_private_buf;
        pa = get_physical_address(va2);
        printf("[PARENT] This is the physical address : 0x%" PRIx64 "\n",pa);

        if (-1 == kill(child_pid, SIGCONT))
                die("kill");
        if (-1 == waitpid(child_pid, &status, WUNTRACED))
                die("waitpid");


        /*
         * Step 10: Get the physical memory address for heap_shared_buf.
         * Step 10 - Parent
         */
        printf(RED "\nStep 10: Write to the shared heap buffer (main) from "
                "child and get the physical address for both the parent and "
                "the child. What happened?\n" RESET);
        press_enter();

        /*
         * TODO: Write your code here to complete parent's part of Step 10.
         */
        uint64_t va3 = (uint64_t)heap_shared_buf;
        pa = get_physical_address(va3);
        printf("[PARENT] This is the physical address of heap_shared_buf : 0x%" PRIx64 "\n",pa);

        if (-1 == kill(child_pid, SIGCONT))
                die("kill");
        if (-1 == waitpid(child_pid, &status, WUNTRACED))
                die("waitpid");


        /*
         * Step 11: Disable writing on the shared buffer for the child
         * (hint: mprotect(2)).
         * Step 11 - Parent
         */
        printf(RED "\nStep 11: Disable writing on the shared buffer for the "
                "child. Verify through the maps for the parent and the "
                "child.\n" RESET);
        press_enter();

        /*
         * TODO: Write your code here to complete parent's part of Step 11.
         */
        printf("[PARENT]\n");
        show_maps();

        if (-1 == kill(child_pid, SIGCONT))
                die("kill");
        if (-1 == waitpid(child_pid, &status, 0))
                die("waitpid");


        /*
         * Step 12: Free all buffers for parent and child.
         * Step 12 - Parent
         */

        /*
         * TODO: Write your code here to complete parent's part of Step 12.
         */
        munmap(heap_private_buf, buffer_size);
        munmap(heap_shared_buf, buffer_size);
        munmap(file_shared_buf, buffer_size);
}
 
int main(void)
{
        pid_t mypid, p;
        int fd = -1;
        uint64_t pa;

        mypid = getpid();
        buffer_size = 1 * get_page_size();

        /*
         * Step 1: Print the virtual address space layout of this process.
         */
        printf(RED "\nStep 1: Print the virtual address space map of this "
                "process [%d].\n" RESET, mypid);
        press_enter();
        /*
         * TODO: Write your code here to complete Step 1.
         */
         show_maps();

        /*
         * Step 2: Use mmap to allocate a buffer of 1 page and print the map
         * again. Store buffer in heap_private_buf.
         */
        printf(RED "\nStep 2: Use mmap(2) to allocate a private buffer of "
                "size equal to 1 page and print the VM map again.\n" RESET);
        press_enter();
        /*
         * TODO: Write your code here to complete Step 2.
         */


         heap_private_buf = mmap(NULL,buffer_size,PROT_EXEC| PROT_READ|PROT_WRITE
                                        ,MAP_SHARED|MAP_ANONYMOUS , fd,0);
         show_maps();
        /*
         * Step 3: Find the physical address of the first page of your buffer
         e in main memory. What do you see?
         */
        printf(RED "\nStep 3: Find and print the physical address of the "
                "buffer in main memory. What do you see?\n" RESET);
        press_enter();
        /*
         * TODO: Write your code here to complete Step 3.
         */

        uint64_t va = (uint64_t)heap_private_buf;
        pa = get_physical_address(va);
        printf("This is the physical address : 0x%" PRIx64 "\n",pa);

        /*
         * Step 4: Write zeros to the buffer and repeat Step 3.
         */
        printf(RED "\nStep 4: Initialize your buffer with zeros and repeat "
                "Step 3. What happened?\n" RESET);
        press_enter();
        /*
         * TODO: Write your code here to complete Step 4.
         */

        memset(heap_private_buf,0,buffer_size);
        va = (uint64_t)heap_private_buf;
        pa = get_physical_address(va);
        printf("This is the physical address : 0x%" PRIx64 "\n",pa);

        /*
         * Step 5: Use mmap(2) to map file.txt (memory-mapped files) and print
         * its content. Use file_shared_buf.
         */
        printf(RED "\nStep 5: Use mmap(2) to read and print file.txt. Print "
                "the new mapping information that has been created.\n" RESET);
        press_enter();
        /*
         * TODO: Write your code here to complete Step 5.
         */
        fd = open("file.txt",O_RDONLY);
        struct stat st;
        fstat(fd, &st);
        int size = st.st_size;
        unsigned char *file_shared_buf;
        file_shared_buf = (unsigned char*) mmap(0,size, PROT_READ,MAP_FILE|MAP_PRIVATE, fd,0);
        int i;
        for (i = 0; i < size; i++) {
                 char c;
                 c = file_shared_buf[i];
                 putchar(c);
        }
        show_maps();
        /*
         * Step 6: Use mmap(2) to allocate a shared buffer of 1 page. Use
         * heap_shared_buf.
         */
        printf(RED "\nStep 6: Use mmap(2) to allocate a shared buffer of size "
                "equal to 1 page. Initialize the buffer and print the new "
                "mapping information that has been created.\n" RESET);
        press_enter();
        /*
         * TODO: Write your code here to complete Step 6.
         */

        heap_shared_buf = mmap(NULL,buffer_size,PROT_EXEC| PROT_READ|PROT_WRITE
                              ,MAP_SHARED|MAP_ANONYMOUS , fd,0);
        p = fork();
        if (p < 0)
                die("fork");
        if (p == 0) {
                child();
                return 0;
        }
        show_maps();
        parent(p);

        if (-1 == close(fd))
                perror("close");
        return 0;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
