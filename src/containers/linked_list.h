#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "string.h"

template <typename T>
class linked_list
{
public:
    linked_list();
    ~linked_list();

    bool empty() const;
    int size() const;

    void push_front(T& data);
    void push_back(T& data);
    T& pop_front();
    T& pop_back();
    T& peek_front() const;
    T& peek_back() const;

    void insert(int index, T &data); // inserts new element before the existing element at the given index
    void replace(int index, T &data);
    void remove(int index);

    void clear();

    T &operator[](int index) const;

    string to_string() const;

   private:
    class node {
       public:
        T& value;
        node* prev;
        node* next;

        node(T &value);

    private:
    };

    static T default_value;

    node *head;
    node *tail;
    int _size;
};

template <typename T>
linked_list<T>::node::node(T &value)
    : value(value), prev(nullptr), next(nullptr) {}

template <typename T>
T linked_list<T>::default_value = 0x0;//CHECKME Legal?

template <typename T>
linked_list<T>::linked_list()
    : head(nullptr), tail(nullptr), _size(0) {}

template <typename T>
linked_list<T>::~linked_list()
{
}

template <typename T>
bool linked_list<T>::empty() const
{
    return _size == 0;
}

template <typename T>
int linked_list<T>::size() const
{
    return _size;
}

template <typename T>
void linked_list<T>::push_front(T &data)
{
    node *new_node = new node(data);

    if (head == nullptr)
    {
        head = new_node;
        tail = new_node;
    }
    else if (head == tail)
    {
        head = new_node;
        head->next = tail;
        tail->prev = head;
    }
    else
    {
        head->prev = new_node;
        new_node->next = head;
        head = new_node;
    }

    _size++;
}

template <typename T>
void linked_list<T>::push_back(T &data)
{
    node *new_node = new node(data);

    if (tail == nullptr)
    {
        tail = new_node;
        head = new_node;
    }
    else if (tail == head)
    {
        tail = new_node;
        tail->prev = head;
        head->next = tail;
    }
    else
    {
        tail->next = new_node;
        new_node->prev = tail;
        tail = new_node;
    }

    _size++;
}

template <typename T>
T& linked_list<T>::pop_front() {
    T& data = default_value;

    if (head == nullptr)
    {
        data = default_value;
    }
    else if (head == tail)
    {
        data = head->value;
        delete head;
        head = nullptr;
        tail = nullptr;
    }
    else
    {
        data = head->value;
        head = head->next;
        delete head->prev;
        head->prev = nullptr;
    }

    _size--;
    return data;
}

template <typename T>
T& linked_list<T>::pop_back() {
    T& data = default_value;

    if (tail == nullptr)
    {
        data = default_value;
    }
    else if (tail == head)
    {
        data = tail->value;
        delete tail;
        tail = nullptr;
        head = nullptr;
    }
    else
    {
        data = tail->value;
        tail = tail->prev;
        delete tail->next;
        tail->next = nullptr;
    }

    _size--;
    return data;
}

template <typename T>
T& linked_list<T>::peek_front() const {
    if (head == nullptr)
        return default_value;
    else
        return head->value;
}

template <typename T>
T& linked_list<T>::peek_back() const {
    if (tail == nullptr)
        return default_value;
    else
        return tail->value;
}

template <typename T>
void linked_list<T>::insert(int index, T &data)
{
    if (index > _size)
        return;

    node *cur = head;
    for (int i = 0; i < index; i++)
        cur = cur->next;

    node *new_node = new node(data);
    if (index == _size && _size == 0)
    {
        head = new_node;
        tail = new_node;
    }
    else if (index == _size && _size == 1)
    {
        tail = new_node;
        head->next = tail;
        tail->prev = head;
    }
    else if (index == _size && _size > 1)
    {
        tail->next = new_node;
        new_node->prev = tail;
        tail = new_node;
    }
    else if (index == 0)
    {
        head->prev = new_node;
        new_node->next = head;
        head = new_node;
    }
    else
    {
        cur->prev->next = new_node;
        new_node->prev = cur->prev;
        cur->prev = new_node;
        new_node->next = cur;
    }

    _size++;
}

template <typename T>
void linked_list<T>::replace(int index, T &data)
{
    if (index >= _size)
        return;

    node *cur = head;
    for (int i = 0; i < index; i++)
        cur = cur->next;

    cur->value = data;
}

template <typename T>
void linked_list<T>::remove(int index)
{
    if (index >= _size)
        return;

    node *cur = head;
    for (int i = 0; i < index; i++)
        cur = cur->next;

    if (head == tail)
    {
        delete head;
        head = nullptr;
        tail = nullptr;
    }
    else if (cur == head)
    {
        head = head->next;
        delete head->prev;
        head->prev = nullptr;
    }
    else if (cur == tail)
    {
        tail = tail->prev;
        delete tail->next;
        tail->next = nullptr;
    }
    else
    {
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
        delete cur;
    }

    _size--;
}

template <typename T>
void linked_list<T>::clear()
{
    node *cur = head;
    while (cur != nullptr)
    {
        if (cur->next != nullptr)
        {
            cur = cur->next;
            delete cur->prev;
        }
        else
        {
            delete cur;
            cur = nullptr;
        }
    }
    head = nullptr;
    tail = nullptr;
    _size = 0;
}

template <typename T>
T &linked_list<T>::operator[](int index) const
{
    if (index >= _size)
        return default_value;

    node *cur = head;
    for (int i = 0; i < index; i++)
        cur = cur->next;

    return cur->value;
}

template <typename T>
string linked_list<T>::to_string() const {
    string str;

    node* cur = head;
    str += "[";
    for (int i = 0; i < _size; i++) {
        str += '0' + cur->value;
        if (i != _size - 1)
            str += ", ";
        cur = cur->next;
    }
    str += "]";

    return str;
}

#endif