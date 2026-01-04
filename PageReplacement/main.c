#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>


typedef struct {   //simulate FIFO replacement
    int *frames;              
    int frame_count;          
    int filled_frames_count;                 
    int stack_pointer;        
} FIFOQueue;


typedef struct {    //Used in LRU algorithm
    int page_num;      
    int last_used;   
} PageFrame;


//Helper function for logging the memory status each time a page is referenced
void printFramesInt(int frames[], int frame_count) {
    printf("[");
    for (int i = 0; i < frame_count; i++) {
        if (frames[i] == -1)
            printf(" - ");
        else
            printf(" %d ", frames[i]);
        if (i < frame_count - 1) printf("|");
    }
    printf("]");
}


void fifoPageReplacement(int pages[], int n, int frame_count) {
    int *frames = (int *)malloc(sizeof(int) * frame_count);
    for (int i = 0; i < frame_count; i++)
        frames[i] = -1; // Initialize all frames as empty

    FIFOQueue q;
    q.frames = frames;
    q.frame_count = frame_count;
    q.filled_frames_count = 0;
    q.stack_pointer = 0;

    int pageFaults = 0;

    for (int i = 0; i < n; i++) {
        int page_num = pages[i];
        bool page_in_mem = false;

        // Check if page is already in memory
        for (int j = 0; j < q.frame_count; j++) {
            if (q.frames[j] == page_num) {
                page_in_mem = true;
                break;    //we break here bacause we have found the page in the memory and no need to check other frames
            }
        }

        printf("Access page %d: ", page_num);   //this is for Logging purpose.

        if (page_in_mem) {
            printf("this page is in memory\t");
            printFramesInt(q.frames, q.frame_count);
            printf("\n");
            continue;   // no need to execute the rest of the code in the main loop because the page is found.
        }

        // Page fault occurs
        pageFaults++;

        
        q.frames[q.stack_pointer] = page_num; // Replace page at the stack_pointer position
        q.stack_pointer = (q.stack_pointer + 1) % q.frame_count;  //and then update the pointer of the stack.

        if (q.filled_frames_count < q.frame_count)
            q.filled_frames_count++;

        printf("PAGE FAULT\t");
        printFramesInt(q.frames, q.frame_count);
        printf("\n");
    }

    printf("Total Page Faults = %d\n", pageFaults);
    free(frames);
}



// MAIN FUNCTION
int main() {
    int pages[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    int n = sizeof(pages) / sizeof(pages[0]);    // this is the number of pages that our process has.
    int frame_count = 4;  

    printf("FIFO Algorithm:\n");
    fifoPageReplacement(pages, n, frame_count);

    // printf("\nLRU Algorithm:\n");
    // lruPageReplacement(pages, n, frame_count);

    // printf("\nOptimal Algorithm:\n");
    // optimalPageReplacement(pages, n, frame_count);

    return 0;
}