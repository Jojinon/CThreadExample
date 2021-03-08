#include <stdlib.h>
#include "task.h"

// Counter used for setting task id
static volatile int taskcounter = 0;

// Definition of global taskqueue
struct tasklist_T taskqueue;

struct task_T *task_create(int taskload)
{
    struct task_T *task;
    task = (struct task_T *)malloc(sizeof(struct task_T));
    task->processtime = taskload;
    task->id = taskcounter++; // Safe, only incremented by one thread, otherwise not safe.
    return task;
}

// True if there are tasks in the queue
int task_hasnext()
{
    return taskqueue.firstentry != NULL;
}

// Get next task in queue
struct task_T *task_next()
{
    struct task_T *task;
    task = taskqueue.firstentry;
    taskqueue.firstentry = task->next;
    return task;
}

// Get last task in queue - used for push
struct task_T *task_last()
{
    struct task_T *task;
    task = taskqueue.firstentry;
    while (task->next != NULL)
        task = task->next;
    return task;
}

// Push new task to queue
void task_push(struct task_T *task)
{
    struct task_T *lasttask;
    if (taskqueue.firstentry == NULL)
        taskqueue.firstentry = task;
    else
    {
        lasttask = task_last();
        lasttask->next = task;
    }
}

// Frees allocated memory
void task_destroy(struct task_T *task)
{
    free(task);
}