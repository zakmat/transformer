/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XPathLexer.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XPATHLEXER_H_
#define XPATHLEXER_H_

#include "ILexer.h"

namespace parsingXPath {

//atomy rozpoznawalne przez analizator leksykalny wyrazen XPath
enum XPathSymbol {
	SLASH, ALL, ASTERISK, ATTR, PARENT, CURRENT, NODE, NUM, LITERAL,
	NODEOR, NOT, OR, AND, EQ, NEQ, GT, LT, GTE, LTE, PLUS, MINUS, LBRACK, RBRACK, WS, OTHER, MAXSYM
};

class XPathLexer: public ILexer {
	Token recognizeDot();
	Token recognizeEscapedSequence();
	Token recognizeNumber(Char c);
	Token recognizeSlash();
	Token recognizeLiteral();

public:
	XPathLexer(ISource * src): ILexer(src){};
	Token matchNextToken();
	int getErrSymbol() {return MAXSYM;}

};

}

#endif /* XPATHLEXER_H_ */
