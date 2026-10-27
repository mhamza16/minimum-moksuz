#include <stdio.h>
#include <stdlib.h>

/* One process as typed by the user. Job i+1 in the output is tasks[i]. */
struct task {
    int cpu;    /* CPU time each job of this process needs */
    int period; /* a new job is released every `period` ms; deadline = release + period */
};

/* One released job (one instance of a process). The list of current jobs is a
 * singly linked list kept sorted in EDF order, so the head is always the job to run. */
struct job {
    int pid;          /* process number, 1..n */
    int remaining;    /* CPU ms still needed (the "p=" in the output) */
    long release;     /* "r=" */
    long deadline;    /* "d=" = release + period */
    struct job *next;
};

int job_before(const struct job *a, const struct job *b)
{
    if (a->deadline != b->deadline)
        return a->deadline < b->deadline;
    if (a->release != b->release)
        return a->release < b->release;
    return a->pid < b->pid;
}

struct job *job_new(int pid, int cpu, long release, long deadline)
{
    struct job *j = malloc(sizeof *j);
    j->pid = pid;
    j->remaining = cpu;
    j->release = release;
    j->deadline = deadline;
    j->next = NULL;
    return j;
}

void list_insert(struct job **head, struct job *j)
{
    struct job **link = head;
    while (*link != NULL && !job_before(j, *link))
        link = &(*link)->next;
    j->next = *link;
    *link = j;
}

void list_remove(struct job **head, struct job *j)
{
    struct job **link = head;
    while (*link != j)
        link = &(*link)->next;
    *link = j->next;
    free(j);
}

void list_print(long t, const struct job *head)
{
    printf("%ld: processes:", t);
    for (const struct job *j = head; j != NULL; j = j->next)
        printf(" [%d|p=%d|r=%ld|d=%ld]", j->pid, j->remaining, j->release, j->deadline);
    printf("\n");
}

void list_free(struct job *head)
{
    while (head != NULL) {
        struct job *next = head->next;
        free(head);
        head = next;
    }
}

long gcd(long a, long b)
{
    while (b != 0) {
        long t = a % b;
        a = b;
        b = t;
    }
    return a;
}

long lcm(long a, long b)
{
    return a / gcd(a, b) * b;
}

int main(void)
{
    int n = 0;
    struct task *tasks = NULL;
    long max_time = 1;

    printf("Enter the number of processes to schedule: ");
    scanf("%d", &n);

    tasks = malloc(n * sizeof *tasks);

    for (int i = 0; i < n; i++) {
        printf("Enter the CPU time of process %d: ", i + 1);
        scanf("%d", &tasks[i].cpu);
        printf("Enter the period of process %d: ", i + 1);
        scanf("%d", &tasks[i].period);
    }

    printf("\n");

    for (int i = 0; i < n; i++)
        max_time = lcm(max_time, tasks[i].period);

    struct job *jobs = NULL;    /* all current jobs, EDF order */
    struct job *running = NULL; /* job on the CPU, or NULL when idle */
    long created = 0, completed = 0, total_wait = 0, max_late = 0;

    /* One pass = what happens at time t, then 1 ms of running. Order matters (PDF). */
    for (long t = 0; t <= max_time; t++) {
        if (running != NULL && running->remaining == 0) {
            printf("%ld: process %d ends\n", t, running->pid);
            completed++;
            if (t - running->deadline > max_late)
                max_late = t - running->deadline;
            list_remove(&jobs, running);
            running = NULL;
        }

        if (t == max_time)
            break;

        for (struct job *j = jobs; j != NULL; j = j->next)
            if (j->deadline == t)
                printf("%ld: process %d missed deadline (%d ms left)\n", t, j->pid, j->remaining);

        int released = 0;
        for (int i = 0; i < n; i++) {
            if (t % tasks[i].period == 0) {
                list_insert(&jobs, job_new(i + 1, tasks[i].cpu, t, t + tasks[i].period));
                created++;
                released = 1;
            }
        }
        if (released)
            list_print(t, jobs);

        if (jobs != running) {
            if (running != NULL)
                printf("%ld: process %d preempted!\n", t, running->pid);
            if (jobs != NULL)
                printf("%ld: process %d starts\n", t, jobs->pid);
            running = jobs;
        }

        for (struct job *j = jobs; j != NULL; j = j->next)
            if (j != running)
                total_wait++;

        if (running != NULL)
            running->remaining--;
    }

    printf("%ld: max time reached\n", max_time);
    list_print(max_time, jobs);
    printf("Number of processes created: %ld\n", created);
    printf("Total waiting time: %ld\n", total_wait);
    printf("Average waiting time: %.2lf\n", (double)total_wait / created);
    printf("Number of processes completed: %ld\n", completed);
    printf("Maximum lateness: %ld\n", max_late);
    list_free(jobs);

    free(tasks);
    return 0;
}
