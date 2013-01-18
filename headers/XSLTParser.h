/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XSLTParser.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XSLTPARSER_H_
#define XSLTPARSER_H_

#include "XMLParser.h"
#include "XSLTStructs.h"

//klasa reprezentuje parser arkusza XSLT,
//dziedziczy po parserze xml by wykorzystac kilka istniejacych juz regul
class XSLTParser : IParser {
	//String matchAttribute(const parsingXML::Name& n);
	//String matchAttribute(const String& n);
	XSLSymbol matchXSLKeyword(const Name& n);
	bool validateName(const Node * n, XSLSymbol t);
	String requiredAttribute(const Node * n, XSLSymbol t);
	String optionalAttribute(const Node *n, XSLSymbol t, String def_val);


	// Order -> 'order' '=' '"'( 'ascending' | 'descending')'"'
	OrderVal matchOrderAttr();

	// DataType -> 'data-type' '=' '"'(number | text)'"'
	DataType matchDataTypeAttr();

	// TestE -> 'test' '=' '"' Expression '"'
	XPathExpr * matchCondition();

	// SelectE -> 'select' '=' '"' Expression '"'
	XPathExpr * matchSelectExpression();

	// SelectP -> 'select' '=' '"' Path '"'
	LocExpr * matchSelectNodeSet();

	// 'match' '=' '"' Path '"'
	LocExpr * matchTemplatePattern();

	// 'version' '=' '"' Value '"'
	String matchVersion();

	//ponizsze dwie funkcje wywoluja parser XPath na stringu jako sekwencji zrodlowej
	LocExpr * parseXPath(const String& sequence);
	XPathExpr * parseXPathExpr(const String& sequence);

	//funkcja parsuje atrybut, ale nie alokuje miejsca, wykorzystywana w prologu, ktory nie jest potrzebny do niczego
	std::pair<String, String> Attribute();

	String Comment();
	XSLText * Text(const Node * n);

	// Stylesheet -> '<' STYLESHEET Version { Attribute } '>' { Comment } { Template { Comment} }'</' STYLESHEET '>'
	XSLTStylesheet * Stylesheet(const Node * n);

	// Template -> '<' TEMPLATE (MATCH | NAME) '=' LITERAL ('priority' '=' NUMBER)? '>' Content '</ TEMPLATE '>'
	XSLTTemplate * Template(const Node * n);


	// TLI -> ApplyTemplates | If | Choose | ForEach | ValueOf
	Instruction * TLI(const Node * n);

	// Content -> { Comment | '<' TopLevelInstruction | Text }
	InstructionVec Content(const Node * n);

	//FEC -> { '<' Sort | Comment } Content
	std::pair<SortVec, InstructionVec> ForEachContent(const Node * n);

	//ApplyTemplates -> APPLYTEMPLATES SelectP? ( '/>' | '>' {'<' Sort} '</' APPLYTEMPLATES '>' )
	XSLApplyTemplates * ApplyTemplates(const Node * n);

	//ForEach -> FOREACH SelectP '>' ForEachContent '</' FOREACH '>'
	XSLRepetition * ForEach(const Node * n);

	// Sort -> SORT SelectE? ( Order | Datatype )? '/>'
	XSLSort * Sort(const Node * n);

	// If -> IF TestE '>' Content '</' IF '>'
	XSLConditional * IfClause(const Node * n);

	// Choose -> CHOOSE '>' {'<' When } ('<' Otherwise)? '</' CHOOSE '>'
	XSLBranch * Choose(const Node * n);

	//When -> WHEN TestE '>' Content '</' WHEN '>'
	XSLConditional * When(const Node * n);

	// Otherwise -> OTHERWISE '>' Content '</' OTHERWISE '>'
	InstructionVec Otherwise(const Node * n);

	// ValueOf -> VALUEOF SelectE '/>'
	XSLValueOf * ValueOf(const Node * n);

	//Complex -> Name { Attribute } ( '/>' | '>' Content '</' Name '>')
	XSLComplex * ComplexInstruction(const Node * n);


public:
	XSLTParser(): IParser(NULL) {};
	XSLTParser(ILexer * l);
	virtual ~XSLTParser() {};

	// Document -> Prolog {Comment} Stylesheet
	XSLTStylesheet * Document(const Node * n);
	virtual ParsedObject * startParsing();
};


#endif /* XSLTPARSER_H_ */
