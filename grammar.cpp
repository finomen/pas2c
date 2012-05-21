#include <boost/config/warning_disable.hpp>

#include <boost/spirit/include/qi.hpp>

#include <boost/spirit/include/support.hpp>


#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

#include "grammar.h"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;

struct pascal_grammar :
    qi::grammar<std::string::const_iterator, file(), ascii::space_type>
{
    pascal_grammar() :
        base_type(file_)
    {
        using qi::lit;
        using qi::lexeme;
        using ascii::char_;
        using ascii::alnum;
        using ascii::no_case;
        using ascii::string;
        using spirit::uint_;
        using spirit::int_;
        using spirit::double_;
        using phoenix::at_c;
        using phoenix::val;
        using qi::omit;
        using namespace qi::labels;
       
        identifier = lexeme[char_("a-zA-Z_") >> *(alnum | char_("_"))];
        char_string = lit("'") >> *~char_("'") >> lit("'");
        cchar_ = lit("\"") >> ~char_("\"") >> lit("\""); //FIXME: "\""
        identifier_list = identifier % lit(',');
        
        relop %= lit("=") [val(EQUAL)]
            | lit("<>") [val(NOTEQUAL)] 
            | lit("<") [val(LT)]
            | lit(">") [val(GT)]
            | lit("<=") [val(LE)]
            | lit(">=") [val(GE)]
            | no_case[lit("IN")] [val(IN)];
            
        addop %= lit("+") [val(PLUS)]
            | lit("-") [val(MINUS)] 
            | no_case[lit("OR")] [val(OR)];
            
        mulop %= lit("*") [val(STAR)]
            | lit("/") [val(SLASH)] 
            | no_case[lit("DIV")] [val(DIV)]
            | no_case[lit("MOD")] [val(MOD)]
            | no_case[lit("AND")] [val(AND)];
            
        sop %= lit("+") [val(PLUS)]
            | lit("-") [val(MINUS)];

        eop %= lit("**") [val(STARSTAR)];
        notop %= no_case[lit("NOT")] [val(NOT)];

        nil_ = no_case[lit("NIL")] [val(nil())];

        label = uint_;
        label_list = label % lit(',');
        label_declaration_part = label_list >> lit(';');
        
        constant_definition = identifier >> lit('=') >>
            cexpression_ >> lit(';');
        constant_list = *constant_definition;
        constant_definition_part = no_case[lit("CONST")] >> constant_list;
        
        cexpression_ = ccomparsion | csimple_expression;
        ccomparsion = csimple_expression >> relop >> csimple_expression;

        csimple_expression = cadd | cterm;
        cadd = cterm >> addop >> csimple_expression;

        cterm = cmul | cfactor;
        cmul = cfactor >> mulop >> cterm;

        cfactor = /*csign | */cexponentation.alias();
        //csign = sop >> cexponentation; //WTF?? why unary_left fails?

        cexponentation = cexp | cprimary;
        cexp = cprimary >> eop >> cexponentation;

        cprimary = identifier
        		| (lit("(") >> cexpression_ [val(_1)] >> lit(")"))
        		| unsigned_constant_
        		/*| cpnot*/;

        //cpnot = notop >> cprimary; //WTF?? why unary_left fails?
        unsigned_constant_ = uint_ | double_ | char_string | nil_; //FIXME: double mus be unsigned and human-readable
        constant_ = int_ | double_ | char_string | identifier | cchar_; //WTF?? identifier here?

        file_ = program_ /*| unit_*/;

        program_ = no_case[lit("PROGRAM")] >> identifier >> -(lit("(") >>
        		identifier_list >> lit(")")) >> lit(";") >> block_[at_c<2>(_val) = _1] >> lit(".");

        block_ = -label_declaration_part [at_c<0>(_val) = _1] >>
        		-constant_definition_part [at_c<1>(_val) = _1]>> type_definition_part
				/*>> variable_declaration_part >> procedure_and_function_declaration_part
				>> statement_part*/;


		type_definition_list = *type_definition;
		type_definition_part = no_case[lit("TYPE")] >> type_definition_list;
		type_definition = identifier >> lit("=") >> type_denoter >> lit(";");
		type_denoter = new_type | identifier;
		new_type = new_ordinal_type | new_structured_type | new_pointer_type;
		new_pointer_type = lit("^") >> identifier;
		new_ordinal_type = subrange_type_ | enumerated_type_;
		enumerated_type_ = lit("(") >> identifier_list >> lit(")");
		subrange_type_ = constant_ >> lit("..") >> constant_;

		new_structured_type = -lit("PACKED") >> structured_type_;
		structured_type_ = array_type /*| record_type | set_type | file_type*/;
		array_type = no_case[lit("ARRAY")] >>  lit("[") >> index_list >> lit("]") >>
			no_case[lit("OF")] >> type_denoter;
		index_list = (new_ordinal_type | identifier) % lit(",");
    }

    typedef std::string::const_iterator iterator;

    qi::rule<iterator, std::string(), ascii::space_type> identifier;
    qi::rule<iterator, std::string(), ascii::space_type> char_string;
    qi::rule<iterator, char(), ascii::space_type> cchar_;
    qi::rule<iterator, std::vector<std::string>(), ascii::space_type> identifier_list;
    
    qi::rule<iterator, operators, ascii::space_type> relop;
    qi::rule<iterator, operators, ascii::space_type> addop;
    qi::rule<iterator, operators, ascii::space_type> mulop;
    qi::rule<iterator, operators, ascii::space_type> sop;
    qi::rule<iterator, operators, ascii::space_type> eop;
    qi::rule<iterator, operators, ascii::space_type> notop;
    
    qi::rule<iterator, size_t(), ascii::space_type> label;
    qi::rule<iterator, std::vector<size_t>(), ascii::space_type> label_list;
    qi::rule<iterator, std::vector<size_t>(), ascii::space_type> label_declaration_part;
    
    qi::rule<iterator, constant(), ascii::space_type> constant_definition;
    qi::rule<iterator, std::vector<constant>(), ascii::space_type> constant_list;
    qi::rule<iterator, std::vector<constant>(), ascii::space_type> constant_definition_part;
    
    qi::rule<iterator, cexpression(), ascii::space_type> cexpression_;
    qi::rule<iterator, cexpression(), ascii::space_type> csimple_expression;
    
    qi::rule<iterator, cexpression(), ascii::space_type> cterm;
    qi::rule<iterator, cexpression(), ascii::space_type> cfactor;
    qi::rule<iterator, cexpression(), ascii::space_type> cexponentation;
    qi::rule<iterator, cexpression(), ascii::space_type> cprimary;
    qi::rule<iterator, unsigned_constant(), ascii::space_type> unsigned_constant_;
    qi::rule<iterator, signed_constant(), ascii::space_type> constant_;

    qi::rule<iterator, nil(), ascii::space_type> nil_;
    
    /// Temp rules
    qi::rule<iterator, cexpression::binary(), ascii::space_type> ccomparsion;
    qi::rule<iterator, cexpression::binary(), ascii::space_type> cadd;
    qi::rule<iterator, cexpression::binary(), ascii::space_type> cmul;
    qi::rule<iterator, cexpression::unary_left(), ascii::space_type> csign;
    qi::rule<iterator, cexpression::binary(), ascii::space_type> cexp;
    qi::rule<iterator, cexpression::unary_left(), ascii::space_type> cpnot;
    
    qi::rule<iterator, file(), ascii::space_type> file_;
    qi::rule<iterator, program(), ascii::space_type> program_;
    
    qi::rule<iterator, block(), ascii::space_type> block_;

    qi::rule<iterator, type_decl(), ascii::space_type> type_definition;
	qi::rule<iterator, std::vector<type_decl>(), ascii::space_type> type_definition_list;
	qi::rule<iterator, std::vector<type_decl>(), ascii::space_type> type_definition_part;
	qi::rule<iterator, type_decl(), ascii::space_type> type_denoter;
	qi::rule<iterator, type(), ascii::space_type> new_type;
	qi::rule<iterator, pointer_type(), ascii::space_type> new_pointer_type;
	qi::rule<iterator, ordinal_type(), ascii::space_type> new_ordinal_type;
	qi::rule<iterator, enumerated_type(), ascii::space_type> enumerated_type_;
	qi::rule<iterator, subrange_type(), ascii::space_type> subrange_type_;
	qi::rule<iterator, structured_type(), ascii::space_type> structured_type_;
	qi::rule<iterator, structured_type(), ascii::space_type> new_structured_type;

	qi::rule<iterator, array(), ascii::space_type> array_type;
	qi::rule<iterator, array::idl_t(), ascii::space_type> index_list;


    
};

#include <fstream>
#include <iostream>

int main() {
	const char * filename = "test.pas";
	std::ifstream in(filename, std::ios_base::in);

	if (!in)
	{
		std::cerr << "Error: Could not open input file: "
			<< filename << std::endl;
		return 1;
	}

	std::string storage;
	in.unsetf(std::ios::skipws);
	std::copy(
		std::istream_iterator<char>(in),
		std::istream_iterator<char>(),
		std::back_inserter(storage));


	pascal_grammar structure_;

	std::vector<file> ast;

	using boost::spirit::ascii::space;
	std::string::const_iterator iter = storage.begin();
	std::string::const_iterator end = storage.end();
	bool r = phrase_parse(iter, end, structure_, space, ast);
	if (r && iter == end) {
		std::cout << "OK" << std::endl;
	} else {
        std::string::const_iterator some = iter+60;
        std::string context(iter, (some>end)?end:some);
        std::cerr << "-------------------------\n";
        std::cerr << "Parsing failed\n";
        std::cerr << "stopped at: \"" << context << "...\"\n";
        std::cerr << "-------------------------\n";
        return 1;
    }

	return 0;
}

