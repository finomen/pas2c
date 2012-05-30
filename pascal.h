#include <boost/tuple/tuple.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <vector>

typedef int integer;
typedef double real;
typedef std::string string;

template<int A, int B>
class pascal_subrange {};

template<typename T0 = void,
         typename T1 = void, 
         typename T2 = void, 
         typename T3 = void, 
         typename T4 = void, 
         typename T5 = void, 
         typename T6 = void, 
         typename T7 = void, 
         typename T8 = void, 
         typename T9 = void, 
         typename T10 = void>
class pascal_array {
public:
    template <typename T>
    T0 & operator[](T const & a) {}
};

template<typename T0, typename T1>
T0 pow(T0 const & a, T1 const & b) {
    T0 r = a;
    for (T1 i = 1; i < b; ++i) {
        r *= a;
    }
    return r;
}