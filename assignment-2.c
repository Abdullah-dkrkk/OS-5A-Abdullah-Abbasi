#include <stdio.h>

#define MAX 100

typedef struct {
    int pid;
    int at;
    int bt;
    int ct;
    int tat;
    int wt;
    int completed;
} Process;

void sortByArrival(Process p[], int n) {
    // Bubble sort to arrange processes by arrival time
    Process temp;
    for(int i = 0; i < n-1; i++) {
        for(int j = 0; j < n-i-1; j++) {
            if(p[j].at > p[j+1].at) {
                temp = p[j];
                p[j] = p[j+1];
                p[j+1] = temp;
            }
        }
    }
}

void executeFCFS(Process p[], int n, int gantt[], int gstart[], int gend[], int *gindex) {
    int current_time = 0;
    *gindex = 0;

    for(int i = 0; i < n; i++) {
        // Handle idle time between processes
        if(current_time < p[i].at) {
            current_time = p[i].at;
        }
        
        // Record process execution details
        gstart[*gindex] = current_time;
        p[i].ct = current_time + p[i].bt;
        gend[*gindex] = p[i].ct;
        gantt[*gindex] = p[i].pid;
        (*gindex)++;
        
        // Update metrics
        current_time = p[i].ct;
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
    }
}

void executeSJF(Process p[], int n, int gantt[], int gstart[], int gend[], int *gindex) {
    int current_time = 0, completed = 0;
    *gindex = 0;

    // Reset completion status
    for(int i = 0; i < n; i++) {
        p[i].completed = 0;
    }

    while(completed < n) {
        int idx = -1;
        int min_bt = 9999;
        
        // Find shortest job among arrived processes
        for(int i = 0; i < n; i++) {
            if(p[i].at <= current_time && !p[i].completed) {
                if(p[i].bt < min_bt) {
                    min_bt = p[i].bt;
                    idx = i;
                }
            }
        }
        
        if(idx != -1) {
            // Execute the process
            gstart[*gindex] = current_time;
            current_time += p[idx].bt;
            gend[*gindex] = current_time;
            gantt[*gindex] = p[idx].pid;
            (*gindex)++;
            
            // Update process metrics
            p[idx].ct = current_time;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            p[idx].completed = 1;
            completed++;
        } else {
            // No available processes, advance time
            current_time++;
        }
    }
}

void executeSRJF(Process p[], int n, int gantt[], int gstart[], int gend[], int *gindex) {
    int current_time = 0, completed = 0;
    int remaining_bt[MAX], last_pid = -1;
    *gindex = 0;

    for(int i = 0; i < n; i++) {
        remaining_bt[i] = p[i].bt;
        p[i].completed = 0;
    }

    while(completed < n) {
        int idx = -1, min_bt = 9999;

        for(int i = 0; i < n; i++) {
            if(p[i].at <= current_time && !p[i].completed && remaining_bt[i] < min_bt && remaining_bt[i] > 0) {
                min_bt = remaining_bt[i];
                idx = i;
            }
        }

        if(idx != -1) {
            if(last_pid != p[idx].pid) {
                gstart[*gindex] = current_time;
                gantt[*gindex] = p[idx].pid;
                (*gindex)++;
            }

            remaining_bt[idx]--;
            current_time++;
            last_pid = p[idx].pid;

            if(remaining_bt[idx] == 0) {
                p[idx].ct = current_time;
                p[idx].tat = p[idx].ct - p[idx].at;
                p[idx].wt = p[idx].tat - p[idx].bt;
                p[idx].completed = 1;
                completed++;

                gend[*gindex - 1] = current_time;
                last_pid = -1;
            }
        } else {
            current_time++;
            last_pid = -1;
        }
    }
}

void executeRR(Process p[], int n, int quantum, int gantt[], int gstart[], int gend[], int *gindex) {
    int remaining_bt[MAX];
    for (int i = 0; i < n; i++) remaining_bt[i] = p[i].bt;

    int current_time = 0, completed = 0, idx = 0;
    int queue[MAX], front = 0, rear = 0, inQueue[MAX] = {0};

    *gindex = 0;

    queue[rear++] = 0;
    inQueue[0] = 1;

    while (completed < n) {
        int pid = queue[front++];
        Process *proc = &p[pid];

        if (current_time < proc->at) current_time = proc->at;

        gstart[*gindex] = current_time;

        if (remaining_bt[pid] > quantum) {
            current_time += quantum;
            remaining_bt[pid] -= quantum;
            queue[rear++] = pid; // push back
        } else {
            current_time += remaining_bt[pid];
            remaining_bt[pid] = 0;
            proc->ct = current_time;
            proc->tat = proc->ct - proc->at;
            proc->wt = proc->tat - proc->bt;
            proc->completed = 1;
            completed++;
        }

        gend[*gindex] = current_time;
        gantt[*gindex] = proc->pid;
        (*gindex)++;

        for (int i = 0; i < n; i++) {
            if (p[i].at <= current_time && remaining_bt[i] > 0 && !inQueue[i]) {
                queue[rear++] = i;
                inQueue[i] = 1;
            }
        }

        if (front == rear) {
            for (int i = 0; i < n; i++) {
                if (remaining_bt[i] > 0) {
                    queue[rear++] = i;
                    inQueue[i] = 1;
                    break;
                }
            }
        }
    }
}

void printResults(Process p[], int n, int gantt[], int gstart[], int gend[], int gindex) {
    // Fancy Gantt Chart with vertical bars
    printf("\nGantt Chart:\n");

    // Top border
    for (int i = 0; i < gindex; i++) printf("-------");
    printf("-\n");

    // Process labels
    for (int i = 0; i < gindex; i++) printf("|  P%d  ", gantt[i]);
    printf("|\n");

    // Bottom border
    for (int i = 0; i < gindex; i++) printf("-------");
    printf("-\n");

    // Time scale
    printf("%d", gstart[0]);
    for (int i = 0; i < gindex; i++) {
        printf("     %d", gend[i]);
    }
    printf("\n");

    // Process table
    printf("\nProcess\tAT\tBT\tCT\tTAT\tWT\n");
    float total_tat = 0, total_wt = 0;
    for (int i = 0; i < n; i++) {
        total_tat += p[i].tat;
        total_wt += p[i].wt;
        printf("P%d\t%d\t%d\t%d\t%d\t%d\n",
               p[i].pid, p[i].at, p[i].bt, p[i].ct, p[i].tat, p[i].wt);
    }

    printf("\nAverage Turnaround Time: %.2f\n", total_tat / n);
    printf("Average Waiting Time: %.2f\n", total_wt / n);
}

int main() {
    int n, choice, quantum;
    Process p[MAX];
    int gantt[MAX], gstart[MAX], gend[MAX], gindex;

    printf("\nChoose scheduling algorithm:\n");
    printf("1. SJF (Non-preemptive)\n");
    printf("2. FCFS\n");
    printf("3. SRJF (Preemptive)\n");
    printf("4. Round Robin\n");
    printf("Enter your choice (1 to 4): ");
    scanf("%d", &choice);

    if(choice == 4) {
        printf("Enter Time Quantum: ");
        scanf("%d", &quantum);
    }

    printf("\nEnter number of processes: ");
    scanf("%d", &n);

    for(int i = 0; i < n; i++) {
        p[i].pid = i;
        printf("Enter Arrival Time and Burst Time for Process %d: ", i + 1);
        scanf("%d %d", &p[i].at, &p[i].bt);
        p[i].completed = 0;
    }

    sortByArrival(p, n);

    switch(choice) {
        case 1: executeSJF(p, n, gantt, gstart, gend, &gindex); break;
        case 2: executeFCFS(p, n, gantt, gstart, gend, &gindex); break;
        case 3: executeSRJF(p, n, gantt, gstart, gend, &gindex); break;
        case 4: executeRR(p, n, quantum, gantt, gstart, gend, &gindex); break;
        default:
            printf("Invalid choice!\n");
            return 1;
    }

    printResults(p, n, gantt, gstart, gend, gindex);
    return 0;
}