#include <stdio.h>
#define MAX 100

typedef struct {
    int pid, at, bt, ct, tat, wt;                                       
    int remaining_bt, completed;
} Process;

void inputProcesses(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].pid = i;
        printf("Enter Arrival Time and Burst Time for P%d (Please include space between AT and BT): ", i + 1);
        scanf("%d %d", &p[i].at, &p[i].bt);
        p[i].remaining_bt = p[i].bt;
        p[i].completed = 0;
    }
}

void calculateTimes(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
    }
}

void printGanttChart(int gantt[], int gstart[], int gend[], int gcount) {
    printf("\nGantt Chart:\n");
    for (int i = 0; i < gcount; i++) printf("-------");
    printf("-\n");
    for (int i = 0; i < gcount; i++) printf("|  P%d  ", gantt[i] + 1);
    printf("|\n");
    for (int i = 0; i < gcount; i++) printf("-------");
    printf("-\n");

    printf("%d", gstart[0]);
    for (int i = 0; i < gcount; i++) printf("     %d", gend[i]);
    printf("\n");
}

void printResults(Process p[], int n) {
    float total_tat = 0, total_wt = 0;
    printf("\nProcess\tAT\tBT\tCT\tTAT\tWT\n");
    for (int i = 0; i < n; i++) {
        total_tat += p[i].tat;
        total_wt += p[i].wt;
        printf("P%d\t%d\t%d\t%d\t%d\t%d\n", p[i].pid + 1, p[i].at, p[i].bt, p[i].ct, p[i].tat, p[i].wt);
    }
    printf("\nAvg Turnaround Time: %.2f\n", total_tat / n);
    printf("Avg Waiting Time: %.2f\n", total_wt / n);
}

// First Come First Serve
void fcfs(Process p[], int n, int gantt[], int gstart[], int gend[], int *gcount) {
    int time = 0;
    *gcount = 0;
    for (int i = 0; i < n; i++) {
        if (time < p[i].at) time = p[i].at;
        gstart[*gcount] = time;
        time += p[i].bt;
        gend[*gcount] = time;
        gantt[*gcount] = p[i].pid;
        p[i].ct = time;
        (*gcount)++;
    }
    calculateTimes(p, n);
}

// Shortest Job First (Non-preemptive)
void sjf(Process p[], int n, int gantt[], int gstart[], int gend[], int *gcount) {
    int time = 0, completed = 0, min_idx;
    *gcount = 0;
    while (completed < n) {
        int min_bt = 9999;
        min_idx = -1;
        for (int i = 0; i < n; i++) {
            if (!p[i].completed && p[i].at <= time && p[i].bt < min_bt) {
                min_bt = p[i].bt;
                min_idx = i;
            }
        }
        if (min_idx == -1) {
            time++;
            continue;
        }
        gstart[*gcount] = time;
        time += p[min_idx].bt;
        gend[*gcount] = time;
        gantt[*gcount] = p[min_idx].pid;
        p[min_idx].ct = time;
        p[min_idx].completed = 1;
        (*gcount)++;
        completed++;
    }
    calculateTimes(p, n);
}

// Shortest Remaining Job First (Preemptive)
void srjf(Process p[], int n, int gantt[], int gstart[], int gend[], int *gcount) {
    int time = 0, completed = 0, last_pid = -1;
    *gcount = 0;
    while (completed < n) {
        int min_bt = 9999, idx = -1;
        for (int i = 0; i < n; i++) {
            if (!p[i].completed && p[i].at <= time && p[i].remaining_bt < min_bt && p[i].remaining_bt > 0) {
                min_bt = p[i].remaining_bt;
                idx = i;
            }
        }

        if (idx != -1) {
            if (last_pid != p[idx].pid) {
                gstart[*gcount] = time;
                gantt[*gcount] = p[idx].pid;
                (*gcount)++;
            }

            p[idx].remaining_bt--;
            time++;
            last_pid = p[idx].pid;

            if (p[idx].remaining_bt == 0) {
                p[idx].ct = time;
                p[idx].completed = 1;
                gend[*gcount - 1] = time;
                completed++;
                last_pid = -1;
            }
        } else {
            time++;
            last_pid = -1;
        }
    }
    calculateTimes(p, n);
}

// Round Robin
void roundRobin(Process p[], int n, int quantum, int gantt[], int gstart[], int gend[], int *gcount) {
    int time = 0, completed = 0, front = 0, rear = 0;
    int queue[MAX], inQueue[MAX] = {0};
    *gcount = 0;

    queue[rear++] = 0;
    inQueue[0] = 1;

    while (completed < n) {
        int i = queue[front++];
        Process *cur = &p[i];

        if (time < cur->at) time = cur->at;

        gstart[*gcount] = time;

        if (cur->remaining_bt > quantum) {
            time += quantum;
            cur->remaining_bt -= quantum;
            queue[rear++] = i;
            
        } else {
            time += cur->remaining_bt;
            cur->remaining_bt = 0;
            cur->ct = time;
            cur->completed = 1;
            completed++;
        }

        gend[*gcount] = time;
        gantt[*gcount] = cur->pid;
        (*gcount)++;

        for (int j = 0; j < n; j++) {
            if (!inQueue[j] && p[j].at <= time && p[j].remaining_bt > 0) {
                queue[rear++] = j;
                inQueue[j] = 1;
            }
        }

        if (front == rear) {
            for (int j = 0; j < n; j++) {
                if (p[j].remaining_bt > 0) {
                    queue[rear++] = j;
                    inQueue[j] = 1;
                    break;
                }
            }
        }
    }
    calculateTimes(p, n);
}

int main() {
    int n, choice, quantum;
    Process p[MAX];
    int gantt[MAX], gstart[MAX], gend[MAX], gcount;

    printf("Choose Scheduling Algorithm:\n");
    printf("1. FCFS (First Come First Serve)\n2. SJF (Shortest Job First)\n3. SRJF (Shortest Remaining Job first)\n4. Round Robin\nChoice: ");
    scanf("%d", &choice);

    printf("Enter number of processes: ");
    scanf("%d", &n);

    inputProcesses(p, n);

    if (choice == 4) {
        printf("Enter Time Quantum: ");
        scanf("%d", &quantum);
    }

    switch (choice) {
        case 1: fcfs(p, n, gantt, gstart, gend, &gcount); break;
        case 2: sjf(p, n, gantt, gstart, gend, &gcount); break;
        case 3: srjf(p, n, gantt, gstart, gend, &gcount); break;
        case 4: roundRobin(p, n, quantum, gantt, gstart, gend, &gcount); break;
        default: printf("Invalid choice!\n"); return 1;
    }

    printGanttChart(gantt, gstart, gend, gcount);
    printResults(p, n);
    return 0;
}
