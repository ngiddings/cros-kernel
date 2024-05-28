#include "queue.h"
#include "process.h"

node::node(kernel::sched::Process *value) : value(value), prev(nullptr), next(nullptr){};

queue::queue()
{
    queue_size = 0;
    linked_list_front = linked_list_back = nullptr;
    cur_Process = nullptr;
};

void queue::enqueue(kernel::sched::Process *process)
{
    node *new_node = new node(process);
    if (empty())
    {
        linked_list_front = new_node;
    }
    else
    {
        node *last_node = linked_list_back;
        last_node->next = new_node;
        new_node->prev = last_node;
    }
    linked_list_back = new_node;
    queue_size++;
}

kernel::sched::Process *queue::dequeue()
{
    if (queue_size == 0)
    {
        return nullptr;
    }
    node *return_node = linked_list_front;
    linked_list_front = linked_list_front->next;
    if (linked_list_back == return_node)
    {
        linked_list_back = nullptr;
    }
    kernel::sched::Process *p = return_node->value;
    delete return_node;
    queue_size--;
    return p;
}

kernel::sched::Process *queue::remove(pid_t pid)
{
    node *cur_node = linked_list_front;
    kernel::sched::Process *p;

    while (cur_node->next)
    {
        if (cur_node->value->getPid() == pid)
        {
            cur_node->prev->next = cur_node->next;
            cur_node->next->prev = cur_node->prev;
            p = cur_node->value;
            delete cur_node;
            return p;
        }
        cur_node = cur_node->next;
    }
}

kernel::sched::Process *queue::peek()
{
    return linked_list_front->value;
}

kernel::sched::Process *queue::sched_next()
{
    if (cur_Process != nullptr)
    {
        enqueue(cur_Process);
    }
    if (!empty())
    {
        cur_Process = dequeue();
        return cur_Process;
    }
    return nullptr;
}

int queue::size() const
{
    return queue_size;
}

bool queue::empty() const
{
    return queue_size == 0 ? true : false;
}

kernel::sched::Process *queue::get_cur_process()
{
    return cur_Process;
}

void queue::set_cur_process(kernel::sched::Process *proc)
{
    cur_Process = proc;
}