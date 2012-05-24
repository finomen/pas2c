#ifndef _H_GRAMMAR_
#define _H_GRAMMAR_

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>


/**
 * Operators
 */
enum operators {
NOOP,    
BRACES,

//enum addop {
PLUS,
MINUS,
OR,
//};

//enum mulop {
STAR,
SLASH,
DIV,
MOD,
AND,
//};

//enum relop {
EQUAL,
NOTEQUAL,
LT,
GT,
LE,
GE,
IN,

//enum OTHER {
STARSTAR,
NOT,
//};
};



#endif
