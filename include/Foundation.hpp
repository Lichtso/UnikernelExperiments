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

static_assert(__LITTLE_ENDIAN__);

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
    typedef Natural32 NativeNaturalType;
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

inline void* operator new(unsigned int, void* ptr) noexcept {
    return ptr;
}

void puts(const char* str);
