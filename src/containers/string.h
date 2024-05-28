#ifndef STRING_H
#define STRING_H

class string
{
public:
    string();
    string(const char *other);
    string(const string &other);
    ~string();

    int size() const;

    bool operator==(const string &other) const;
    bool operator!=(const string &other) const;

    string &operator=(char c);
    string &operator=(const char *other);
    string &operator=(const string &other);
    string operator+(char other) const;
    string operator+(const char *other) const;
    string operator+(const string &other) const;
    string &operator+=(char c);
    string &operator+=(const char *other);
    string &operator+=(const string &other);

    const char &operator[](int index) const;

    const char *c_str() const;

    string substr(int start_index) const;
    string substr(int start_index, int end_index) const;

    static bool strcmp(const char *first, const char *second); // differs from standard implemtation
    static char *substr(const char *str, int start_index);
    static char *substr(const char *str, int start_index, int end_index);
    static char *strdup(const char *s);
    static void strcpy(char *&dest, const char *src);           // differs from standard implemtation, allocates more memory in destination string if its not big enough
    static void strncpy(char *&dest, const char *src, int len); // ^
    static void strcat(char *&dest, const char *src);           // ^
    static int strlen(const char *s);                           // doesn't include the null-terminator

private:
    char *s;
};

#endif