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
void Frames_Logger(int frames[], int frame_count) {
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
            Frames_Logger(q.frames, q.frame_count);
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
        Frames_Logger(q.frames, q.frame_count);
        printf("\n");
    }

    printf("Total Page Faults = %d\n", pageFaults);
    free(frames);
}


void lruPageReplacement(int pages[], int n, int frame_count) {
    PageFrame *frames = (PageFrame *)malloc(sizeof(PageFrame) * frame_count);
    // this is initialization:
    for (int i = 0; i < frame_count; i++) {
        frames[i].page_num = -1;     // Empty frame
        frames[i].last_used = 0;
    }

    int timeCounter = 0;
    int pageFaults = 0;

    for (int i = 0; i < n; i++) {
        int page_num = pages[i];
        timeCounter++;
        bool page_in_mem = false;

        // Check if page exists and update access time
        for (int j = 0; j < frame_count; j++) {
            if (frames[j].page_num == page_num) {
                frames[j].last_used = timeCounter;
                page_in_mem = true;
                break;
            }
        }

        printf("Access page %d: ", page_num);

        if (page_in_mem) {
            printf("page_in_mem\t");
            int frames_currentstatus[frame_count];
            for (int k = 0; k < frame_count; k++)
                frames_currentstatus[k] = frames[k].page_num;
            Frames_Logger(frames_currentstatus, frame_count);
            printf("\n");
            continue;
        }

        // Page fault occurs
        pageFaults++;

        // Look for an empty frame
        int emptyIndex = -1;
        for (int j = 0; j < frame_count; j++) {
            if (frames[j].page_num == -1) {
                emptyIndex = j;
                break;
            }
        }

        if (emptyIndex != -1) {
            frames[emptyIndex].page_num = page_num;
            frames[emptyIndex].last_used = timeCounter;
        } else {
            // Find least recently used page
            int lruIndex = 0;
            int minTime = frames[0].last_used;

            for (int j = 1; j < frame_count; j++) {
                if (frames[j].last_used < minTime) {
                    minTime = frames[j].last_used;
                    lruIndex = j;
                }
            }

            frames[lruIndex].page_num = page_num;
            frames[lruIndex].last_used = timeCounter;
        }

        int frames_currentstatus[frame_count];
        for (int k = 0; k < frame_count; k++)
            frames_currentstatus[k] = frames[k].page_num;

        printf("PAGE FAULT\t");
        Frames_Logger(frames_currentstatus, frame_count);
        printf("\n");
    }

    printf("Total Page Faults = %d\n", pageFaults);
    free(frames);
}


void optimalPageReplacement(int pages[], int n, int frame_count) {
    int *frames = (int *)malloc(sizeof(int) * frame_count);
    for (int i = 0; i < frame_count; i++)
        frames[i] = -1;

    int pageFaults = 0;

    for (int i = 0; i < n; i++) {
        int page = pages[i];
        bool page_in_mem = false;

        // Check if page is already in memory
        for (int j = 0; j < frame_count; j++) {
            if (frames[j] == page) {
                page_in_mem = true;
                break;
            }
        }

        printf("Access page %d: ", page);

        if (page_in_mem) {
            printf("page_in_mem\t");
            Frames_Logger(frames, frame_count);
            printf("\n");
            continue;
        }

        // Page fault occurs
        pageFaults++;

        // Check for empty frame
        int emptyIndex = -1;
        for (int j = 0; j < frame_count; j++) {
            if (frames[j] == -1) {
                emptyIndex = j;
                break;
            }
        }

        if (emptyIndex != -1)
        {
            frames[emptyIndex] = page;
        }
        else
        {
            int replaceIndex = -1;  //this is the index of the frame which its page will be replaced with the page now we have on the reference string.
            int farthest = -1;    //from all the pages we have in the frames this will be updated to the page that will be referenced in the farthest future.

            // Find the page with farthest next use
            for (int j = 0; j < frame_count; j++) {  //at the end of this loop we will have the final replaceindex and farthest variables value.
                int nextUse;
                for (nextUse = i + 1; nextUse < n; nextUse++) {
                    if (frames[j] == pages[nextUse])
                        break;    //if we find a page which will be referenced in the future ,we save its index which is nextuse variable(index shows the time which it will be reference in the future)
                }

                if (nextUse >= n) {     // this is the case that we have found the best candidate frame the page we have to be replaced here.(no need to check other pages in the frames so we break)
                    replaceIndex = j;
                    break;  
                }

                if (nextUse > farthest) {   //this is the time we update the two variables (farthest and replaceindex so we can update them on the next iterations of the loop)
                    farthest = nextUse;
                    replaceIndex = j;
                }
            }

            if (replaceIndex == -1)
                replaceIndex = 0;

            frames[replaceIndex] = page;
        }

        printf("PAGE FAULT\t");
        Frames_Logger(frames, frame_count);
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

    printf("\nLRU Algorithm:\n");
    lruPageReplacement(pages, n, frame_count);

    printf("\nOptimal Algorithm:\n");
    optimalPageReplacement(pages, n, frame_count);

    return 0;
}