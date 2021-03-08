#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Mutex for shared taskqueue
static pthread_mutex_t mtx_task = PTHREAD_MUTEX_INITIALIZER;

// Condition - New task in queue
static pthread_cond_t cond_newtask = PTHREAD_COND_INITIALIZER;

// Condition - Wait for threads to terminate
static pthread_cond_t cond_finished = PTHREAD_COND_INITIALIZER;

// Flag indicating to consumer threads to stop execution
static volatile int quit = 0;

// Counter used for setting task id
static volatile int taskcounter = 0;

// Struct for simulating a task
struct task_T
{
    int id;              // Task id
    int processtime;     // Simulated time the process should take
    struct task_T *next; // Next task in queue - used internal for queue
};

// FIFO list of tasks to do for threads
struct tasklist_T
{
    struct task_T *firstentry;
} taskqueue;

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

// Thread who created tasks
static void *thread_taskgenerator(void *arg)
{
    int gensleep, taskload;
    unsigned int gen_seed = 200;
    struct task_T *task;

    for (int i = 0; i < 10; i++)
    {
        // Sleep between 0-2 seconds
        // rand_r reentrant
        gensleep = rand_r(&gen_seed) % 3;
        sleep(gensleep);

        // Taskload between 1-4 seconds
        taskload = 1 + rand_r(&gen_seed) % 4;

        // Create new task
        task = (struct task_T *)malloc(sizeof(struct task_T));
        task->processtime = taskload;
        task->id = taskcounter++; // Safe, only incremented here

        pthread_mutex_lock(&mtx_task);
        task_push(task);
        printf("TaskGen: Task%d created of length %d\n", task->id, task->processtime);
        pthread_mutex_unlock(&mtx_task);
        pthread_cond_broadcast(&cond_newtask); // Tell consumer there are new tasks in list
    }

    // Prevents race condition with consumer when quit flag is set
    pthread_mutex_lock(&mtx_task);
    quit = 1;
    pthread_mutex_unlock(&mtx_task);
    pthread_cond_broadcast(&cond_newtask);
}

static void cleanup_consumer_cancle(void *arg)
{
    printf("Consumer cancled, freeing allocated memory\n");
    task_destroy(arg);
}

// Thread who consumes tasks
static void *thread_taskconsumer(void *arg)
{
    int s;
    struct task_T *task;

    pthread_mutex_lock(&mtx_task);
    while (!quit || task_hasnext())
    {
        while (!task_hasnext())
        {
            s = pthread_cond_wait(&cond_newtask, &mtx_task);
            if (s != 0)
            {
                exit(-1);
            }
        }
        task = task_next();

        // If thread is cancled during processing make sure allocated memory is freed
        pthread_cleanup_push(cleanup_consumer_cancle, task);

        printf("TaskCon: Task%d of length %d acquired\n", task->id, task->processtime);

        pthread_mutex_unlock(&mtx_task);

        sleep(task->processtime);
        printf("TaskCon: Task%d of length %d consumed\n", task->id, task->processtime);

        pthread_cleanup_pop(0);
        task_destroy(task);

        pthread_mutex_lock(&mtx_task);
    }
    pthread_mutex_unlock(&mtx_task);
}

int main(int argc, char **argv)
{

    pthread_t generator_thread, consumer1, consumer2, consumer3;

    // Thread generator should create 100 tasks before exit
    pthread_create(&generator_thread, NULL, thread_taskgenerator, NULL);

    // Task consumer 1
    pthread_create(&consumer1, NULL, thread_taskconsumer, NULL);
    // Task consumer 2
    pthread_create(&consumer2, NULL, thread_taskconsumer, NULL);
    // Task consumer 3
    pthread_create(&consumer3, NULL, thread_taskconsumer, NULL);

    // Wait for generator to return (all tasks generated)
    pthread_join(generator_thread, NULL);

    // Cancle consumer 1
    pthread_cancel(consumer1);
    pthread_join(consumer1, NULL);

    // Wait for consumer 2 & 3 to finish
    pthread_join(consumer2, NULL);
    pthread_join(consumer3, NULL);

    printf("Main terminating\n");
}
