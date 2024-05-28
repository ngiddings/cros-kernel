#ifndef VECTOR_H
#define VECTOR_H

template <typename T>
class vector
{
public:
    vector();
    vector(int capacity);
    vector(const vector<T> &other);
    ~vector();

    bool empty() const;
    int size() const;
    int capacity() const;

    void push_back(const T &data);
    void pop_back();
    T &back();
    void remove(int index);

    void clear();
    void resize(int new_capacity);

    T &operator[](int index) const;

private:
    T *array;
    int _capacity;
    int _size;
};

template <typename T>
vector<T>::vector()
{
    _size = 0;
    _capacity = 8;
    array = new T[_capacity];
}

template <typename T>
vector<T>::vector(int capacity)
{
    _size = 0;
    _capacity = capacity;
    array = new T[_capacity];
}

template <typename T>
vector<T>::vector(const vector<T> &other)
{
    _size = other.size();
    _capacity = other.capacity();
    array = new T[_capacity];
    for (int i = 0; i < _size; i++)
        array[i] = other[i];
}

template <typename T>
vector<T>::~vector()
{
    if (array != nullptr)
        delete[] array;
}

template <typename T>
bool vector<T>::empty() const
{
    return _size == 0;
}

template <typename T>
int vector<T>::size() const
{
    return _size;
}

template <typename T>
int vector<T>::capacity() const
{
    return _capacity;
}

template <typename T>
T &vector<T>::operator[](int index) const
{
    return array[index];
}

template <typename T>
void vector<T>::push_back(const T &data)
{
    if (_size == _capacity)
        resize(_capacity * 2);
    array[_size] = data;
    _size++;
}

template <typename T>
void vector<T>::pop_back()
{
    array[_size - 1].~T();
    _size--;
}

template <typename T>
T &vector<T>::back()
{
    return array[_size - 1];
}

template <typename T>
void vector<T>::remove(int index)
{
    return array[index] = T();
}

template <typename T>
void vector<T>::clear()
{
    _size = 0;
    for (int i = 0; i < _capacity; i++)
        array[i].~T();
}

template <typename T>
void vector<T>::resize(int new_capacity)
{
    _capacity = new_capacity;
    T *temp = new T[_capacity];
    for (int i = 0; i < _size; i++)
        temp[i] = array[i];
    delete[] array;
    array = temp;
}

#endif