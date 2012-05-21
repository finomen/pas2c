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

struct nil {};

typedef boost::variant<unsigned int, double, std::string, nil> unsigned_constant;
typedef boost::variant<int, double, std::string, char> signed_constant;

struct cexpression {
    typedef boost::tuple<cexpression, operators, cexpression> binary;
    typedef boost::tuple<operators, cexpression> unary_left;
    typedef boost::tuple<cexpression, operators> unary_right;

    cexpression() : value(std::string()){}
    cexpression(binary const & v) : value(v) {}
    cexpression(unary_left const & v) : value(v) {}
    cexpression(unary_right const & v) : value(v) {}
    cexpression(unsigned_constant const & v) : value(v) {}
    cexpression(std::string const & v) : value(v) {}
    cexpression(cexpression const & v) : value(v.value) {}
    
    typedef boost::variant<
		boost::recursive_wrapper<binary>,
        boost::recursive_wrapper<unary_left>,
        boost::recursive_wrapper<unary_right>,
        unsigned_constant,
        std::string> value_type;
    value_type value;    
};


BOOST_FUSION_ADAPT_STRUCT(
    cexpression,
    (cexpression::value_type, value)
)



struct constant {
    std::string name;
    cexpression value;
};

BOOST_FUSION_ADAPT_STRUCT(
    constant,
    (std::string, name)
    (cexpression, value)
)

struct pointer_type {
	std::string domain_type;
};

BOOST_FUSION_ADAPT_STRUCT(
	pointer_type,
	(std::string, domain_type)
)

struct subrange_type {
	signed_constant from;
	signed_constant to;
};

BOOST_FUSION_ADAPT_STRUCT(
	subrange_type,
	(signed_constant, from)
	(signed_constant, to)
)

struct enumerated_type {
	std::vector<std::string> values;
};

BOOST_FUSION_ADAPT_STRUCT(
	enumerated_type,
	(std::vector<std::string>, values)
)

typedef boost::variant<enumerated_type, subrange_type> ordinal_type;
struct array;
typedef boost::variant<boost::recursive_wrapper<array> > structured_type;
typedef boost::variant<pointer_type, ordinal_type, structured_type> type;
typedef boost::variant<std::string, type> type_decl;

struct array {
	typedef std::vector<boost::variant<ordinal_type, std::string> > idl_t;
	idl_t index_list;
	type_decl component_type;
};

BOOST_FUSION_ADAPT_STRUCT(
	::array,
	(::array::idl_t, index_list)
	(type_decl, component_type)
)

struct block {
	std::vector<size_t> labels;
	std::vector<constant> constants;
	//TODO: type
	//TODO: var
	//TODO: procedure/function
	//TODO: statement
};

BOOST_FUSION_ADAPT_STRUCT(
	block,
	(std::vector<size_t>, labels)
	(std::vector<constant>, constants)
)

struct program {
    std::string name;
    std::vector<std::string> descr;
    block block_;

    program() {}
    program(boost::tuple<std::string, std::vector<std::string>, block> const & t) {
    	name = t.get<0>();
    	descr = t.get<1>();
    	block_ = t.get<2>();
    }
};

BOOST_FUSION_ADAPT_STRUCT(
    program,
    (std::string, name)
    (std::vector<std::string>, descr)
    (block, block_)
)


typedef boost::variant<program> file;


#endif
