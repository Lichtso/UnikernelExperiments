static_assert(__LITTLE_ENDIAN__);

template<bool B, typename T, typename F>
struct conditional {
    typedef F type;
};
template<typename T, typename F>
struct conditional<true, T, F> {
    typedef T type;
};

typedef char unsigned Natural8;
typedef char Integer8;
typedef short unsigned Natural16;
typedef short Integer16;
typedef unsigned Natural32;
typedef int Integer32;
typedef float Float32;
typedef long long unsigned Natural64;
typedef long long int Integer64;
typedef double Float64;
// typedef unsigned __int128 Natural128;
// typedef __int128 Integer128;
// typedef long double Float128;

const Natural8 architectureSize = sizeof(void*)*8;
typedef conditional<architectureSize == 32, Natural32, Natural64>::type NativeNaturalType;
typedef conditional<architectureSize == 32, Integer32, Integer64>::type NativeIntegerType;
typedef conditional<architectureSize == 32, Float32, Float64>::type NativeFloatType;
typedef NativeNaturalType PageRefType;
typedef NativeNaturalType Symbol;

template<typename DataType>
constexpr static DataType swapedEndian(DataType value);

template<>
constexpr Natural16 swapedEndian(Natural16 value) {
    return __builtin_bswap16(value);
}

template<>
constexpr Natural32 swapedEndian(Natural32 value) {
    return __builtin_bswap32(value);
}

template<>
constexpr Natural64 swapedEndian(Natural64 value) {
    return __builtin_bswap64(value);
}

template<typename DataType>
void swapEndian(DataType& value) {
    value = swapedEndian(value);
}

template<typename T>
constexpr T min(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T, typename... Args>
constexpr T min(T c, Args... args) {
    return min(c, min(args...));
}

template<typename T>
constexpr T max(T a, T b) {
    return (a > b) ? a : b;
}

template<typename T, typename... Args>
constexpr T max(T c, Args... args) {
    return max(c, max(args...));
}

extern "C" {
    NativeNaturalType strlen(const char* str) {
        const char* pos;
        for(pos = str; *pos; ++pos);
        return pos-str;
    }
    NativeNaturalType memcpy(void* dst, const void* src, NativeNaturalType len) {
        for(NativeNaturalType i = 0; i < len; ++i)
            reinterpret_cast<char*>(dst)[i] = reinterpret_cast<const char*>(src)[i];
        return 0;
    }
    NativeNaturalType memset(void* dst, NativeNaturalType value, NativeNaturalType len) {
        for(NativeNaturalType i = 0; i < len; ++i)
            reinterpret_cast<char*>(dst)[i] = value;
        return 0;
    }
    Natural8 memcmp(const void* a, const void* b, NativeNaturalType length) {
        for(NativeNaturalType i = 0; i < length; ++i) {
            Natural8 diff = reinterpret_cast<const Natural8*>(a)[i]-reinterpret_cast<const Natural8*>(b)[i];
            if(diff != 0)
                return diff;
        }
        return 0;
    }
    void __cxa_atexit(void(*)(void*), void*, void*) {}
    void __cxa_pure_virtual() {}
    void __cxa_deleted_virtual() {}
}

inline void* operator new(conditional<architectureSize == 32, unsigned, unsigned long>::type, void* ptr) noexcept {
    return ptr;
}

NativeNaturalType fromPointer(void* ptr) {
    return reinterpret_cast<NativeNaturalType>(ptr);
}

template<typename Type>
Type* toPointer(NativeNaturalType ptr) {
    return reinterpret_cast<Type*>(ptr);
}

void puts(const char* str);
