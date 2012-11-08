/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XPathParser.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XPATHPARSER_H_
#define XPATHPARSER_H_

#include <stack>

#include "IParser.h"
#include "XPathLexer.h"
#include "XPathStructs.h"

namespace parsingXPath {

//parser wyrazen XPath, klasa buduje drzewo wyrazenia w oparciu o hierarchie operatorow,
//oprocz reguly startowej publiczna jest rowniez metoda Path, wywolywana gdy wiemy ze wyrazenie jest sciezka
class XPathParser: public IParser {
	bool isStartOfPathExpr(XPathSymbol s) const;
	bool isStartOfNode(XPathSymbol s) const;

	//ponizsze funkcje sa wykorzystywane przy obliczaniu precedensu operatorow
	bool hasLowerPrecedence(const XPathSymbol op1, const XPathSymbol op2) const;

	bool isEqOp(const XPathSymbol op) const;
	bool isRelOp(const XPathSymbol op) const;
	bool isAddOp(const XPathSymbol op) const;
	bool isMulOp(const XPathSymbol op) const;
	// Op -> '*' | '+' | '-' | '=' | '!=' | '<' | '<=' | '>' | '>='
	bool isOperator(XPathSymbol s) const;

	//funkcja rekurencyjna parsujaca wyrazenie i budujaca jego drzewo,
	//opiera sie o algorytm rozrzadowy Dijkstry
	void readExprRecursively(std::stack<XPathExpression *>& ns, std::stack<XPathSymbol> & ops,
			const XPathSymbol delimiter);

	String Number();

	//SinglePath -> '/' |
	//			 ->	Sep? Node {Sep Node} ;
	XPath SinglePath();

	// Sep -> '//' | '/';
	void PathSep(XPath & path);

	//Node -> '..' | '.' | '*' | NodeName | AttrNode
	XPathNode Node();

	//AttrNode -> '@' ( '*' | NodeName)
	XPathNode AttrNode();

	/* Leaf -> Negation | '(' Expression ')' | Literal | Number | Path*/
	XPathExpression * Leaf();

	/* Negation -> 'not' '(' Expression ')' */
	XPathNegationExpression * Negation();

public:
	// Path -> SinglePath { '|' SinglePath};
	XPathLocationExpression * Path();

	// Expression -> Leaf { Op Leaf }
	XPathExpression * Expression(const XPathSymbol delimiter);

	XPathParser(ILexer * l);

	ParsedObject * startParsing() { return Expression(MAXSYM); };
};


}
#endif /* XPATHPARSER_H_ */
