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

void memcpy(void* dst, void* src, Natural32 length) {
    for(Natural32 i = 0; i < length; ++i)
        reinterpret_cast<Natural8*>(dst)[i] = reinterpret_cast<Natural8*>(src)[i];
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

void puts(const char* str);
