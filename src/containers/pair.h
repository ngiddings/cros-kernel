#ifndef PAIR_H
#define PAIR_H

template <typename T, typename U>
struct pair
{
    T first;
    U second;

    pair()
        : first(T()), second(U()){};

    pair(const T &first, const U &second)
        : first(first), second(second) {}

    bool operator==(const pair<T, U> &other) const
    {
        return first == other.first && second == other.second;
    }

    bool operator!=(const pair<T, U> &other) const
    {
        return !(*this == other);
    }
};

#endif