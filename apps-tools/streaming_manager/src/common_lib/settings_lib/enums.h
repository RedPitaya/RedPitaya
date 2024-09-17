#ifndef SETTINGS_LIB_ENUMS_H
#define SETTINGS_LIB_ENUMS_H

#include <iostream>
#include <cstddef>
#include <cstring>

#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...)                                    \
  __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, a2, ...)                     \
  macro(a1, a2)                                                 \
  __VA_OPT__(, FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER
// Magic ends

#define EXTRACT_SECOND(x, y) y

#define EXTRACT_FIRST(x, y) x

#define EXTRACT_EACH_SECOND(...) FOR_EACH(EXTRACT_SECOND, __VA_ARGS__)

#define EXTRACT_EACH_FIRST(...) FOR_EACH(EXTRACT_FIRST, __VA_ARGS__)

#define MAP(macro, ...) \
    IDENTITY( \
        APPLY(CHOOSE_MAP_START, COUNT(__VA_ARGS__)) \
            (macro, __VA_ARGS__))

#define CHOOSE_MAP_START(count) MAP ## count

#define APPLY(macro, ...) IDENTITY(macro(__VA_ARGS__))

// Needed to expand __VA_ARGS__ "eagerly" on the MSVC preprocessor.
#define IDENTITY(x) x

#define MAP1(m, x)      m(x)
#define MAP2(m, x, ...) m(x) IDENTITY(MAP1(m, __VA_ARGS__))
#define MAP3(m, x, ...) m(x) IDENTITY(MAP2(m, __VA_ARGS__))
#define MAP4(m, x, ...) m(x) IDENTITY(MAP3(m, __VA_ARGS__))
#define MAP5(m, x, ...) m(x) IDENTITY(MAP4(m, __VA_ARGS__))
#define MAP6(m, x, ...) m(x) IDENTITY(MAP5(m, __VA_ARGS__))
#define MAP7(m, x, ...) m(x) IDENTITY(MAP6(m, __VA_ARGS__))
#define MAP8(m, x, ...) m(x) IDENTITY(MAP7(m, __VA_ARGS__))

#define EVALUATE_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, count, ...) count

#define COUNT(...) \
    IDENTITY(EVALUATE_COUNT(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1))


struct ignore_assign {
    ignore_assign(int value) : _value(value) { }
    operator int() const { return _value; }

    const ignore_assign& operator =(int) { return *this; }

    int _value;
};

#define IGNORE_ASSIGN_SINGLE(expression) (ignore_assign)expression,
#define IGNORE_ASSIGN(...) IDENTITY(MAP(IGNORE_ASSIGN_SINGLE, __VA_ARGS__))

#define STRINGIZE_SINGLE(expression) #expression,
#define STRINGIZE(...) IDENTITY(MAP(STRINGIZE_SINGLE, __VA_ARGS__))


#define ENUM(EnumName, ...)                                                         \
struct EnumName {                                                                   \
    enum _enumerated { EXTRACT_EACH_FIRST(__VA_ARGS__) };                           \
                                                                                    \
    _enumerated     value;                                                          \
                                                                                    \
    EnumName(_enumerated _value) : value(_value) { }                                \
    operator _enumerated() const { return value; }                                  \
                                                                                    \
    const char* name() const                                                        \
    {                                                                               \
        for (size_t index = 0; index < count; ++index) {                            \
            if (values()[index] == value)                                           \
                return names()[index];                                              \
        }                                                                           \
                                                                                    \
        return NULL;                                                                \
    }                                                                               \
                                                                                    \
    const char* to_string() const noexcept(false)                                   \
    {                                                                               \
        for (size_t index = 0; index < count; ++index) {                            \
            if (values()[index] == value)                                           \
                return strings()[index];                                            \
        }                                                                           \
                                                                                    \
        throw std::invalid_argument( "Not found" );                                 \
    }                                                                               \
    static _enumerated from_string(const std::string &name) noexcept(false)         \
    {                                                                               \
        for (size_t index = 0; index < count; ++index) {                            \
            if (strcmp(names()[index],name.c_str()) == 0)                           \
                return (_enumerated)values()[index];                                \
        }                                                                           \
        throw std::invalid_argument( "Not found" );                                 \
    }                                                                               \
                                                                                    \
    static const size_t count = IDENTITY(COUNT(EXTRACT_EACH_FIRST(__VA_ARGS__)));   \
                                                                                    \
    static const int* values()                                                      \
    {                                                                               \
        static const int _values[] =                                                \
            { IDENTITY(IGNORE_ASSIGN(EXTRACT_EACH_FIRST(__VA_ARGS__))) };           \
        return _values;                                                             \
    }                                                                               \
                                                                                    \
    static const char* const* names()                                               \
    {                                                                               \
        static const char* const    raw_names[] =                                   \
            { IDENTITY(STRINGIZE(EXTRACT_EACH_FIRST(__VA_ARGS__))) };               \
                                                                                    \
        static char*                processed_names[count];                         \
        static bool                 initialized = false;                            \
                                                                                    \
        if (!initialized) {                                                         \
            for (size_t index = 0; index < count; ++index) {                        \
                size_t length =                                                     \
                    std::strcspn(raw_names[index], " =\t\n\r");                     \
                                                                                    \
                processed_names[index] = new char[length + 1];                      \
                                                                                    \
                std::memcpy(                                                        \
                    processed_names[index], raw_names[index], length);              \
                processed_names[index][length] = '\0';                              \
            }                                                                       \
            initialized = true;                                                     \
        }                                                                           \
                                                                                    \
        return processed_names;                                                     \
    }                                                                               \
    static const char* const* strings()                                             \
    {                                                                               \
        static const char* const    raw_names[] =                                   \
            { IDENTITY(EXTRACT_EACH_SECOND(__VA_ARGS__)) };                         \
                                                                                    \
        static char*                processed_names[count];                         \
        static bool                 initialized = false;                            \
                                                                                    \
        if (!initialized) {                                                         \
            for (size_t index = 0; index < count; ++index) {                        \
                size_t length =                                                     \
                    std::strcspn(raw_names[index], " =\t\n\r");                     \
                                                                                    \
                processed_names[index] = new char[length + 1];                      \
                                                                                    \
                std::memcpy(                                                        \
                    processed_names[index], raw_names[index], length);              \
                processed_names[index][length] = '\0';                              \
            }                                                                       \
            initialized = true;                                                     \
        }                                                                           \
                                                                                    \
        return processed_names;                                                     \
    }                                                                               \
                                                                                    \
    bool operator==( const EnumName &b ) {                                          \
        return value == b.value;                                                    \
    }                                                                               \
    static bool IsValid(int _value)                                                 \
    {                                                                               \
        for (size_t index = 0; index < count; ++index) {                            \
            if (values()[index] == _value) return true;                             \
        }                                                                           \
        return false;                                                               \
    }                                                                               \
};

#endif