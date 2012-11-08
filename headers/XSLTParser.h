/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XSLTParser.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XSLTPARSER_H_
#define XSLTPARSER_H_

#include "XMLParser.h"
#include "XSLTLexer.h"
#include "XSLTStructs.h"

namespace parsingXSLT {

//klasa reprezentuje parser arkusza XSLT,
//dziedziczy po parserze xml by wykorzystac kilka istniejacych juz regul
class XSLTParser: public parsingXML::XMLParser {

	//funkcje pomocnicze zgodnie z zasada DRY pozwalaja uniknac powtorzen w kodzie

	// '</' KEYWORD '>'
	void closeElement(XSLTSymbol s);

	// 'Name' '=' '"' Value '"'
	String matchNamedAttr(const String & Name, TokSymbol t);

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
	XSLText * Text();

	// Stylesheet -> '<' STYLESHEET Version { Attribute } '>' { Comment } { Template { Comment} }'</' STYLESHEET '>'
	XSLTStylesheet * Stylesheet();

	// Template -> '<' TEMPLATE (MATCH | NAME) '=' LITERAL ('priority' '=' NUMBER)? '>' Content '</ TEMPLATE '>'
	XSLTTemplate * Template();


	// TLI -> ApplyTemplates | If | Choose | ForEach | ValueOf
	Instruction * TopLevelInstruction();

	// Content -> { Comment | '<' TopLevelInstruction | Text }
	InstructionVec Content();

	//FEC -> { '<' Sort | Comment } Content
	std::pair<SortVec, InstructionVec> ForEachContent();

	//ApplyTemplates -> APPLYTEMPLATES SelectP? ( '/>' | '>' {'<' Sort} '</' APPLYTEMPLATES '>' )
	XSLApplyTemplates * ApplyTemplates();

	//ForEach -> FOREACH SelectP '>' ForEachContent '</' FOREACH '>'
	XSLRepetition * ForEach();

	// Sort -> SORT SelectE? ( Order | Datatype )? '/>'
	XSLSort * Sort();

	// If -> IF TestE '>' Content '</' IF '>'
	XSLConditional * IfClause();

	// Choose -> CHOOSE '>' {'<' When } ('<' Otherwise)? '</' CHOOSE '>'
	XSLBranch * Choose();

	//When -> WHEN TestE '>' Content '</' WHEN '>'
	XSLConditional * When();

	// Otherwise -> OTHERWISE '>' Content '</' OTHERWISE '>'
	InstructionVec Otherwise();

	// ValueOf -> VALUEOF SelectE '/>'
	XSLValueOf * ValueOf();

	//Complex -> Name { Attribute } ( '/>' | '>' Content '</' Name '>')
	XSLComplex * ComplexInstruction();

	// Document -> Prolog {Comment} Stylesheet
	XSLTStylesheet * Document();

public:
	XSLTParser(ILexer * l);
	virtual ~XSLTParser() {};

	virtual ParsedObject * startParsing();
};

}

#endif /* XSLTPARSER_H_ */
