#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include "math.h"
#include "string.h"

template <typename K, typename V>
class binary_search_tree
{
public:
    class node
    {
    public:
        binary_search_tree *tree;

        K key;
        V value;

        node *parent;
        node *left;
        node *right;

        int height;

        node(binary_search_tree *tree, K key, V &val);

        int balance() const;
        void update_height();

        node *successor() const;
        void replace(node *other);
        void unlink();

    private:
    };

    class tree_iterator
    {
    public:
        tree_iterator();

        tree_iterator(node *p);

        tree_iterator(tree_iterator &other);

        tree_iterator &operator=(tree_iterator &other);

        tree_iterator &operator++();

        bool operator!=(tree_iterator &other);

        K &operator*();

        K *operator->();

    private:
        node *p;
    };

    binary_search_tree();
    ~binary_search_tree();

    int size() const;
    bool empty() const;

    void insert(K key, V &val);
    void remove(K key);
    void clear();

    V &get(K key) const;
    V &search(K key) const;
    bool contains(K key) const;

    V &pop_min();
    V &pop_max();
    V &peek_min() const;
    V &peek_max() const;

    tree_iterator begin();
    tree_iterator end();

    string to_string() const;

private:
    node *root;
    int _size;

    node *search_rec(K key) const;
    void add_rec(node *cur, K key, V &val, bool &node_added);
    void remove_rec(node *cur, K key);
    void clear_rec(node *cur);

    void rebalance(node *node);
    void rotate_right(node *node);
    void rotate_left(node *node);

    void to_string_rec(K ***grid, node *cur, int row, int col, int height) const;
};

template <typename K, typename V>
binary_search_tree<K, V>::node::node(binary_search_tree *tree, K key, V &val)
    : tree(tree), key(key), value(val), parent(nullptr), left(nullptr), right(nullptr), height(1) {}

template <typename K, typename V>
int binary_search_tree<K, V>::node::balance() const
{
    if (left == nullptr && right == nullptr)
        return 0;
    else if (left == nullptr)
        return -right->height;
    else if (right == nullptr)
        return left->height;
    else
        return left->height - right->height;
}

template <typename K, typename V>
void binary_search_tree<K, V>::node::update_height()
{
    if (left == nullptr && right == nullptr)
        height = 1;
    else if (left == nullptr)
        height = 1 + right->height;
    else if (right == nullptr)
        height = 1 + left->height;
    else
        height = 1 + max(left->height, right->height);
}

template <typename K, typename V>
typename binary_search_tree<K, V>::node *binary_search_tree<K, V>::node::successor() const
{
    if (right != nullptr)
    {
        node *cur = right;
        while (cur->left != nullptr)
            cur = cur->left;
        return cur;
    }
    else if (left != nullptr)
    {
        node *cur = left;
        while (cur->right != nullptr)
            cur = cur->right;
        return cur;
    }
    else
    {
        return nullptr;
    }
}

template <typename K, typename V>
void binary_search_tree<K, V>::node::replace(node *other)
{
    key = other->key;
    value = other->value;

    node *succ = other->successor();
    if (succ == nullptr)
    {
        node *other_parent = other->parent;
        other->unlink();
        delete other;
        tree->rebalance(other_parent);
    }
    else
    {
        node *succ_parent = succ->parent;
        other->replace(succ);
        tree->rebalance(succ_parent);
    }
}

template <typename K, typename V>
void binary_search_tree<K, V>::node::unlink()
{
    if (parent != nullptr)
    {
        if (parent->left == this)
            parent->left = nullptr;
        else
            parent->right = nullptr;
    }
}

template <typename K, typename V>
binary_search_tree<K, V>::binary_search_tree()
    : root(nullptr), _size(0) {}

template <typename K, typename V>
binary_search_tree<K, V>::~binary_search_tree()
{
    clear();
}

template <typename K, typename V>
int binary_search_tree<K, V>::size() const
{
    return _size;
}

template <typename K, typename V>
bool binary_search_tree<K, V>::empty() const
{
    return _size == 0;
}

template <typename K, typename V>
void binary_search_tree<K, V>::insert(K key, V &val)
{
    bool node_added = false;
    add_rec(root, key, val, node_added);
}

template <typename K, typename V>
void binary_search_tree<K, V>::remove(K key)
{
    remove_rec(root, key);
}

template <typename K, typename V>
V &binary_search_tree<K, V>::get(K key) const
{
    binary_search_tree<K, V>::node *node = search_rec(key);
    if (node == nullptr || node->key != key)
        return root->value;
    else
        return node->value;
}

template <typename K, typename V>
V &binary_search_tree<K, V>::search(K key) const
{
    binary_search_tree<K, V>::node *node = search_rec(key);
    if (node == nullptr)
        return root->value;
    else
        return node->value;
}

template <typename K, typename V>
bool binary_search_tree<K, V>::contains(K key) const
{
    binary_search_tree<K, V>::node *node = search_rec(key);
    if (node == nullptr || node->key != key)
        return false;
    else
        return true;
}

template <typename K, typename V>
void binary_search_tree<K, V>::clear()
{
    clear_rec(root);
    root = nullptr;
    _size = 0;
}

template <typename K, typename V>
V &binary_search_tree<K, V>::peek_min() const
{
    node *cur = root;

    if (cur == nullptr)
        return root->value;

    while (cur->left != nullptr)
        cur = cur->left;

    return cur->value;
}

template <typename K, typename V>
V &binary_search_tree<K, V>::peek_max() const
{
    node *cur = root;

    if (cur == nullptr)
        return root->value;

    while (cur->right != nullptr)
        cur = cur->right;

    return cur->value;
}

template <typename K, typename V>
inline typename binary_search_tree<K, V>::tree_iterator binary_search_tree<K, V>::begin()
{
    if (root == nullptr)
    {
        return tree_iterator();
    }
    node *p = root;
    while (p->left != nullptr)
    {
        p = p->left;
    }
    return tree_iterator(p);
}

template <typename K, typename V>
inline typename binary_search_tree<K, V>::tree_iterator binary_search_tree<K, V>::end()
{
    return tree_iterator();
}

template <typename K, typename V>
V &binary_search_tree<K, V>::pop_min()
{
    node *cur = root;

    if (cur == nullptr)
        return root->value;

    while (cur->left != nullptr)
        cur = cur->left;

    V &val = cur->value;
    remove_rec(root, cur->key);

    return val;
}

template <typename K, typename V>
V &binary_search_tree<K, V>::pop_max()
{
    node *cur = root;

    if (cur == nullptr)
        return root->value;

    while (cur->right != nullptr)
        cur = cur->right;

    V &val = cur->value;
    remove_rec(root, cur->key);

    return val;
}

template <typename K, typename V>
string binary_search_tree<K, V>::to_string() const
{
    string str = "";

    if (root == nullptr)
        return str;

    int height = root->height;
    int cols = (1 << height) - 1;
    K ***grid = new K **[height];
    for (int i = 0; i < height; i++)
        grid[i] = new K *[cols];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            grid[i][j] = nullptr;
        }
    }

    to_string_rec(grid, root, 0, cols / 2, height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (grid[i][j] == nullptr)
            {
                str += "  ";
            }
            else
            {
                str += ('0' + *(grid[i][j]));
                str += " ";
            }
        }
        if (i != height - 1)
            str += "\n";
    }

    return str;
}

template <typename K, typename V>
typename binary_search_tree<K, V>::node *binary_search_tree<K, V>::search_rec(K key) const
{
    binary_search_tree<K, V>::node *cur = root;
    while (cur != nullptr)
    {
        if (cur->key > key)
        {
            if (cur->left == nullptr)
                return cur;
            cur = cur->left;
        }
        else if (cur->key < key)
        {
            if (cur->right == nullptr)
                return cur;
            cur = cur->right;
        }
        else
        {
            return cur;
        }
    }
    return nullptr;
}

template <typename K, typename V>
void binary_search_tree<K, V>::add_rec(node *cur, K key, V &val, bool &node_added)
{
    if (root == nullptr)
    {
        _size++;
        node_added = true;
        root = new node(this, key, val);
        cur = root;
    }
    else if (cur->key > key)
    {
        if (cur->left != nullptr)
        {
            add_rec(cur->left, key, val, node_added);
        }
        else
        {
            _size++;
            node_added = true;
            cur->left = new node(this, key, val);
            cur->left->parent = cur;
        }
    }
    else if (cur->key < key)
    {
        if (cur->right != nullptr)
        {
            add_rec(cur->right, key, val, node_added);
        }
        else
        {
            _size++;
            node_added = true;
            cur->right = new node(this, key, val);
            cur->right->parent = cur;
        }
    }
    else
    {
        if (node_added)
        {
            return;
        }
        else if (cur->right != nullptr)
        {
            add_rec(cur->right, key, val, node_added);
        }
        else
        {
            _size++;
            node_added = true;
            cur->right = new node(this, key, val);
            cur->right->parent = cur;
        }
    }

    rebalance(cur);
}

template <typename K, typename V>
void binary_search_tree<K, V>::remove_rec(node *cur, K key)
{
    if (cur == nullptr)
        return;

    if (cur->key > key)
    {
        remove_rec(cur->left, key);
    }
    else if (cur->key < key)
    {
        remove_rec(cur->right, key);
    }
    else
    {
        node *succ = cur->successor();
        if (succ == nullptr)
        {
            if (root == cur)
                root = nullptr;
            node *cur_parent = cur->parent;
            cur->unlink();
            delete cur;
            cur = nullptr;
            if (cur_parent != nullptr)
                rebalance(cur_parent);
        }
        else
        {
            node *succ_parent = succ->parent;
            cur->replace(succ);
            rebalance(succ_parent);
        }
        _size--;
    }

    if (cur == nullptr)
        return;

    rebalance(cur);
}

template <typename K, typename V>
void binary_search_tree<K, V>::clear_rec(node *cur)
{
    if (cur->left != nullptr && cur->right != nullptr)
    {
        clear_rec(cur->left);
        clear_rec(cur->right);
    }
    else if (cur->left != nullptr)
    {
        clear_rec(cur->left);
    }
    else if (cur->right != nullptr)
    {
        clear_rec(cur->right);
    }
    delete cur;
}

template <typename K, typename V>
void binary_search_tree<K, V>::rebalance(node *node)
{
    node->update_height();
    int bal = node->balance();

    if (bal > 1)
    {
        if (node->left != nullptr && node->left->balance() < 0)
            rotate_left(node->left);
        rotate_right(node);
    }
    else if (bal < -1)
    {
        if (node->right != nullptr && node->right->balance() > 0)
            rotate_right(node->right);
        rotate_left(node);
    }
}

template <typename K, typename V>
void binary_search_tree<K, V>::rotate_right(node *node)
{
    binary_search_tree<K, V>::node *parent = node->parent;
    binary_search_tree<K, V>::node *left = node->left;
    binary_search_tree<K, V>::node *left_right = left->right;

    left->right = node;
    node->left = left_right;

    left->parent = parent;
    node->parent = left;
    if (left_right != nullptr)
        left_right->parent = node;

    if (parent != nullptr)
    {
        if (parent->left == node)
            parent->left = left;
        else
            parent->right = left;
    }
    else
    {
        root = left;
    }

    node->update_height();
    left->update_height();
}

template <typename K, typename V>
void binary_search_tree<K, V>::rotate_left(node *node)
{
    binary_search_tree<K, V>::node *parent = node->parent;
    binary_search_tree<K, V>::node *right = node->right;
    binary_search_tree<K, V>::node *right_left = right->left;

    right->left = node;
    node->right = right_left;

    right->parent = parent;
    node->parent = right;
    if (right_left != nullptr)
        right_left->parent = node;

    if (parent != nullptr)
    {
        if (parent->left == node)
            parent->left = right;
        else
            parent->right = right;
    }
    else
    {
        root = right;
    }

    node->update_height();
    right->update_height();
}

template <typename K, typename V>
void binary_search_tree<K, V>::to_string_rec(K ***grid, node *cur, int row, int col, int height) const
{
    if (cur == nullptr)
        return;

    grid[row][col] = &cur->key;
    to_string_rec(grid, cur->left, row + 1, col - pow(2, height - 2), height - 1);
    to_string_rec(grid, cur->right, row + 1, col + pow(2, height - 2), height - 1);
}

template <typename K, typename V>
inline binary_search_tree<K, V>::tree_iterator::tree_iterator()
    : p(nullptr)
{
}

template <typename K, typename V>
inline binary_search_tree<K, V>::tree_iterator::tree_iterator(node *p)
    : p(p)
{
}

template <typename K, typename V>
inline binary_search_tree<K, V>::tree_iterator::tree_iterator(tree_iterator &other)
    : p(other.p)
{
}

template <typename K, typename V>
inline typename binary_search_tree<K, V>::tree_iterator &binary_search_tree<K, V>::tree_iterator::operator=(tree_iterator &other)
{
    p = other.p;
    return *this;
}

template <typename K, typename V>
inline typename binary_search_tree<K, V>::tree_iterator &binary_search_tree<K, V>::tree_iterator::operator++()
{
    if (p->right != nullptr)
    {
        p = p->right;
        while (p->left != nullptr)
        {
            p = p->left;
        }
    }
    else
    {
        while (true)
        {
            if (p->parent == nullptr)
            {
                p = nullptr;
                break;
            }
            else if (p->parent->left == p)
            {
                p = p->parent;
                break;
            }
            else
            {
                p = p->parent;
            }
        }
    }
    return *this;
}

template <typename K, typename V>
inline bool binary_search_tree<K, V>::tree_iterator::operator!=(tree_iterator &other)
{
    return p != other.p;
}

template <typename K, typename V>
inline K &binary_search_tree<K, V>::tree_iterator::operator*()
{
    return p->key;
}

template <typename K, typename V>
inline K *binary_search_tree<K, V>::tree_iterator::operator->()
{
    return &p->key;
}

#endif
