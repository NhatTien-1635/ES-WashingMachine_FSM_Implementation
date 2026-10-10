#include "scheduler.h"

typedef uint8_t EntryIndex;
struct SchedularEntry
{
    void (*task_function)() = nullptr;

    int32_t timer = 0;
    int32_t period = 0;

    EntryIndex next_entry = NONE;
};

static SchedularEntry task_table[TABLE_SIZE];

static EntryIndex ready_queue[QUEUE_SIZE];
static uint8_t ready_count = 0;
static EntryIndex queue_front = 0;
static EntryIndex queue_back = 0;

static EntryIndex head_task = NONE;

static void InsertDeltaTask(uint8_t index, int32_t delay);

void Scheduler_Dispatch()
{
    while (ready_count > 0)
    {
        // Get entry
        EntryIndex index = ready_queue[queue_front];
        queue_front = ((queue_front + 1) & (QUEUE_SIZE - 1));
        --ready_count;

        void (*func)() = task_table[index].task_function;
        if (func)
            func();

        if (task_table[index].period == 0)
        {
            task_table[index].task_function = nullptr;
        }
    }
}

/**
 * @brief Add a task to the scheduler.
 *
 * @param task   Function to run. Called from Scheduler_Dispatch(), not from the ISR.
 * @param delay  Time until the first run, in units of 10 ms.
 * @param period Time between later runs, in units of 10 ms. 0 runs the task once.
 *
 * @note The task is silently dropped if the task table is full.
 */
void Scheduler_AddTask(void (*task)(), uint32_t delay, uint32_t period)
{
    for (uint8_t index = 0; index < TABLE_SIZE; ++index)
    {
        if (task_table[index].task_function == nullptr)
        {
            task_table[index].task_function = task;
            task_table[index].period = period;
            InsertDeltaTask(index, delay);
            return;
        }
    }
}

void Scheduler_Step()
{
    if (head_task == NONE)
        return;

    // Update timer
    if (task_table[head_task].timer > 0)
        --task_table[head_task].timer;

    // Time is up
    while (head_task != NONE && task_table[head_task].timer == 0)
    {
        EntryIndex old_head = head_task;
        head_task = task_table[head_task].next_entry;

        // Prevent overwriting other tasks
        if (ready_count < QUEUE_SIZE)
        {
            // Add new task to ready queue
            ready_queue[queue_back] = old_head;
            queue_back = ((queue_back + 1) & (QUEUE_SIZE - 1));
            ++ready_count;
        }

        if (task_table[old_head].period != 0)
            InsertDeltaTask(old_head, task_table[old_head].period);
    }
}

static void InsertDeltaTask(uint8_t index, int32_t delay)
{
    EntryIndex curr = head_task;
    EntryIndex prev = NONE;

    while (curr != NONE && delay >= task_table[curr].timer)
    {
        delay -= task_table[curr].timer;
        prev = curr;
        curr = task_table[curr].next_entry;
    }

    // Main case
    task_table[index].next_entry = curr;
    task_table[index].timer = delay;

    // Update next task delay
    if (curr != NONE)
        task_table[curr].timer -= delay;

    // Node inserted at head
    if (prev == NONE)
        head_task = index;
    // Node is not added at head
    else
        task_table[prev].next_entry = index;
}