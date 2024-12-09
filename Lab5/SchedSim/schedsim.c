// C program for implementation of Simulation 
#include<stdio.h> 
#include<limits.h>
#include<stdlib.h>
#include "process.h"
#include "util.h"

void printMetrics(ProcessType plist[], int n);

// Function to find the waiting time for all  
// processes
void findWaitingTimeRR(ProcessType plist[], int n, int quantum) {
    int *rem_bt = (int *)malloc(n * sizeof(int)); // remaining burst times
    int t = 0; // current time
    int done = 0; // count of completed processes

    for (int i = 0; i < n; i++)
        rem_bt[i] = plist[i].bt; // initialize remaining burst times

    while (done < n) {
        int progress = 0;
        for (int i = 0; i < n; i++) {
            if (rem_bt[i] > 0) {
                progress = 1;
                if (rem_bt[i] > quantum) {
                    t += quantum;
                    rem_bt[i] -= quantum;
                } else {
                    t += rem_bt[i];
                    plist[i].wt = t - plist[i].bt - plist[i].art;
                    rem_bt[i] = 0;
                    done++;
                }
            }
        }
        if (!progress) { // no progress made, increase time
            t++;
        }
    }
    free(rem_bt);
}

// Function to find the waiting time for all  
// processes 
void findWaitingTimeSJF(ProcessType plist[], int n) {
    int completed = 0, current_time = 0;
    int shortest = -1, finish_time;
    int *remaining_time = (int *)malloc(n * sizeof(int));
    int is_completed = 0;

    // Initialize remaining burst times
    for (int i = 0; i < n; i++) {
        remaining_time[i] = plist[i].bt;
    }

    // SJF Algorithm
    while (completed != n) {
        int min_remaining_time = INT_MAX;
        is_completed = 0;
        
        // Find the process with the shortest remaining time that has arrived
        for (int i = 0; i < n; i++) {
            if (plist[i].art <= current_time && remaining_time[i] > 0 && remaining_time[i] < min_remaining_time) {
                shortest = i;
                min_remaining_time = remaining_time[i];
                is_completed = 1;
            }
        }

        if (!is_completed) {
            // If no process is ready to run, increment the current time
            current_time++;
            continue;
        }

        // Reduce the remaining burst time of the selected process
        remaining_time[shortest]--;

        // If the process is completed
        if (remaining_time[shortest] == 0) {
            completed++;
            finish_time = current_time + 1;
            plist[shortest].wt = finish_time - plist[shortest].bt - plist[shortest].art;

            // If waiting time is negative, set it to 0
            if (plist[shortest].wt < 0) {
                plist[shortest].wt = 0;
            }
        }

        // Increment the current time
        current_time++;
    }

    free(remaining_time);
}


// Function to find the waiting time for all  
// processes 
void findWaitingTime(ProcessType plist[], int n)
{ 
    // waiting time for first process is 0, or the arrival time if not 
    plist[0].wt = 0 +  plist[0].art; 
  
    // calculating waiting time 
    for (int  i = 1; i < n ; i++ ) 
        plist[i].wt =  plist[i-1].bt + plist[i-1].wt; 
} 
  
// Function to calculate turn around time 
void findTurnAroundTime( ProcessType plist[], int n)
{ 
    // calculating turnaround time by adding bt[i] + wt[i] 
    for (int  i = 0; i < n ; i++) 
        plist[i].tat = plist[i].bt + plist[i].wt; 
} 
  
// Function to sort the Process acc. to priority
int my_comparer(const void *this, const void *that)
{ 
  const ProcessType *p1 = (const ProcessType *)this;
  const ProcessType *p2 = (const ProcessType *)that;

    /*  
     * 1. Cast this and that into (ProcessType *)
     * 2. return 1 if this->pri < that->pri
     */ 
  
    return p1->pri - p2->pri;
} 

//Function to calculate average time 
void findavgTimeFCFS( ProcessType plist[], int n) 
{ 
    //Function to find waiting time of all processes 
    findWaitingTime(plist, n); 
  
    //Function to find turn around time for all processes 
    findTurnAroundTime(plist, n); 
  
    //Display processes along with all details 
    printf("\n*********\nFCFS\n");
}

//Function to calculate average time 
void findavgTimeSJF( ProcessType plist[], int n) 
{ 
    //Function to find waiting time of all processes 
    findWaitingTimeSJF(plist, n); 
  
    //Function to find turn around time for all processes 
    findTurnAroundTime(plist, n); 
  
    //Display processes along with all details 
    printf("\n*********\nSJF\n");
}

//Function to calculate average time 
void findavgTimeRR( ProcessType plist[], int n, int quantum) 
{ 
    //Function to find waiting time of all processes 
    findWaitingTimeRR(plist, n, quantum); 
  
    //Function to find turn around time for all processes 
    findTurnAroundTime(plist, n); 
  
    //Display processes along with all details 
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

//Function to calculate average time 
void findavgTimePriority( ProcessType plist[], int n) 
{ 
  qsort(plist,n,sizeof(ProcessType),my_comparer);
  findWaitingTime(plist,n);
  findTurnAroundTime(plist,n);
   /*
    * 1- Sort the processes (i.e. plist[]), burst time and priority according to the priority.
    * 2- Now simply apply FCFS algorithm.
    */
  
    //Display processes along with all details 
    printf("\n*********\nPriority\n");
}

void printMetrics(ProcessType plist[], int n)
{
    int total_wt = 0, total_tat = 0; 
    float awt, att;
    
    printf("\tProcesses\tPriority\tBurst time\tWaiting time\tTurn around time\n"); 
  
    // Calculate total waiting time and total turn  
    // around time 
    for (int  i=0; i<n; i++) 
    { 
        total_wt = total_wt + plist[i].wt; 
        total_tat = total_tat + plist[i].tat; 
        // printf("\t%d\t\t%d\t\t%d\t\t%d\n", plist[i].pid, plist[i].bt, plist[i].wt, plist[i].tat); 
        printf("\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", plist[i].pid,plist[i].pri, plist[i].bt, plist[i].wt, plist[i].tat); 
    } 
  
    awt = ((float)total_wt / (float)n);
    att = ((float)total_tat / (float)n);
    
    printf("\nAverage waiting time = %.2f", awt); 
    printf("\nAverage turn around time = %.2f\n", att); 
} 

ProcessType * initProc(char *filename, int *n) 
{
  	FILE *input_file = fopen(filename, "r");
	  if (!input_file) {
		    fprintf(stderr, "Error: Invalid filepath\n");
		    fflush(stdout);
		    exit(0);
	  }

    ProcessType *plist = parse_file(input_file, n);
  
    fclose(input_file);
  
    return plist;
}
  
// Driver code 
int main(int argc, char *argv[]) 
{ 
    int n; 
    int quantum = 2;

    ProcessType *proc_list;
  
    if (argc < 2) {
		   fprintf(stderr, "Usage: ./schedsim <input-file-path>\n");
		   fflush(stdout);
		   return 1;
	   }
    
  // FCFS
    n = 0;
    proc_list = initProc(argv[1], &n);
  
    findavgTimeFCFS(proc_list, n);
    
    printMetrics(proc_list, n);
    free(proc_list);
  
  // SJF
    n = 0;
    proc_list = initProc(argv[1], &n);
   
    findavgTimeSJF(proc_list, n); 
   
    printMetrics(proc_list, n);
    free(proc_list);
  
  // Priority
    n = 0; 
    proc_list = initProc(argv[1], &n);
    
    findavgTimePriority(proc_list, n); 
    
    printMetrics(proc_list, n);
    free(proc_list);
    
  // RR
    n = 0;
    proc_list = initProc(argv[1], &n);
    
    findavgTimeRR(proc_list, n, quantum); 
    
    printMetrics(proc_list, n);
    free(proc_list);
    
    return 0; 
} 