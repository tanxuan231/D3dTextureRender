#ifndef SINGLETON_H
#define SINGLETON_H

#include <mutex>

template <typename T>
class Singleton {
public:
    static T& Instance()
    {
        if (m_instance == nullptr) {
            std::unique_lock<std::mutex> unique_locker(m_mutex);
            if (m_instance == nullptr) {
                m_instance.reset(new T());
            }
        }
        return *m_instance;
    }

    Singleton(const Singleton &other);
    Singleton<T>& operator=(const Singleton &other);

private:
    static std::mutex m_mutex;
    static std::unique_ptr<T> m_instance;
};

template <typename T> std::mutex Singleton<T>::m_mutex;
template <typename T> std::unique_ptr<T> Singleton<T>::m_instance;

// MUST implement the default ctor and dtor
#define SINGLETON(T)                  \
public:                               \
    ~T();                             \
                                      \
private:                              \
    explicit T();                     \
    T(const T&) = delete;             \
    T(const T&&) = delete;            \
    T& operator=(const T&) = delete;  \
    T& operator=(const T&&) = delete; \
                                      \
    friend class Singleton<T>;

#endif // SINGLETON_H
