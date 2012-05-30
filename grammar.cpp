#define BOOST_SPIRIT_DEBUG

#include <boost/config/warning_disable.hpp>

#include <boost/spirit/include/qi.hpp>

#include <boost/spirit/include/support.hpp>


#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <locale>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;


struct fargs_impl
{
  template <typename T1, typename T2>
  struct result { typedef std::string type; };

  std::string operator()(std::vector<std::string> const & ids, std::string const & type) const {
		std::string res = "";
		std::string delim = "";

		BOOST_FOREACH(std::string const & s, ids) {
			res += delim + type + " " + s;
			delim = ", ";
		}

		return res;
	}
};

struct counter_impl {
    template<typename T1>
    struct result {typedef std::string type; };

    std::string operator()(std::string const & s) const {
        return s[0] == '<' ? "--" : "++";
    }
};

struct mklower_impl {
    template<typename T1>
    struct result {typedef char type; };

    char operator()(char const & s) const {
        return std::tolower(s);
    }
};

phoenix::function<fargs_impl> fargs;
phoenix::function<counter_impl> counter;
phoenix::function<mklower_impl> mklower;

struct pascal_keywords : boost::spirit::qi::symbols<char, std::string> {
	pascal_keywords() {
		add
		("PROGRAM")
		("VAR")
		("CONST")
		("TYPE")
		("RECORD")
		("PACKED")
		("FOR")
		("WHILE")
		("DO")
		("BEGIN")
		("UNTIL")
		("END")
		("TO")
		("DOWNTO")
		("NOT")
		("AND")
		("OR")
		("XOR")
		("DIV")
		("MOD")
		("IF")
		("THEN")
		("ELSE")
		("CASE")
		("OF")
		("IN")
		("ARRAY")
		("SET")
		("OTHERWISE")
		("FILE")
		("REPEAT")
		//TODO: add others
		;
	}
} keywords_table;

struct pascal_grammar :
    qi::grammar<std::string::const_iterator, std::string(), ascii::space_type>
{
    pascal_grammar() :
        base_type(file_)
    {
        using qi::lit;
        using qi::eps;
        using qi::lexeme;
        using ascii::char_;
        using ascii::alnum;
        using ascii::no_case;
        using ascii::string;
        using phoenix::at_c;
        using phoenix::val;
        using qi::omit;
        using namespace qi::labels;

        squote = char_("'") [_val += "\""];
        dquote = char_("\"") [_val += "'"];
        uint_ = lexeme[char_("1-9") >> *char_("0-9")];
        int_ = -char_("+-") >> lexeme[char_("1-9") >> *char_("0-9")];
        double_ = -int_ [_val += _1] >> lit(".") [_val += "."] >> uint_ [_val += _1];
       
        identifier = (lexeme[char_("a-zA-Z_") [_val += mklower(_1)]>> *((alnum | char_("_")) [_val += mklower(_1)])]) - keywords_table;

        char_string = squote >> *~char_("'") >> squote;
        cchar_ = dquote >> ~char_("\"") >> dquote; //FIXME: "\""

        identifier_list = identifier [_val += _1] % lit(',') [_val += ", "];
        identifier_list_ = identifier % lit(',');

        relop %= lit("=") [_val += "=="]
            | lit("<>") [_val += "!="]
            | lit("<") [_val += "<"]
            | lit(">") [_val += ">"]
            | lit("<=") [_val += "<="]
            | lit(">=") [_val += ">="]
            | no_case[lit("IN")] [_val += "IN"]; //FIXME: IN is special case
            
        addop %= lit("+") [_val += "+"]
            | lit("-") [_val += "-"]
            | no_case[lit("OR")] [_val += "||"];
            
        mulop %= lit("*") [_val += "*"]
            | lit("/") [_val += "/"]
            | no_case[lit("DIV")] [_val += "/"]
            | no_case[lit("MOD")] [_val += "%"]
            | no_case[lit("AND")] [_val += "&&"];
            
        sop %= lit("+") [_val += "+"]
            | lit("-") [_val += "-"];

        eop %= lit("**") [_val += "STARSTAR"]; //FIXME: STARSTAR is pow
        notop %= no_case[lit("NOT")] [_val += "!"];


        nil_ = no_case[lit("NIL")] [val("0")];

        label = char_("[1-9]") >> *char_("[0-9]");
        label_list = label % lit(',');
        label_declaration_part = label_list [_val = ""]>> lit(';'); //TODO: what is this&

        constant_definition = identifier [_val = "#define " + _1] >> lit('=') [_val += " "] >>
            cexpression_ [_val += _1] >> lit(';'); //FIXME: ugly hack because type is unknown
        constant_list = *(constant_definition [_val += _1 + "\n"]);
        constant_definition_part =
        		no_case[lit("CONST")] [_val += "/**\n * Constants\n */\n"]
			    >> constant_list [_val += _1];

        cexpression_ = ccomparsion [_val = _1] | csimple_expression [_val = _1];
        ccomparsion = csimple_expression >> relop >> csimple_expression;

        csimple_expression = cadd [_val = _1] | cterm [_val = _1];
        cadd = cterm [_val += _1] >> addop [_val += " " + _1 + " "] >> csimple_expression [_val += _1];

        cterm = cmul [_val = _1] | cfactor [_val = _1];
        cmul = cfactor [_val += _1] >> mulop [_val += " " + _1 + " "] >> cterm [_val += _1];

        cfactor = csign [_val = _1] | cexponentation [_val = _1];
        csign = sop >> cexponentation;

        cexponentation = cexp [_val = _1] | cprimary [_val = _1];
        cexp = cprimary [_val = "pow(" + _1 + ", "] >> eop
        		>> cexponentation [_val += _1 +")"];

        cprimary = identifier [_val = _1]
        		| (lit("(") >> cexpression_ >> lit(")")) [_val = _1]
        		| unsigned_constant [_val = _1]
        		| cpnot [_val = _1];

        cpnot = notop >> cprimary;

        unsigned_constant = uint_ |
        		double_ |
        		char_string |
        		nil_ ; //FIXME: double must be unsigned and human-readable

        constant = int_ |
        		double_  |
        		char_string |
        		identifier |
        		cchar_ ; //WTF?? identifier here?

        file_ = program_.alias() /*| unit_*/;

        program_ = no_case[lit("PROGRAM")]
			    [_val = "#include \"pascal.h\" \n/**\n * Autogenerated from "]
				>> identifier [_val += _1] >> -(lit("(") >>
        		identifier_list >> lit(")")) >> lit(";") [_val += "\n */\n"] >> block [_val += _1] >> lit(".");

        block = -label_declaration_part [_val += _1] >>
        		-constant_definition_part [_val += _1] >> 
        		-type_definition_part [_val += _1]
				>> -variable_declaration_part [_val += _1]
				>> -procedure_and_function_declaration_part [_val += _1]
				>> statement_part [_val += "void pascal_main() " + _1 + "\n\n int main() {\npascal_main();\nreturn 0;\n}"];


		type_definition_list = *type_definition;
		type_definition_part = no_case[lit("TYPE")] [_val += "/**\n * Types\n */\n"] >> type_definition_list [_val += _1];
		type_definition = (identifier >> lit("=") >> type_denoter ) [_val += "typedef " + _2 + " " + _1]>> lit(";") [_val += ";\n"];
		type_denoter = new_type [_val = _1] | identifier [_val = _1];
		new_type = new_ordinal_type | new_structured_type | new_pointer_type;
		new_pointer_type = lit("^") >> identifier [_val += _1 + "*"];
		new_ordinal_type = subrange_type_ | enumerated_type_;
		enumerated_type_ = lit("(") [_val += "enum {"] >> identifier_list [_val += _1] >> lit(")") [_val += "}"];
		subrange_type_ = constant [_val += "pascal_subrange<" + _1 + ", "] >> lit("..") >> constant [_val += _1 + ">"];

		new_structured_type = -lit("PACKED") >> structured_type_;
		structured_type_ = array_type.alias() /*| record_type | set_type | file_type*/;
		array_type = no_case[lit("ARRAY")] [_val = "pascal_array<"]
			>>  (lit("[") >> index_list >> lit("]") >>
			no_case[lit("OF")] >> type_denoter ) [_val += _2 + ", " + _1 + " >"];
		index_list = (new_ordinal_type [_val += _1] | identifier [_val += _1])  % lit(",") [_val += ", "];

		variable_declaration_part = lit("VAR") [_val += "/**\n * Variables\n */\n"] >> variable_declaration_list [_val += _1] >> lit(";");
		variable_declaration_list = variable_declaration % lit(";");
		variable_declaration =
				(identifier_list >> lit (":") >> type_denoter)
					[_val += _2 + " " + _1 + ";\n"];

		procedure_and_function_declaration_part = proc_or_func_declaration_list
				[_val += "/**\n * Functions\n */\n" + _1] >> lit(";");

		proc_or_func_declaration_list = proc_or_func_declaration [_val += _1 ]% lit(";");
		proc_or_func_declaration = procedure_declaration [_val = _1] | function_declaration [_val = _1];

		procedure_declaration = (procedure_heading >> lit(";") >> directive) [_val = _1 + ";" + _2 + "\n"] |
				(procedure_heading >> lit(";") >> procedure_block) [_val = _1 + " {\n" + _2 + "\n}\n"];

		procedure_heading = procedure_identification [_val = "void " + _1 + "("]
			  >> formal_parameter_list [_val += _1] >> eps [_val += ")"];
		directive = no_case[lit("FORWARD")] [_val = "/* FORWARD */"]
			 | no_case[lit("EXTERNAL")] [_val = "/* EXTERNAL */"];

		formal_parameter_list = -(lit("(") >> formal_parameter_section_list >> lit(")"));
		formal_parameter_section_list = formal_parameter_section [_val += _1] % lit(";") [_val += ", "];
		formal_parameter_section = value_parameter_specification
		 | variable_parameter_specification
		 | procedural_parameter_specification
		 | functional_parameter_specification
		 ;

		value_parameter_specification = (identifier_list_ >> lit(":") >> identifier) [_val = fargs(_1, _2)];
		variable_parameter_specification = (no_case[lit("VAR")] >>
				identifier_list_ >> lit(":") >> identifier) [_val = fargs(_1, _2 + "&")];

		procedural_parameter_specification = procedure_heading.alias(); //TODO:
		functional_parameter_specification = function_heading.alias(); //TODO:

		procedure_identification = no_case[lit("PROCEDURE")] >> identifier;
		procedure_block = /*block.alias()*/ function_block.alias();

		function_declaration = (function_heading >> lit(";") >> directive) [_val = _1 + ";" + _2 + "\n"]
		 | (function_identification >> lit(";") >> function_block) [_val = _1 + "\n" + _2 + "\n"]
		 | (function_heading >> lit(";") >> function_block) [_val = _1 + " {\n" + _2 + "\n}\n"];
		function_heading = (no_case[lit("FUNCTION")] >> identifier
				>> formal_parameter_list >> lit(":") >> result_type) [_val = _3 + " " + _1 + "("+ _2 + ")"];
		result_type = identifier.alias();
		function_identification = no_case[lit("FUNCTION")] >> identifier;
		function_block = /*block.alias();*/
            -constant_definition_part
            >> -variable_declaration_part
            >> statement_part;

		statement_part = compound_statement.alias();
		compound_statement = no_case[lit("BEGIN")] [_val += "{\n"]
				>> statement_sequence [_val += _1]
				>> no_case[lit("END")] [_val += "\n}\n"];
		statement_sequence = (statement [_val += _1] % lit(";") [_val += ";\n"] >> eps [_val += ";\n"]) | eps;

		statement = open_statement [_val = _1] | closed_statement [_val = _1];
		open_statement = /*-(label >> lit(":")) >>*/ non_labeled_open_statement.alias(); //TODO: label
		closed_statement = /*-(label >> lit(":")) >>*/ non_labeled_closed_statement.alias(); //TODO: label

		non_labeled_closed_statement = assignment_statement [_val = _1]
		 | procedure_statement [_val = _1]
		 | goto_statement [_val = _1]
		 | compound_statement [_val = _1]
		 | case_statement [_val = _1]
		 | repeat_statement [_val = _1]
		 | closed_with_statement [_val = _1]
		 | closed_if_statement [_val = _1]
		 | closed_while_statement [_val = _1]
		 | closed_for_statement [_val = _1]
		 ;

		non_labeled_open_statement = open_with_statement [_val = _1]
		 | open_if_statement [_val = _1]
		 | open_while_statement [_val = _1]
		 | open_for_statement [_val = _1]
		 ;

		repeat_statement = no_case[lit("REPEAT")] [_val += "do {\n"]
			   >> statement_sequence [_val += _1]
			   >> no_case[lit("UNTIL")] [_val += "} while ("]
			   >> boolean_expression [_val += _1 + ")"];

		open_while_statement = no_case[lit("WHILE")] [_val += "while ("]
			   >> boolean_expression [_val += _1]
			   >> no_case[lit("DO")] [_val += ") "]
			   >> closed_statement [_val += _1]; //FIXME: open statement

		closed_while_statement = no_case[lit("WHILE")] [_val += "while ("]
               >> boolean_expression [_val += _1]
               >> no_case[lit("DO")] [_val += ") "]
               >> closed_statement [_val += _1]; 

		open_for_statement = (no_case[lit("FOR")]
			   >> control_variable
			   >> lit(":=")
			   >> initial_value
			   >> direction
		       >> final_value
		       >> no_case[lit("DO")] ) 
                    [_val = "for (" + _1 + " = " + _2 + "; " + _1 + " " + _3 + " " + _4 + "; " + counter(_3) + _1 + ")\n"]
		       >> closed_statement [_val += _1]; //FIXME: open statement

		closed_for_statement = (no_case[lit("FOR")]
               >> control_variable
               >> lit(":=")
               >> initial_value
               >> direction
               >> final_value
               >> no_case[lit("DO")] ) 
                    [_val = "for (" + _1 + " = " + _2 + "; " + _1 + " " + _3 + " " + _4 + "; " + counter(_3) + _1 + ")\n"]
               >> closed_statement [_val += _1]; 

		open_with_statement = no_case[lit("WITH")]
			   >> record_variable_list
			   >> no_case[lit("DO")]
			   >> closed_statement; //TODO: unsupported error, open_statement

		closed_with_statement = no_case[lit("WITH")]
			   >> record_variable_list
			   >> no_case[lit("DO")]
			   >> closed_statement; //TODO: unsupported error


		open_if_statement = (no_case[lit("IF")] [_val = "if ("]
			   >> boolean_expression [_val += _1]
			   >> no_case[lit("THEN")] [_val += ") "]
			   >> closed_statement [_val += _1 + ";"]
			   >> no_case[lit("ELSE")] [_val += " else "]
			   >> closed_statement [_val += _1]) //FIXME: open satement?
			   | (no_case[lit("IF")] [_val = "if ("]
			   >> boolean_expression [_val += _1]
			   >> no_case[lit("THEN")] [_val += ") "]
			   >> statement [_val += _1]);

		closed_if_statement = no_case[lit("IF")] [_val += "if ("]
			   >> boolean_expression [_val += _1]
			   >> no_case[lit("THEN")] [_val += ") "]
			   >> closed_statement [_val += _1 + ";"]
			   >> no_case[lit("ELSE")] [_val += " else "]
			   >> closed_statement [_val += _1];

		assignment_statement = variable_access [_val += _1]
				>> lit(":=") [_val += " = "]
				>> expression [_val += _1];

		variable_access = indexed_variable [_val = _1]
		 /* | field_designator [_val = _1] */ //FIXME: record support
		 /* | (variable_access >> lit("^")) [_val = "*" + _1]*/ 
         | (identifier >> lit("^")) [_val = "*" + _1]
		 | identifier [_val = _1];
         
        variable_access_ = /* field_designator_ [_val = _1]
         | (variable_access_ >> lit("^")) [_val = "*" + _1] */ //FIXME: record support
         (identifier >> lit("^")) [_val = "*" + _1]
         | identifier [_val = _1]; 

		indexed_variable = variable_access_ [_val += _1]
				>> +(lit("[") [_val += "[boost::make_tuple("]
				>> index_expression_list [_val += _1]
				>> lit("]") [_val += ")]"]);

		index_expression_list = index_expression % lit(",");

		index_expression = expression.alias();

		field_designator = variable_access [_val += _1] >> lit(".") [_val += "."]
				>> identifier [_val += _1];
                
        field_designator_ = variable_access_ [_val += _1] >> lit(".") [_val += "."]
                >> identifier [_val += _1];

		procedure_statement = identifier [_val = _1 + "("] >> -params [_val += _1] >> eps [_val += ")"];

		params = lit("(") >> actual_parameter_list >> lit(")");

		actual_parameter_list = actual_parameter % lit(",");

		actual_parameter = expression [_val = _1]
		 | (expression >> lit(":") >> expression)
		 	 [_val = "boost::make_tuple(" + _1 + ", " + _2 + ")"]
		 | (expression >> lit(":") >> expression >> lit(":") >> expression)
		 	 [_val = "boost::make_tuple(" + _1 + ", " + _2 + ", " + _3 + ")"];

		goto_statement = no_case[lit("GOTO")] >> label;

		case_statement = no_case[lit("CASE")]
				>> case_index
				>> no_case[lit("OF")]
				>> case_list_element_list
				>> -(lit(";")
				>> otherwisepart
				>> statement)
				>> -lit(";")
				>> no_case[lit("END")];

		case_index = expression.alias();
		case_list_element_list = case_list_element % lit(";");
		case_list_element = case_constant_list >> lit(":") >> statement;
		otherwisepart = no_case[lit("OTHERWISE")] >> -lit(":");
		control_variable = identifier.alias();
		initial_value = expression.alias();
		direction = no_case[lit("TO")] [_val = "<="] | no_case[lit("DOWNTO")] [_val = ">="];
		final_value = expression.alias();

		record_variable_list = variable_access % lit(",");
		boolean_expression = expression.alias();

		expression = (simple_expression [_val = _1] >> relop [_val += " " + _1 + " "] >> simple_expression [_val += _1])
				| simple_expression [_val = _1];

		simple_expression = (term [_val = _1] >> addop [_val += " " + _1 + " "] >> simple_expression [_val += _1])
				| term [_val = _1];

		term = (factor [_val = _1] >> mulop [_val += " " + _1 + " "] >> term [_val += _1]) | factor [_val = _1];

		factor = (sop [_val += _1] >> factor [_val += _1]) | exponentiation [_val = _1];

		exponentiation = (primary >> eop >> exponentiation) [_val = "pow(" + _1 + ", " + _3 + ")"]
				| primary [_val = _1];

		primary = (notop >> primary) [_val = _1 + _2]
		 | variable_access [_val = _1]
		 | unsigned_constant [_val = _1]
		 | function_designator [_val = _1]
		 | set_constructor [_val = _1] //FIXME: crash
		 | (lit("(") >> expression >> lit(")")) [_val = "(" + _1 + ")"]; //FIXME: crash

		/* functions with no params will be handled by plain identifier */
		function_designator = identifier >> -params;

		set_constructor = lit("[") >> -member_designator_list >> lit("]");

		member_designator_list = member_designator % lit(",");

		member_designator =	expression % lit("..");

		case_constant_list = case_constant % lit(",");

		case_constant = constant >> lit("..") >> constant | constant;

		//BOOST_SPIRIT_DEBUG_NODE(squote);
		//BOOST_SPIRIT_DEBUG_NODE(dquote);
		//BOOST_SPIRIT_DEBUG_NODE(uint_);
		//BOOST_SPIRIT_DEBUG_NODE(int_);
		//BOOST_SPIRIT_DEBUG_NODE(double_);
		//BOOST_SPIRIT_DEBUG_NODE(identifier);
		//BOOST_SPIRIT_DEBUG_NODE(char_string);
		//BOOST_SPIRIT_DEBUG_NODE(cchar_);
		//BOOST_SPIRIT_DEBUG_NODE(identifier_list);
		//BOOST_SPIRIT_DEBUG_NODE(identifier_list_);
		//BOOST_SPIRIT_DEBUG_NODE(relop);
		//BOOST_SPIRIT_DEBUG_NODE(addop);
		//BOOST_SPIRIT_DEBUG_NODE(mulop);
		//BOOST_SPIRIT_DEBUG_NODE(sop);
		//BOOST_SPIRIT_DEBUG_NODE(eop);
		//BOOST_SPIRIT_DEBUG_NODE(notop);
		//BOOST_SPIRIT_DEBUG_NODE(label);
		//BOOST_SPIRIT_DEBUG_NODE(label_list);
		//BOOST_SPIRIT_DEBUG_NODE(label_declaration_part);
		//BOOST_SPIRIT_DEBUG_NODE(constant_definition);
		//BOOST_SPIRIT_DEBUG_NODE(constant_list);
		//BOOST_SPIRIT_DEBUG_NODE(constant_definition_part);
		//BOOST_SPIRIT_DEBUG_NODE(cexpression_);
		//BOOST_SPIRIT_DEBUG_NODE(csimple_expression);
		//BOOST_SPIRIT_DEBUG_NODE(cterm);
		//BOOST_SPIRIT_DEBUG_NODE(cfactor);
		//BOOST_SPIRIT_DEBUG_NODE(cexponentation);
		//BOOST_SPIRIT_DEBUG_NODE(cprimary);
		//BOOST_SPIRIT_DEBUG_NODE(unsigned_constant);
		//BOOST_SPIRIT_DEBUG_NODE(constant);
		//BOOST_SPIRIT_DEBUG_NODE(nil_);
		//BOOST_SPIRIT_DEBUG_NODE(ccomparsion);
		//BOOST_SPIRIT_DEBUG_NODE(cadd);
		//BOOST_SPIRIT_DEBUG_NODE(cmul);
		//BOOST_SPIRIT_DEBUG_NODE(csign);
		//BOOST_SPIRIT_DEBUG_NODE(cexp);
		//BOOST_SPIRIT_DEBUG_NODE(cpnot);
		//BOOST_SPIRIT_DEBUG_NODE(file_);
		//BOOST_SPIRIT_DEBUG_NODE(program_);
		BOOST_SPIRIT_DEBUG_NODE(block);
		//BOOST_SPIRIT_DEBUG_NODE(type_definition);
		//BOOST_SPIRIT_DEBUG_NODE(type_definition_list);
		//BOOST_SPIRIT_DEBUG_NODE(type_definition_part);
		//BOOST_SPIRIT_DEBUG_NODE(type_denoter);
		//BOOST_SPIRIT_DEBUG_NODE(new_type);
		//BOOST_SPIRIT_DEBUG_NODE(new_pointer_type);
		//BOOST_SPIRIT_DEBUG_NODE(new_ordinal_type);
		//BOOST_SPIRIT_DEBUG_NODE(enumerated_type_);
		//BOOST_SPIRIT_DEBUG_NODE(subrange_type_);
		//BOOST_SPIRIT_DEBUG_NODE(structured_type_);
		//BOOST_SPIRIT_DEBUG_NODE(new_structured_type);
		//BOOST_SPIRIT_DEBUG_NODE(array_type);
		//BOOST_SPIRIT_DEBUG_NODE(index_list);
		//BOOST_SPIRIT_DEBUG_NODE(variable_declaration_part);
		//BOOST_SPIRIT_DEBUG_NODE(variable_declaration_list);
		//BOOST_SPIRIT_DEBUG_NODE(variable_declaration);
		//BOOST_SPIRIT_DEBUG_NODE(procedure_and_function_declaration_part);
		//BOOST_SPIRIT_DEBUG_NODE(proc_or_func_declaration_list);
		//BOOST_SPIRIT_DEBUG_NODE(proc_or_func_declaration);
		//BOOST_SPIRIT_DEBUG_NODE(procedure_declaration);
		//BOOST_SPIRIT_DEBUG_NODE(procedure_heading);
		//BOOST_SPIRIT_DEBUG_NODE(directive);
		//BOOST_SPIRIT_DEBUG_NODE(formal_parameter_list);
		//BOOST_SPIRIT_DEBUG_NODE(formal_parameter_section_list);
		//BOOST_SPIRIT_DEBUG_NODE(formal_parameter_section);
		BOOST_SPIRIT_DEBUG_NODE(value_parameter_specification);
		BOOST_SPIRIT_DEBUG_NODE(variable_parameter_specification);
		BOOST_SPIRIT_DEBUG_NODE(procedural_parameter_specification);
		BOOST_SPIRIT_DEBUG_NODE(functional_parameter_specification);
		BOOST_SPIRIT_DEBUG_NODE(procedure_identification);
		BOOST_SPIRIT_DEBUG_NODE(procedure_block);
		BOOST_SPIRIT_DEBUG_NODE(function_declaration);
		BOOST_SPIRIT_DEBUG_NODE(function_heading);
		BOOST_SPIRIT_DEBUG_NODE(result_type);
		BOOST_SPIRIT_DEBUG_NODE(function_identification);
		BOOST_SPIRIT_DEBUG_NODE(function_block);
		BOOST_SPIRIT_DEBUG_NODE(statement_part);
		BOOST_SPIRIT_DEBUG_NODE(compound_statement);
		BOOST_SPIRIT_DEBUG_NODE(statement_sequence);
		BOOST_SPIRIT_DEBUG_NODE(statement);
		BOOST_SPIRIT_DEBUG_NODE(open_statement);
		BOOST_SPIRIT_DEBUG_NODE(closed_statement);
		BOOST_SPIRIT_DEBUG_NODE(non_labeled_open_statement);
		BOOST_SPIRIT_DEBUG_NODE(non_labeled_closed_statement);
		BOOST_SPIRIT_DEBUG_NODE(repeat_statement);
		BOOST_SPIRIT_DEBUG_NODE(open_while_statement);
		BOOST_SPIRIT_DEBUG_NODE(closed_while_statement);
		BOOST_SPIRIT_DEBUG_NODE(open_for_statement);
		BOOST_SPIRIT_DEBUG_NODE(closed_for_statement);
		BOOST_SPIRIT_DEBUG_NODE(open_with_statement);
		BOOST_SPIRIT_DEBUG_NODE(closed_with_statement);
		BOOST_SPIRIT_DEBUG_NODE(open_if_statement);
		BOOST_SPIRIT_DEBUG_NODE(closed_if_statement);
		BOOST_SPIRIT_DEBUG_NODE(assignment_statement);
		BOOST_SPIRIT_DEBUG_NODE(variable_access);
        BOOST_SPIRIT_DEBUG_NODE(variable_access_);
		BOOST_SPIRIT_DEBUG_NODE(indexed_variable);
		BOOST_SPIRIT_DEBUG_NODE(index_expression_list);
		BOOST_SPIRIT_DEBUG_NODE(index_expression);
		BOOST_SPIRIT_DEBUG_NODE(field_designator);
        BOOST_SPIRIT_DEBUG_NODE(field_designator_);
		BOOST_SPIRIT_DEBUG_NODE(procedure_statement);
		BOOST_SPIRIT_DEBUG_NODE(params);
		BOOST_SPIRIT_DEBUG_NODE(actual_parameter_list);
		BOOST_SPIRIT_DEBUG_NODE(actual_parameter);
		BOOST_SPIRIT_DEBUG_NODE(goto_statement);
		BOOST_SPIRIT_DEBUG_NODE(case_statement);
		BOOST_SPIRIT_DEBUG_NODE(case_index);
		BOOST_SPIRIT_DEBUG_NODE(case_list_element_list);
		BOOST_SPIRIT_DEBUG_NODE(case_list_element);
		BOOST_SPIRIT_DEBUG_NODE(otherwisepart);
		BOOST_SPIRIT_DEBUG_NODE(control_variable);
		BOOST_SPIRIT_DEBUG_NODE(initial_value);
		BOOST_SPIRIT_DEBUG_NODE(direction);
		BOOST_SPIRIT_DEBUG_NODE(final_value);
		BOOST_SPIRIT_DEBUG_NODE(record_variable_list);
		BOOST_SPIRIT_DEBUG_NODE(boolean_expression);
		BOOST_SPIRIT_DEBUG_NODE(expression);
		BOOST_SPIRIT_DEBUG_NODE(simple_expression);
		BOOST_SPIRIT_DEBUG_NODE(term);
		BOOST_SPIRIT_DEBUG_NODE(factor);
		BOOST_SPIRIT_DEBUG_NODE(exponentiation);
		BOOST_SPIRIT_DEBUG_NODE(primary);
		BOOST_SPIRIT_DEBUG_NODE(function_designator);
		BOOST_SPIRIT_DEBUG_NODE(set_constructor);
		BOOST_SPIRIT_DEBUG_NODE(member_designator_list);
		BOOST_SPIRIT_DEBUG_NODE(member_designator);
		BOOST_SPIRIT_DEBUG_NODE(case_constant_list);
		BOOST_SPIRIT_DEBUG_NODE(case_constant);

    }

	typedef std::string::const_iterator iterator;

	// Ugly hack
	qi::rule<iterator, std::string(), ascii::space_type> squote;
	qi::rule<iterator, std::string(), ascii::space_type> dquote;
	qi::rule<iterator, std::string(), ascii::space_type> uint_;
	qi::rule<iterator, std::string(), ascii::space_type> int_;
	qi::rule<iterator, std::string(), ascii::space_type> double_;

	qi::rule<iterator, std::string(), ascii::space_type> identifier;
	qi::rule<iterator, std::string(), ascii::space_type> char_string;
	qi::rule<iterator, std::string(), ascii::space_type> cchar_;
	qi::rule<iterator, std::string(), ascii::space_type> identifier_list;
	qi::rule<iterator, std::vector<std::string>(), ascii::space_type> identifier_list_;

	qi::rule<iterator, std::string(), ascii::space_type> relop;
	qi::rule<iterator, std::string(), ascii::space_type> addop;
	qi::rule<iterator, std::string(), ascii::space_type> mulop;
	qi::rule<iterator, std::string(), ascii::space_type> sop;
	qi::rule<iterator, std::string(), ascii::space_type> eop;
	qi::rule<iterator, std::string(), ascii::space_type> notop;

	qi::rule<iterator, std::string(), ascii::space_type> label;
	qi::rule<iterator, std::string(), ascii::space_type> label_list;
	qi::rule<iterator, std::string(), ascii::space_type> label_declaration_part;

	qi::rule<iterator, std::string(), ascii::space_type> constant_definition;
	qi::rule<iterator, std::string(), ascii::space_type> constant_list;
	qi::rule<iterator, std::string(), ascii::space_type> constant_definition_part;

	qi::rule<iterator, std::string(), ascii::space_type> cexpression_;
	qi::rule<iterator, std::string(), ascii::space_type> csimple_expression;

	qi::rule<iterator, std::string(), ascii::space_type> cterm;
	qi::rule<iterator, std::string(), ascii::space_type> cfactor;
	qi::rule<iterator, std::string(), ascii::space_type> cexponentation;
	qi::rule<iterator, std::string(), ascii::space_type> cprimary;
	qi::rule<iterator, std::string(), ascii::space_type> unsigned_constant;
	qi::rule<iterator, std::string(), ascii::space_type> constant;

	qi::rule<iterator, std::string(), ascii::space_type> nil_;

	/// Temp rules
	qi::rule<iterator, std::string(), ascii::space_type> ccomparsion;
	qi::rule<iterator, std::string(), ascii::space_type> cadd;
	qi::rule<iterator, std::string(), ascii::space_type> cmul;
	qi::rule<iterator, std::string(), ascii::space_type> csign;
	qi::rule<iterator, std::string(), ascii::space_type> cexp;
	qi::rule<iterator, std::string(), ascii::space_type> cpnot;

	qi::rule<iterator, std::string(), ascii::space_type> file_;
	qi::rule<iterator, std::string(), ascii::space_type> program_;

	qi::rule<iterator, std::string(), ascii::space_type> block;

	qi::rule<iterator, std::string(), ascii::space_type> type_definition;
	qi::rule<iterator, std::string(), ascii::space_type> type_definition_list;
	qi::rule<iterator, std::string(), ascii::space_type> type_definition_part;
	qi::rule<iterator, std::string(), ascii::space_type> type_denoter;
	qi::rule<iterator, std::string(), ascii::space_type> new_type;
	qi::rule<iterator, std::string(), ascii::space_type> new_pointer_type;
	qi::rule<iterator, std::string(), ascii::space_type> new_ordinal_type;
	qi::rule<iterator, std::string(), ascii::space_type> enumerated_type_;
	qi::rule<iterator, std::string(), ascii::space_type> subrange_type_;
	qi::rule<iterator, std::string(), ascii::space_type> structured_type_;
	qi::rule<iterator, std::string(), ascii::space_type> new_structured_type;

	qi::rule<iterator, std::string(), ascii::space_type> array_type;
	qi::rule<iterator, std::string(), ascii::space_type> index_list;

	qi::rule<iterator, std::string(), ascii::space_type> variable_declaration_part;
	qi::rule<iterator, std::string(), ascii::space_type> variable_declaration_list;
	qi::rule<iterator, std::string(), ascii::space_type> variable_declaration;
	qi::rule<iterator, std::string(), ascii::space_type> procedure_and_function_declaration_part;
	qi::rule<iterator, std::string(), ascii::space_type> proc_or_func_declaration_list;
	qi::rule<iterator, std::string(), ascii::space_type> proc_or_func_declaration;
	qi::rule<iterator, std::string(), ascii::space_type> procedure_declaration;
	qi::rule<iterator, std::string(), ascii::space_type> procedure_heading;
	qi::rule<iterator, std::string(), ascii::space_type> directive;
	qi::rule<iterator, std::string(), ascii::space_type> formal_parameter_list;
	qi::rule<iterator, std::string(), ascii::space_type> formal_parameter_section_list;
	qi::rule<iterator, std::string(), ascii::space_type> formal_parameter_section;

	qi::rule<iterator, std::string(), ascii::space_type> value_parameter_specification;
	qi::rule<iterator, std::string(), ascii::space_type> variable_parameter_specification;
	qi::rule<iterator, std::string(), ascii::space_type> procedural_parameter_specification;
	qi::rule<iterator, std::string(), ascii::space_type> functional_parameter_specification;
	qi::rule<iterator, std::string(), ascii::space_type> procedure_identification;
	qi::rule<iterator, std::string(), ascii::space_type> procedure_block;
	qi::rule<iterator, std::string(), ascii::space_type> function_declaration;
	qi::rule<iterator, std::string(), ascii::space_type> function_heading;
	qi::rule<iterator, std::string(), ascii::space_type> result_type;
	qi::rule<iterator, std::string(), ascii::space_type> function_identification;
	qi::rule<iterator, std::string(), ascii::space_type> function_block;

	qi::rule<iterator, std::string(), ascii::space_type> statement_part;
	qi::rule<iterator, std::string(), ascii::space_type> compound_statement;
	qi::rule<iterator, std::string(), ascii::space_type> statement_sequence;

	qi::rule<iterator, std::string(), ascii::space_type> statement;
	qi::rule<iterator, std::string(), ascii::space_type> open_statement;
	qi::rule<iterator, std::string(), ascii::space_type> closed_statement;
	qi::rule<iterator, std::string(), ascii::space_type> non_labeled_open_statement;
	qi::rule<iterator, std::string(), ascii::space_type> non_labeled_closed_statement;

	qi::rule<iterator, std::string(), ascii::space_type> repeat_statement;
	qi::rule<iterator, std::string(), ascii::space_type> open_while_statement;
	qi::rule<iterator, std::string(), ascii::space_type> closed_while_statement;
	qi::rule<iterator, std::string(), ascii::space_type> open_for_statement;
	qi::rule<iterator, std::string(), ascii::space_type> closed_for_statement;

	qi::rule<iterator, std::string(), ascii::space_type> open_with_statement;
	qi::rule<iterator, std::string(), ascii::space_type> closed_with_statement;
	qi::rule<iterator, std::string(), ascii::space_type> open_if_statement;
	qi::rule<iterator, std::string(), ascii::space_type> closed_if_statement;

	qi::rule<iterator, std::string(), ascii::space_type> assignment_statement;
	qi::rule<iterator, std::string(), ascii::space_type> variable_access;
    qi::rule<iterator, std::string(), ascii::space_type> variable_access_;
	qi::rule<iterator, std::string(), ascii::space_type> indexed_variable;
	qi::rule<iterator, std::string(), ascii::space_type> index_expression_list;
	qi::rule<iterator, std::string(), ascii::space_type> index_expression;
	qi::rule<iterator, std::string(), ascii::space_type> field_designator;
    qi::rule<iterator, std::string(), ascii::space_type> field_designator_;
	qi::rule<iterator, std::string(), ascii::space_type> procedure_statement;
	qi::rule<iterator, std::string(), ascii::space_type> params;
	qi::rule<iterator, std::string(), ascii::space_type> actual_parameter_list;

	qi::rule<iterator, std::string(), ascii::space_type> actual_parameter;
	qi::rule<iterator, std::string(), ascii::space_type> goto_statement;

	qi::rule<iterator, std::string(), ascii::space_type> case_statement;
	qi::rule<iterator, std::string(), ascii::space_type> case_index;
	qi::rule<iterator, std::string(), ascii::space_type> case_list_element_list;
	qi::rule<iterator, std::string(), ascii::space_type> case_list_element;
	qi::rule<iterator, std::string(), ascii::space_type> otherwisepart;
	qi::rule<iterator, std::string(), ascii::space_type> control_variable;
	qi::rule<iterator, std::string(), ascii::space_type> initial_value;
	qi::rule<iterator, std::string(), ascii::space_type> direction;
	qi::rule<iterator, std::string(), ascii::space_type> final_value;

	qi::rule<iterator, std::string(), ascii::space_type> record_variable_list;
	qi::rule<iterator, std::string(), ascii::space_type> boolean_expression;
	qi::rule<iterator, std::string(), ascii::space_type> expression;
	qi::rule<iterator, std::string(), ascii::space_type> simple_expression;
	qi::rule<iterator, std::string(), ascii::space_type> term;
	qi::rule<iterator, std::string(), ascii::space_type> factor;
	qi::rule<iterator, std::string(), ascii::space_type> exponentiation;

	qi::rule<iterator, std::string(), ascii::space_type> primary;
	qi::rule<iterator, std::string(), ascii::space_type> function_designator;
	qi::rule<iterator, std::string(), ascii::space_type> set_constructor;
	qi::rule<iterator, std::string(), ascii::space_type> member_designator_list;
	qi::rule<iterator, std::string(), ascii::space_type> member_designator;

	qi::rule<iterator, std::string(), ascii::space_type> case_constant_list;
	qi::rule<iterator, std::string(), ascii::space_type> case_constant;
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

	std::string ast;

	using boost::spirit::ascii::space;
	std::string::const_iterator iter = storage.begin();
	std::string::const_iterator end = storage.end();
	bool r = phrase_parse(iter, end, structure_, space, ast);
	if (r && iter == end) {
		std::cout << ast << std::endl;
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

