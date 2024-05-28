#ifndef QUEUE_H
#define QUEUE_H

#include "containers/linked_list.h"
#include "process.h"

class node
{
public:
    node(kernel::sched::Process *value);
    kernel::sched::Process *value;
    node *prev;
    node *next;
};

class queue
{
public:
    queue();
    void enqueue(kernel::sched::Process *process);
    kernel::sched::Process *dequeue();
    kernel::sched::Process *remove(pid_t pid);
    kernel::sched::Process *peek();
    kernel::sched::Process *sched_next();
    kernel::sched::Process *get_cur_process();
    void set_cur_process(kernel::sched::Process *proc);
    bool empty() const;
    int size() const;

private:
    int queue_size;
    node *linked_list_front;
    node *linked_list_back;
    kernel::sched::Process *cur_Process;
};

#endif