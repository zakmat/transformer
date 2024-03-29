/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XMLParser.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include "IParser.h"
#include "XMLLexer.h"
#include "XMLStructs.h"



extern const char * xmlTokDesc[MAXSYMXML];

//klasa parsera dokumentow XML, metody prywatne i chronione to poszczegolne reguly
class XMLParser: public IParser {
	//Document -> Prolog { Comment } Element
	Node * Document();

	// Element -> '<' Name { Attribute } ( '/>' | ('>' Content '</' Name '>'))
	ElementNode * Element();

	// Content -> { Element | Comment | Plain } ^'</'
	NodeVec Content();

	//Plain -> ^{ '<' | '</' }
	TextNode * Plain();
	String CData();

	CommentNode * Comment();
	String Literal();
protected:

	// { Comment }
	void optionalComment();

	//Attribute -> Name '=' Literal
	AttributeNode * Attribute();

	//Prolog -> '<?' Name { Attribute } '?>'
	NodeVec Prolog();

	String SimpleName();
	Name ExpName();
public:
	XMLParser(ILexer * l);
	virtual ~XMLParser() {};

	virtual ParsedObject * startParsing();
};


#endif /* XMLPARSER_H_ */
