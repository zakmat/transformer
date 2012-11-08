/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XPathParser.cpp
 *
 * Autor: Mateusz Zak
 */


#include "XPathParser.h"
#include "XPathStructs.h"

namespace parsingXPath {

char const * xpathTokDesc[MAXSYM] = {
		"/", "//" , "*", "@", "..", ".", "node", "number", "literal", "|", "not", "or", "and",
		"=", "!=", ">", "<", ">=", "<=", "+", "-", "(", ")", "whitespace", "other"
};

XPathParser::XPathParser(ILexer * l): IParser(l) {
	tokenDescriptions = xpathTokDesc;
	getNextToken();
}

/*wczytywanie wyrazenia opiera sie o algorytm rozrzadzowy Djikstry
 *
 * sekwencja do przetworzenia zakonczona jest straznikiem przekazanym do funkcji jako argument,
 * czasami bedzie to ')' czasami MAXSYM reprezentujacy koniec sekwencji zrodlowej
 */
XPathExpression * XPathParser::Expression(const XPathSymbol delimiter) {
	std::stack<XPathExpression *> exprStack;
	std::stack<XPathSymbol> opStack;

	exprStack.push(Leaf());
	//rekurencyjnie wczytujemy wyrazenie
	readExprRecursively(exprStack, opStack, delimiter);

	XPathExpression * l, * r;

	//konczymy konstrukcje wyrazenia
	while(!opStack.empty()) {
		r = exprStack.top();
		exprStack.pop();
		l = exprStack.top();
		exprStack.pop();
		exprStack.push(new XPathExpression(l,r,opStack.top()));
		opStack.pop();
	}
	return exprStack.top();
}

/*
 * na poczatku funkcja wywolywana jest ze stosem podwyrazen liczacym 1 element i pustym stosem operatorow
 */
void XPathParser::readExprRecursively(std::stack<XPathExpression *>& ns, std::stack<XPathSymbol> & ops, const XPathSymbol delim) {
	//funkcja wykonywana jest az do natrafienia na symbol strazniczy
	while(symbol!=delim) {
		//jesli stos operatorow jest pusty lub natrafilismy na op o nizszym precedensie to powiekszamy stosy
		if(ops.empty() || hasLowerPrecedence(ops.top(),(XPathSymbol)symbol)) {
			ops.push((XPathSymbol)symbol);
			accept(symbol);
			ns.push(Leaf());
		}
		//wpp jesli poprzedni operator ma taki sam lub wyzszy precedens to znaczy ze czas
		//nalezy zdjac ze stosu ostatnie dwa wyrazenia oraz operator,
		//polaczyc wyrazenia i rezultat wlozyc na stos
		//a nastepnie wywolac funkcje rekurencyjnie
		else {
			XPathExpression * r = ns.top();
			ns.pop();
			XPathExpression * l = ns.top();
			ns.pop();
			ns.push(new XPathExpression(l,r,ops.top()));
			ops.pop();
			readExprRecursively(ns,ops,delim);
		}
	}
}

//prawda wtw gdy op1 ma ostro nizszy precedens niz op2
bool XPathParser::hasLowerPrecedence(const XPathSymbol op1, const XPathSymbol op2) const {
	if (op1==OR && op2!=OR)
		return true;
	if(op1==AND && op2!=OR && op2!=AND)
		return true;
	if(isEqOp(op1) && op2!=OR && op2!=AND && !isEqOp(op2))
		return true;
	if(isRelOp(op1) && !isRelOp(op2) && !isEqOp(op2) && op2!=AND && op2!=OR)
		return true;
	if(isAddOp(op1) && isMulOp(op2))
		return true;
	return false;
}

//zwraca true jesli operator nalezy do grupy = lub !=
bool XPathParser::isEqOp(const XPathSymbol op) const {
	return op==EQ || op==NEQ;
}

//zwraca true jesli op jest operatorem porownania
bool XPathParser::isRelOp(const XPathSymbol op) const {
	return op == LT || op == LTE || op == GT || op == GTE;
}

//zwraca true jesli jest operatorem addytywnym
bool XPathParser::isAddOp(const XPathSymbol op) const {
	return op==PLUS || op==MINUS;
}

//zwraca true dla operatora mnozenia
bool XPathParser::isMulOp(const XPathSymbol op) const {
	return op == ASTERISK;
}

/* Leaf = Negation | '(' Expression ')' | Literal | Number | Path */
XPathExpression * XPathParser::Leaf() {
	if(isStartOfPathExpr((XPathSymbol)symbol))
		return Path();
	else if(symbol==NOT)
		return Negation();
	else if(symbol==LBRACK) {
		accept(LBRACK);
		XPathExpression * e = Expression(RBRACK);
		accept(RBRACK);
		return e;
	}
	else if(symbol==LITERAL) {
		accept(LITERAL);
		return new XPathLiteralExpression(accepted.lexeme);
	}
	else if(symbol==MINUS || symbol==NUM) {
		String ret;
		if(symbol==MINUS) {
			accept(MINUS);
			ret = String("-").append(Number());
		}
		else
			ret = Number();
		return new XPathNumericExpression(ret);
	}
	else {
		syntaxError("Napotkany symbol nie odpowiada zadnemu wyrazeniu atomowemu. Nieoczekiwany token: ", symbol);
		return 0;
	}
}

//fcja zwraca true jesli nastepny symbol rozpoczyna XPath
bool XPathParser::isStartOfPathExpr(XPathSymbol s) const {
	return s==SLASH || s==ALL || isStartOfNode(s);
}

//funkcja zwraca true jesli nastepny symbol rozpoczyna wezel sciezki
bool XPathParser::isStartOfNode (XPathSymbol s) const {
	return s==ASTERISK || s==ATTR || s==PARENT || s==CURRENT || s==NODE;
}

//sprawdza czy atom jest operatorem
bool XPathParser::isOperator(XPathSymbol s) const {
	return isRelOp(s) || isEqOp(s) || isAddOp(s) || isMulOp(s) || s==NODEOR || s==OR || s==AND;
}

/* Negation = 'not' '(' Expression ')' */
XPathNegationExpression * XPathParser::Negation() {
	accept(NOT);
	accept(LBRACK);
	XPathExpression * e = Expression(RBRACK);
	accept(RBRACK);
	return new XPathNegationExpression(e);
}

String XPathParser::Number() {
	accept(NUM);
	return accepted.lexeme;
}

// Path = SinglePath { '|' SinglePath}
XPathLocationExpression * XPathParser::Path() {
	std::vector<XPath> ret;

	ret.push_back(SinglePath());
	while(symbol==NODEOR) {
		accept(NODEOR);
		if(isStartOfPathExpr((XPathSymbol)symbol))
			ret.push_back(SinglePath());
		else {
			syntaxError("Nadmiarowy token: ", NODEOR);
			return new XPathLocationExpression(ret);
		}
	}
	return new XPathLocationExpression(ret);
}


/* SinglePath = '/' | Sep? Node {Sep Node} ;
 */
XPath XPathParser::SinglePath() {
	XPath path(false);

	if (symbol==SLASH) {//albo root ('/') albo sciezka absolutna
		path = XPath(true);
		PathSep(path);
		if(isOperator((XPathSymbol)symbol) || symbol == MAXSYM)//czyli root
			return path;
	}
	else if(symbol == ALL) {//nadal absolutna ale do wszystkich
		path = XPath(true);
		PathSep(path);
	}
	else// sciezka relatywna
		path = XPath(false);

	//obsluga pierwszego wezla jest na tym etapie przetwarzania obowiazkowa, bowiem
	//wykluczylismy przypadek szczegolny '/'
	if(isStartOfNode((XPathSymbol)symbol)) {
		XPathNode n = Node();
		path.append(n);
		if(n.isAttrNode())
			return path;
	}
	else { //wyswietlamy blad
		syntaxError("Nieprawidlowy format sciezki. Niespodziewany token: ", symbol);
		return path;
	}

	//zczytujemy kolejne wezly sciezki
	while(symbol==SLASH || symbol==ALL) {
		PathSep(path);
		if(isStartOfNode((XPathSymbol)symbol)) {
			XPathNode n = Node();
			path.append(n);
			if(n.isAttrNode())
				return path;
		}
		else { //wyswietlamy blad
			syntaxError("Nieprawidlowy format sciezki. Niespodziewany token: ", symbol);
			return path;
		}
	}
	//TODO ewentualnie dodac obsluge predykatow, nie starczylo na to czasu
	return path;
}

// Sep = '//' | '/';
void XPathParser::PathSep(XPath & path) {
	if(symbol==ALL) {
		accept(ALL);
		path.branchToAllDescendants();
	}
	else
		accept(SLASH);
}

//Node = '..' | '.' | '*' | NodeName | AttrNode
XPathNode XPathParser::Node() {
	switch (symbol) {
	case ATTR:
		return AttrNode();
	case PARENT:
		accept(PARENT);
		//		accept(NODE);
		return XPathNode(PARENTT);
	case CURRENT:
		accept(CURRENT);
		//		accept(NODE);
		return XPathNode(CURRENTT);
	case ASTERISK:
		accept(ASTERISK);
		return XPathNode(ANYT);
	default:
		accept(NODE);
		return XPathNode(SIMPLET, accepted.lexeme);
	}
}

//AttrNode = '@' ( '*' | NodeName)
XPathNode XPathParser::AttrNode() {
	accept(ATTR);
	if(symbol==ASTERISK)
		return XPathNode(ANYATTRT);
	if(symbol==NODE) {
		accept(NODE);
		return XPathNode(ATTRT, accepted.lexeme);
	}
	syntaxError("Nazwa atrybutu nieprawidlowa. Napotkano token: ", symbol);
	//zwroc wszystkie atrybuty
	return XPathNode(ANYATTRT);
}



}

