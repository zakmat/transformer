/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XSLTStructs.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XSLTSTRUCTS_H_
#define XSLTSTRUCTS_H_

#include <vector>
#include "IParser.h"
#include "StringSource.h"
#include "XMLStructs.h"
#include "XPathStructs.h"

namespace parsingXSLT {

/*
 * atomy akceptowane przez analizator leksykalny XSLT, dziela sie na dwie grupy, pierwsza z nich to atomy wystepujace
 * rowniez w a.l. dla dokumentow XML, natomiast druga grupa to rozpoznawane slowa kluczowe procesora XSLT
 */

enum XSLSymbol {
	STYLESHEET, TEMPLATE, APPLYTEMPLATES, VALUEOF, FOREACH, IFCLAUSE, CHOOSE, WHEN, OTHERWISE, SORT,
	TEMPLATENAME, MATCH, PRIORITY, SELECT, ORDER, DATATYPE, TEST, MAXSYM
};

const int KEYWORDSNUM = MAXSYM - 1;

struct KeywordMapping {
		const char * key;
		XSLSymbol symbol;
	};

static KeywordMapping keywords[KEYWORDSNUM];

	//dla identyfikatorow obliczany jest hash
	//int calculateHash(const String & sequence) const;
	//funkcja probuje dopasowac odczytany identyfikator do slowa kluczowego
	//Token tryToMatchKeyword(const String & sequence);

class Context;
class Instruction;
class XSLTStylesheet;
class XSLTTemplate;
class XSLConditional;
class XSLSort;

typedef parsingXPath::XPathLocationExpression LocExpr;
typedef parsingXPath::XPathExpression XPathExpr;

typedef parsingXML::Node xmlNode;
typedef std::vector<parsingXML::Node*> NodeVec;

typedef std::vector<XSLTTemplate *> TemplateVec;
typedef std::vector<Instruction *> InstructionVec;
typedef std::vector<XSLConditional *> ConditionalVec;
typedef std::vector<XSLSort *> SortVec;



//klasa reprezentuje instrukcje procesora XSLT
class Instruction {
protected:
	XSLSymbol instrType;
public:
	void indent(const int & depth) {
		for(int i=0;i<depth;++i)
			std::cout << "  ";
	}
	//tylko na potrzeby debuggingu o ile dla drzew xml wypisany obiekt wyglada calkiem znosnie,
	//o tyle tu w ogole
	virtual void print(int depth) = 0;
	XSLSymbol type() const {return instrType; };
	//napotkana instrukcja jest realizowana w kontekscie biezacego wezla
	virtual NodeVec evaluate(const Context & context)const = 0;
	virtual Instruction* clone() const = 0;
	virtual ~Instruction() {};
};

enum TemplateType{TPATTERN, TNAME};

class XSLTTemplate {
	//sciezka do ktorej poszczegolne wezly probuja sie dopasowac
	LocExpr * pattern;
	//w zamysle chcialem jeszcze zaimplementowac instrukcje call-template ktora wywoluje template po nazwie,
	//zabraklo czasu
	String name;
	//priorytet - w razie gdy do wezla dopasowanie kilka wzorcow z kontekstu wybierany jest ten o najwyzszym priorytecie,
	//lub jesli takich jest wiecej - ostatni napotkany
	double priority;
	//cialo szablonu zawiera sekwencje instrukcji, w razie dopasowania zostana one wykonanie w biezacym kontekscie
	InstructionVec instructions;
public:
	XSLTTemplate(LocExpr * _pattern = NULL, String _name = "", double priority = 0.0, InstructionVec l = InstructionVec());
	XSLTTemplate(const XSLTTemplate & rhs);
	virtual ~XSLTTemplate();
	//for debug purposes
	void print();

	InstructionVec getBody() const {return instructions; };
	double getPriority() const { return priority; }
	bool isMatching(xmlNode * path) const;

};

//klasa reprezentujaca arkusz styli, w praktyce zbior szablonow do przegladania + szablon domyslny
class XSLTStylesheet : public ParsedObject {

	TemplateVec templates;

	//w przypadku gdy zaden z szablonow nie pasuje do wezla, wywolywany jest szablon wbudowany
	//odpowiada on instrukcji <xsl:apply-templates select="*"/>
	//i zapewnia dalsze, rekurencyjne przegladanie drzewa
	XSLTTemplate * defTemplate;
	XSLTTemplate * getDefaultTemplate() const;
public:
	XSLTStylesheet(const TemplateVec& t);
	virtual ~XSLTStylesheet();

	XSLTTemplate * findBestFittingTemplate(xmlNode * node) const;

	void print();
};


//klasa reprezentujaca jawny tekst w arkuszu styli, zostaje przeksztalcona do wezla tekstowego w drzewie wynikowym
//po uprzednim usunieciu skrajnych bialych znakow
class XSLText : public Instruction {
	String text;
public:
	void print(int d);
	XSLText(const String & _s = String()): text(_s) {};
	virtual XSLText* clone() const { return new XSLText(*this);};
	NodeVec evaluate(const Context & context)const;
};

//instrukcja value of ewaluuje wartosc wyrazenia select i uzyskany wynik konwertuje na string,
//czego efektem jest wezel tekstowy w drzewie wynikowym
class XSLValueOf : public Instruction {
	XPathExpr * selected;

public:
	void print(int d);
	XSLValueOf(XPathExpr * ns) : selected(ns) {};
	XSLValueOf(const XSLValueOf& rhs);
	virtual XSLValueOf * clone() const { return new XSLValueOf(*this);};
	~XSLValueOf() { if(selected) delete selected; }

	NodeVec evaluate(const Context & context)const;
};

//klasa reprezentujaca instrukcje for_each
//select jest wymagany
//na podstawie select obliczane sa wezly ktore staja sie nowa lista do przetworzenia
//nastepnie instrukcje sort sortuja tenze zbior zgodnie ze swa specyfikacja
//a ostatecznie na kazdym kolejnym wezle nowego kontekstu sa wykonywane instrukcje z body
class XSLRepetition : public Instruction {
	LocExpr * selected;
	SortVec sorts;
	InstructionVec body;

public:
	void print(int d);
	XSLRepetition(LocExpr * ns, const SortVec & s, const InstructionVec & b) : selected(ns), sorts(s),body(b) {};
	XSLRepetition(const XSLRepetition& rhs);
	virtual XSLRepetition* clone() const { return new XSLRepetition(*this);};
	~XSLRepetition();

	NodeVec evaluate(const Context & context)const;
};


//klasa sort implementuje porzadek rosnacy i malejacy
//oraz interpretuje typ sortowania jako liczbowy lub leksykograficzny
enum OrderVal {ASCENDING, DESCENDING};
enum DataType {TEXT, NUMBER};

class XSLSort : public Instruction {
//poniewaz do sortowania po wielu kluczach wykorzystuje stable_sort z STLa potrzebny mi byl predykat binarny
//ustalajacy porzadek zgodnie z wytycznymi konkretnej instrukcji sort - stad Comparator

//ponadto xsl:sort choc sortuje wezly to kluczami sa stringi wiec potrzebny byl mi odpowiedni wrapper
	struct SortWrapper {
		String val;
		xmlNode * n;
		SortWrapper(const String& v, xmlNode * n_): val(v), n(n_) {};
	};
	struct Comparator {
		const XSLSort * sort;
		Comparator(const XSLSort * s): sort(s) {};
		bool operator()(const SortWrapper& l, const SortWrapper& r) const;
	};
	//sluzy do wyboru klucza domyslnie jest to '.' czyli wywolujemy string na badanym wezle, ale w ogolnosci
	//moze to byc dowolne wyrazenie XPath
	XPathExpr * criterion;
	OrderVal order;
	DataType type;


	String string(const NodeVec& v) const;
	String string(xmlNode *n) const;
	bool compare(const String & a, const String &b) const;
public:
	OrderVal getOrder()const { return order;};
	DataType getType() const { return type; };
	void print(int d);
	XSLSort(XPathExpr * _s, OrderVal _o = ASCENDING, DataType _t = TEXT):
		criterion(_s), order(_o), type(_t) {};
	XSLSort(const XSLSort& rhs);
	virtual XSLSort* clone() const { return new XSLSort(*this);};
	~XSLSort();

	NodeVec evaluate(const Context & context)const;

	/*
	 * funkcja oblicza klucze dla poszczegolnych wezlow ktore bedziemy sortowac
	 *
	 * Klucz dla wezla jest obliczony poprzez ewaluacje wyrazenia select w kontekscie tegoz wezla, a rezultat
	 * jest konwertowany do stringa
	 */
	std::vector<String> extractKeys(Context & c)const;
};

//jedyna oprocz for-each instrukcja zmieniajaca kontekst przetwarzania
//Wskazane za pomoca select wezly tworza nowa liste wezlow do ktorych nastepnie dopasowuje sie szablony
class XSLApplyTemplates : public Instruction {
	//jesli select sie nie pojawil to domyslnie do przetworzenia powolywane sa wszystkie potomki biezacego wezla
	LocExpr * selected;
	//opcjonalnie mozna posortowac wezly przed ich przetworzeniem
	SortVec sorts;
public:
	void print(int d);
	XSLApplyTemplates(LocExpr * s = NULL, const SortVec& sorts_= SortVec()): selected(s), sorts(sorts_) {};
	XSLApplyTemplates(const XSLApplyTemplates& rhs);
	virtual XSLApplyTemplates* clone() const { return new XSLApplyTemplates(*this);};
	~XSLApplyTemplates();

	NodeVec evaluate(const Context & context)const;
};

//instrukcja warunkowa - cialo jest wykonane jesli wyrazenie jest prawdziwe dla biezacego kontekstu
class XSLConditional : public Instruction {
	XPathExpr * expression;
	InstructionVec instructions;

public:
	XPathExpr * getExpression() const { return expression; };
	InstructionVec getBody() const { return instructions; };
	void print(int d);
	XSLConditional(XPathExpr * expr, const InstructionVec & i):
		expression(expr), instructions(i) {};
	virtual XSLConditional* clone() const { return new XSLConditional(*this);};
	XSLConditional(const XSLConditional& rhs);
	~XSLConditional();
	NodeVec evaluate(const Context & context) const;
};


//instrukcja rozgalezienia
//testowane sa kolejne warunki, dla pierwszego prawdziwego jest wykonywane cialo,
//a nastepne warunki nie sa juz nawet sprawdzane
//jesli zaden z warunkow nie okazal sie prawdziwy a mamy zdefiniowana instrukcje otherwise,
//to jest ona wykonywana

class XSLBranch : public Instruction {
	bool isDefaultDefined;
	ConditionalVec conditionalBranches;
	InstructionVec defBranch;

public:
	void print(int d);
	XSLBranch(ConditionalVec cB, InstructionVec dB = InstructionVec());
	virtual XSLBranch* clone() const { return new XSLBranch(*this);};
	XSLBranch(const XSLBranch& rhs);
	~XSLBranch();

	NodeVec evaluate(const Context & context)const;
};


/*
 * Poniewaz arkusz styli jest w szczegolnosci poprawnie zdefiniowanym dokumentem XML,
 * jesli napotkamy znak '<' i dalsza sekwencja nie zostanie zinterpretowana jako instrukcja XSLT
 * to i tak nawiasowanie zostanie zachowane, a zatem element taki mozna przeniesc bezposrednio
 * z drzewa zrodlowego do wynikowego, a procesor XSLT powinien rekurencyjnie przeszukac wszyskich
 * potomkow elementu w poszukiwaniu instrukcji do przetworzenia.
 *
 * Zatem korzen i atrybuty sa przenoszone bez zmian, a dzieci sa traktowane jak instrukcje
 * (ktore byc moze rowniez zostana zinterpretowane jako instrukcje zlozone).
 *
 */
class XSLComplex : public Instruction {
	typedef std::vector<std::pair< String, String> > AttrList;
	parsingXML::Name name;
	NodeVec attrs;
	InstructionVec children;

public:
	void print(int d);
	XSLComplex(const parsingXML::Name & _name, const NodeVec & _attrs,
			const InstructionVec & _children = InstructionVec()):
				name(_name), attrs(_attrs), children(_children) {};
	virtual XSLComplex* clone() const { return new XSLComplex(*this);};
	XSLComplex(const XSLComplex& rhs);
	~XSLComplex() ;
	NodeVec evaluate(const Context & context) const;
};


/*
 * Klasa Context jest kluczowa jesli chodzi o przetwarzanie dokumentu XML z pomoca arkusza XSLT
 * Context przechowuje informacje na temat aktualnie przetwarzanego (tzw. biezacego) wezla wejsciowego oraz liste
 * wezlow ktore wciaz oczekuja na przetworzenie.
 * Znajomosc biezacego wezla jest potrzebna podczas ewaluacji sciezek XPath
 */
class Context {
	XSLTStylesheet * stylesheet;
	int position;
	NodeVec currentList;
public:
	Context(const NodeVec& l, XSLTStylesheet * s): stylesheet(s), position(0), currentList(l) {};
	xmlNode * getCurrent() const;
	int getPosition() const;
	void setPosition(int i) {position = i; };
	int getSize() const;
	NodeVec getCurrentList() const;
	void setCurrentList(const NodeVec& v) { currentList = v; };
	NodeVec select (LocExpr * s) const;
	bool test(XPathExpr * e) const;
	NodeVec instantiate(const InstructionVec & v) const;
	NodeVec instantiateForAll(const InstructionVec & v);


	void sort(const SortVec& sorts);
	NodeVec proccess();

	XSLTStylesheet * getStylesheet() const {return stylesheet;};
};


}

#endif /* XSLTSTRUCTS_H_ */
