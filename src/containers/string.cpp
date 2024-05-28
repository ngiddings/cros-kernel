#include "string.h"

string::string()
{
    s = strdup("");
}

string::string(const char *other)
{
    s = strdup(other);
}

string::string(const string &other)
{
    s = strdup(other.s);
}

string::~string()
{
    if (s != nullptr)
        delete[] s;
}

int string::size() const
{
    return strlen(s);
}

bool string::operator==(const string &other) const
{
    return strcmp(s, other.s);
}

bool string::operator!=(const string &other) const
{
    return strcmp(s, other.s);
}

string &string::operator=(char c)
{
    if (s != nullptr)
        delete[] s;
    char cnvrt[2] = {c, '\0'};
    s = strdup(cnvrt);
    return *this;
}

string &string::operator=(const char *other)
{
    if (s != nullptr)
        delete[] s;
    s = strdup(other);
    return *this;
}

string &string::operator=(const string &other)
{
    if (s != nullptr)
        delete[] s;
    s = strdup(other.s);
    return *this;
}

string string::operator+(char c) const
{
    string result;
    result.s = new char[strlen(s) + 1 + 1];
    strcpy(result.s, s);
    char cnvrt[2] = {c, '\0'};
    strcat(result.s, cnvrt);
    return result;
}

string string::operator+(const char *other) const
{
    string result;
    result.s = new char[strlen(s) + strlen(other) + 1];
    strcpy(result.s, s);
    strcat(result.s, other);
    return result;
}

string string::operator+(const string &other) const
{
    return *this + other.s;
}

string &string::operator+=(char c)
{
    char *temp = new char[strlen(s) + 1 + 1];
    strcpy(temp, s);
    char cnvrt[2] = {c, '\0'};
    strcat(temp, cnvrt);
    if (s != nullptr)
        delete[] s;
    s = temp;
    return *this;
}

string &string::operator+=(const char *other)
{
    char *temp = new char[strlen(s) + strlen(other) + 1];
    strcpy(temp, s);
    strcat(temp, other);
    if (s != nullptr)
        delete[] s;
    s = temp;
    return *this;
}

string &string::operator+=(const string &other)
{
    return *this += other.s;
}

const char &string::operator[](int index) const
{
    return s[index];
}

const char *string::c_str() const
{
    return s;
}

string string::substr(int start_index) const
{
    if (start_index < 0 || start_index >= strlen(s)) // invalid index
        return string();
    return string(s + start_index); // pointer arithmatic, eat your heart out rust
}

string string::substr(int start_index, int end_index) const
{
    if (start_index < 0 || start_index >= strlen(s) || end_index <= start_index || end_index > strlen(s)) // invalid indices
        return string();

    int length = end_index - start_index;
    char *temp = new char[length + 1];
    strncpy(temp, s + start_index, length);
    temp[length] = '\0'; // make sure to null-terminate the string
    return string(temp);
}

bool string::strcmp(const char *first, const char *second)
{
    while (*first && *second && *first == *second)
    {
        first++;
        second++;
    }
    return *first == *second;
}

char *string::substr(const char *str, int start_index)
{
    int len = strlen(str + start_index);
    char *substr = new char[len + 1];
    strcpy(substr, str + start_index);
    return substr;
}

char *string::substr(const char *str, int start_index, int end_index)
{
    int len = end_index - start_index;
    char *substr = new char[len + 1];
    strncpy(substr, str + start_index, len);
    substr[len] = '\0';
    return substr;
}

char *string::strdup(const char *s)
{
    int len = strlen(s);
    char *dup = new char[len + 1];
    strcpy(dup, s);
    return dup;
}

void string::strcpy(char *&dest, const char *src)
{
    if (dest != nullptr)
        delete[] dest;

    int len = strlen(src);
    dest = new char[len + 1];
    char *temp = dest;

    while (*src)
    {
        *temp = *src;
        temp++;
        src++;
    }
    *temp = '\0';
}

void string::strncpy(char *&dest, const char *src, int len)
{
    if (dest != nullptr)
        delete[] dest;

    dest = new char[len + 1];
    char *temp = dest;

    while (*src && len > 0)
    {
        *temp = *src;
        temp++;
        src++;
        len--;
    }
    *temp = '\0';
}

void string::strcat(char *&dest, const char *src)
{
    if (dest == nullptr)
    {
        dest = strdup(src);
        return;
    }

    int len_dest = strlen(dest);
    int len_src = strlen(src);
    char *temp = new char[len_dest + len_src + 1];

    char *temp_ptr = temp;
    const char *dest_ptr = dest;
    const char *src_ptr = src;

    while (*dest_ptr)
    {
        *temp_ptr = *dest_ptr;
        temp_ptr++;
        dest_ptr++;
    }

    while (*src_ptr)
    {
        *temp_ptr = *src_ptr;
        temp_ptr++;
        src_ptr++;
    }
    *temp_ptr = '\0';

    delete[] dest;
    dest = temp;
}

int string::strlen(const char *s)
{
    int len = 0;
    while (*s)
    {
        len++;
        s++;
    }
    return len;
}
