
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
};

// Global taskqueue - global variables should be avoided
extern struct tasklist_T taskqueue;

// Create new task
struct task_T *task_create(int taskload);

// True if there are tasks in the queue
int task_hasnext();

// Get next task in queue
struct task_T *task_next();

// Get last task in queue - used for push
struct task_T *task_last();

// Push new task to queue
void task_push(struct task_T *task);

// Frees allocated memory
void task_destroy(struct task_T *task);