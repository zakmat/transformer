/*
 * Parser plików XML oraz wykonywanie przekształcenia XSLT
 * XPathStructs.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XPATHSTRUCTS_H_
#define XPATHSTRUCTS_H_

#include "IParser.h"
#include "XMLStructs.h"
#include "XPathLexer.h"

namespace parsingXPath {

//przydatne typedefy
typedef parsingXML::Node xmlNode;
typedef std::vector<xmlNode *> NodeVec;



/*
 * najbardziej ogolna klasa reprezentujaca wyrazenie XPath
 * jest w istocie drzewem etykietowanym operatorami
 */
class XPathExpression : public ParsedObject{
	XPathExpression * left;
	XPathExpression * right;
	//symbol reprezentuje operator - jest to etykieta wezla
	XPathSymbol type;


	bool compareNumbers(double a, double b) const;
	bool compareStrings(const String& a, const String& b) const;

	/*
	 * ponizsze 4 funkcje sa wykorzystywane przy ustalaniu typu wyrazenia zlozonego
	 * robia to na podstawie operatora. Ewaluacja wyrazenia zalezy od jego typu.
	 */
	bool isLogicalExpr() const;
	bool isArithmeticExpr() const;
	bool isEqualityExpr() const;
	bool isRelationalExpr() const;

public:
	//W XPath w wersji 1.0 wystepuja 4 podstawowe typy: boolean, string, number i nodeset
	//ponizsze funkcje umozliwiaja konwersje wyrazenia do typow prostych
	virtual bool boolean(xmlNode * contextNode) const;
	virtual String string(xmlNode * contextNode) const;
	virtual double number(xmlNode * contextNode) const;

	//ponizsze funkcje wirtualne maja na celu ustalenie z jakim rodzajem wyrazenia mamy do czynienia
	virtual bool isNumericExpr() const;
	virtual bool isBooleanExpr() const;
	virtual bool isLocationExpr() const;

	XPathExpression(XPathExpression * l, XPathExpression *r = NULL, XPathSymbol s=MAXSYM):
		left(l), right(r), type(s) {}

	virtual XPathExpression * clone() { return new XPathExpression(*this); }
	XPathExpression(const XPathExpression& rhs);
	virtual ~XPathExpression() {if (left) delete left; if(right) delete right;};

};

/*
 * ponizsze klasy reprezentuja pewna specjalizacje wyrazenia XPath,
 * kazdy typ liscia w drzewie ma zdefiniowana klase
 *
 * Pierwszym jest wyrazenie negacji
 */
class XPathNegationExpression : public XPathExpression{
	XPathExpression * arg;
public:
	bool boolean(xmlNode * contextNode) const;
	String string(xmlNode * contextNode) const;
	double number(xmlNode * contextNode) const;


	bool isNumericExpr() const{return false;};
	bool isBooleanExpr() const {return true;};
	bool isLocationExpr() const {return false;};

	XPathNegationExpression(XPathExpression * e): XPathExpression(NULL), arg(e){};
	~XPathNegationExpression() { if(arg) delete arg; };
	virtual XPathExpression * clone() { return new XPathNegationExpression(*this); }
};


//wyrazenie liczbowe
class XPathNumericExpression: public XPathExpression {
	String value;
public:
	bool boolean(xmlNode * contextNode) const;
	String string(xmlNode * contextNode) const;
	double number(xmlNode * contextNode) const;

	bool isNumericExpr() const{return true;};
	bool isBooleanExpr() const{return false;};
	bool isLocationExpr() const{return false;};

	XPathNumericExpression(const String&  v): XPathExpression(NULL), value(v) {};
	virtual XPathExpression * clone() { return new XPathNumericExpression(*this); }
};

//wyrazenie - literal
class XPathLiteralExpression: public XPathExpression {
	String value;
public:
	bool boolean(xmlNode * contextNode) const;
	String string(xmlNode * contextNode) const;
	double number(xmlNode * contextNode) const;

	bool isNumericExpr() const {return false;};
	bool isBooleanExpr() const {return false;};
	bool isLocationExpr() const{return false;};

	XPathLiteralExpression(const String&  v): XPathExpression(NULL), value(v) {};
	virtual XPathExpression * clone() { return new XPathLiteralExpression(*this); }
};

class XPath;

//wreszcie wyrazenie bedace unia sciezek
class XPathLocationExpression: public XPathExpression {
	std::vector<XPath> paths;

public:
	bool boolean(xmlNode * contextNode) const;
	String string(xmlNode * contextNode) const;
	double number(xmlNode * contextNode) const;

	bool isNumericExpr() const {return false;};
	bool isBooleanExpr() const {return false;};
	bool isLocationExpr() const{return true;};

	bool compareWithOtherExpr(XPathExpression * other, xmlNode * contextNode) const;

	//funkcja sprawdza czy wezel kontekstowy mozna dopasowac do sciezki
	//(uzywana jest podczas dopasowywania template'ow)
	bool match(xmlNode * contextNode) const;

	//funkcja oblicza wezly do jakich mozna dojsc z wezla kontekstowego z pomoca sciezek z tego wyrazenia
	NodeVec evaluate(xmlNode * contextNode) const;

	//jako ze zbiory wezlow uzyskane podczas ewaluacji poszczegolnych sciezek moga posiadac niezerowa czesc wspolna
	//nalezy zapewnic sposob ich laczenia, ponadto funkcja zwraca wezly w porzadku dokumentu
	NodeVec mergeSets(const NodeVec& v1, const NodeVec& v2) const;

	XPathLocationExpression(const std::vector<XPath>&  p): XPathExpression(NULL), paths(p) {};
	void print() const;

};

//enum reprezentujacy typy wezlow w sciezce
//sa to kolejno zwykly potomek, wezel aktualny('.'), rodzic('..'),
//dowolny potomek ('*'), atrybut('@'), dowolny atrybut('@*')
enum NodeType {SIMPLET, CURRENTT, PARENTT, ANYT, ATTRT, ANYATTRT};

class XPathNode {
	NodeType type;
	String name;
public:
	bool isAttrNode() const {return type == ANYATTRT || type ==ATTRT;};
	XPathNode(NodeType t, String n=""):type(t), name(n){};

	NodeVec selectChildren(const NodeVec& base) const;
	bool matchElementNodeName(const String &n) const;

	void print() const;
};

//obiekt klasy reprezentuje pojedyncza sciezke
//sciezka to wektor wezlow sciezki, oddzielony separatorami, ktorych rozrozniamy dwa rodzaje:
// '/' - reprezentuje przejscie do bezposredniego potomka, natomiast '//' dowolnego zstepnego lub siebie samego
class XPath {
	std::vector<XPathNode> path;
	bool isAbsolute;
	//wektor utrzymuje informacje na ktorym poziomie mamy przejscia zwykle, a na ktorych 'glebokie'
	std::vector<int> descendantOrSelfLevel;

	//jesli mielismy do czynienia z 'glebokim' krokiem to dla wszystkich dopasowanych do tej pory wezlow (base)
	//musimy dopasowac pasujacych zstepnych
	NodeVec descOrSelf(const NodeVec& base) const;

	//sprawdza czy n-ty krok w sciezce jest 'gleboki'
	bool isDescStep(int n) const;

	//funkcja zwraca wektor wezlow XML v, po usunieciu duplikatow i posortowaniu wezlow
	//w kolejnosci dokumentu zrodlowego
	NodeVec pruneAndSort(const NodeVec &v) const;

public:
	XPath(bool abs_): isAbsolute(abs_), descendantOrSelfLevel() {};

	//uzywany podczas budowania sciezki do dodania kroku na sciezce
	void append(XPathNode n);
	//uzywany podczas budowania sciezki zaznacza, ze krok byl 'gleboki'
	void branchToAllDescendants();

	//match i evaluate maja to samo znaczenie co w XPathLocationExpression,
	bool match(xmlNode * contextNode) const;
	NodeVec evaluate(xmlNode * contextNode) const;

	void print() const;
};

/*
 * Funktor operujacy na wskaznikach do wezlow ustalajacy je w porzadku dokumentu
 */
class DocumentOrderComparator {
public:
	bool operator()(xmlNode * a, xmlNode * b) {
		return a->getNumber() < b->getNumber();
	}
};

}


#endif /* XPATHSTRUCTS_H_ */
