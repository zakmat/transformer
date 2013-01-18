/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XPathStructs.cpp
 *
 * Autor: Mateusz Zak
 */


#include <math.h>
#include <limits>
#include <sstream>
#include "XPathStructs.h"

namespace parsingXPath {

XPathExpression::XPathExpression(const XPathExpression& rhs) {
	if(rhs.left)
		left = rhs.left->clone();
	else
		left = NULL;
	if(rhs.right)
		right = rhs.left->clone();
	else
		right = NULL;
	type = rhs.type;
}

bool XPathExpression::isNumericExpr() const {
	return isArithmeticExpr();
}

bool XPathExpression::isBooleanExpr() const {
	return isLogicalExpr() || isRelationalExpr() || isEqualityExpr();
}

bool XPathExpression::isLocationExpr() const {
	return false;
}

bool XPathExpression::isLogicalExpr() const {
	return type == AND || type == OR;
}

bool XPathExpression::isArithmeticExpr() const {
	return type == PLUS || type == MINUS || type == ASTERISK;
}

bool XPathExpression::isEqualityExpr() const {
	return type == EQ || type == NEQ;
}

bool XPathExpression::isRelationalExpr() const {
	return type == LT || type == LTE || type == GT || type == GTE;
}

bool XPathExpression::compareNumbers(double a, double b) const {
	switch(type) {
	case LT: 	return a<b;
	case LTE: 	return a<=b;
	case GT: 	return a>b;
	case GTE:	return a>=b;
	case EQ:	return a==b;
	case NEQ:	return a!=b;
	default:	return false;
	}
}

bool XPathExpression::compareStrings(const String &a, const String&b) const {
	switch(type) {
	case EQ:	return a.compare(b)==0;
	case NEQ:	return a.compare(b)!=0;
	default:	return false;
	}
}

bool XPathExpression::boolean(Node * contextNode) const {

	/*
	 * zgodnie ze specyfikacja jesli wyrazenie posiada operator arytmetyczny to dokonuje konwersji i oblicza wynik
	 */
	if(isArithmeticExpr()) {
		double result = number(contextNode);
		return result!=0;
		//TODO isnan nie rozpoznawana w tej wersji kompilatora
		//return !std::isnan(result) && result!=0;
	}
	/*
	 * jesli nadrzednym operatorem jest op. logiczny nastepuje konwersja obu podwyrazen do boolean i obliczenie wyrazenia glownego
	 */
	if(isLogicalExpr()) {
		if(type==OR)
			return left->boolean(contextNode) || right->boolean(contextNode);
		else if(type==AND)
			return left->boolean(contextNode) && right->boolean(contextNode);
	}
	/*
	 * przypadek dla operatorow rownosci, nierownosci i relacyjnych jest najbardziej skomplikowany
	 */
	if(isRelationalExpr()) {
		/*
		 * przypadek dla operatorow relacyjnych *zawsze* dokonuje konwersji na liczby
		 * spedzilem duzo czasu na w3.schools.com, bawiac sie ich procesorem XSLT by sie o tym upewnic
		 *
		 * jednak w przypadku gdy ktorys z operandow jest zbiorem wezlow zachowanie staje sie bardziej wysublimowane
		 * niz bezmyslna konwersja string-value
		 */
		if(left->isLocationExpr()&&right->isLocationExpr()) {
			//porownanie metoda kazdy z kazdym, jesli choc dla jednego warunek jest poprawny to true wpp false

			XPathLocationExpression * l = (XPathLocationExpression *) left;
			XPathLocationExpression *r = (XPathLocationExpression *) right;

			NodeVec lns = l->evaluate(contextNode);
			NodeVec rns = r->evaluate(contextNode);

			for(NodeVec::iterator itl = lns.begin(); itl!= lns.end(); itl++)
				for(NodeVec::iterator itr = rns.begin(); itr!=rns.end(); itr++)
					if(compareNumbers((*itl)->numberValue(),(*itr)->numberValue()))
						return true;

			return false;
		}
		else if(!left->isLocationExpr()&&!right->isLocationExpr()) {
			return compareNumbers(left->number(contextNode),right->number(contextNode));
		}
		//ponizsze 2 przypadki obsluguja sytuacje gry jeden z operandow jest zbiorem wezlow
		//wowczas porownanie nastepuje z kazdym z wezlow z osobna (wezly konwertowane sa na liczby)
		else if(left->isLocationExpr())
			return ((XPathLocationExpression*)left)->compareWithOtherExpr(right, contextNode);
		else
			return ((XPathLocationExpression*)right)->compareWithOtherExpr(left, contextNode);
			//TODO napisać test case w którym tylko jeden z operandów jest zbiorem ścieżek
	}
	if(isEqualityExpr()) {
		/*
		 * Rozrozniamy 3 przypadki:
		 *
		 * Pierwszy przypadek gdy obydwa wyrazenia sa sciezkami:
		 * w. jest prawdziwe wtw gdy w poszczegolnych zbiorach istnieja wezly takie ze porownanie ich string-values jest prawdziwe
		 *
		 */
		if(left->isLocationExpr()&&right->isLocationExpr()) {
			XPathLocationExpression * l = (XPathLocationExpression *) left;
			XPathLocationExpression *r = (XPathLocationExpression *) right;

			NodeVec lns = l->evaluate(contextNode);
			NodeVec rns = r->evaluate(contextNode);

			for(NodeVec::iterator itl = lns.begin(); itl!= lns.end(); itl++)
				for(NodeVec::iterator itr = rns.begin(); itr!=rns.end(); itr++)
					if(compareStrings((*itl)->string(),(*itr)->string()))
						return true;

			return false;
		}
		/*
		 * Drugi przypadek gdy zaden nie jest zbiorem wezlow
		 */
		if(!left->isLocationExpr()&&!right->isLocationExpr()) {
			/*
			 * przypadek dla operatorow == oraz !=
			 * dokonuje konwersji operandow na ten sam typ zgodnie z precedensem: boolean, number, string
			 */

			//jesli co najmniej jeden jest boolean to oba operandy nalezy skonwertowac do tego typu
			if(left->isBooleanExpr() || right->isBooleanExpr())
				return ((type==EQ && left->boolean(contextNode) == right->boolean(contextNode)) ||
						(type==NEQ && left->boolean(contextNode)!= right->boolean(contextNode)));
			//jesli co najmniej jeden jest liczba to oba trzeba zamienic na liczby
			else if(left->isNumericExpr() || right->isNumericExpr()) {
				return compareNumbers(left->number(contextNode),right->number(contextNode));
			}
			//ostatni przypadek obie strony konwertujemy do stringow
			else {
				return compareStrings(left->string(contextNode),right->string(contextNode));
			}
		}
		/*
		 * Trzeci przypadek gdy tylko jeden operand jest zbiorem wezlow
		 */
		else {
			XPathExpression *other;
			XPathLocationExpression * ns;
			//ustalamy ktory jest ktory
			if(left->isLocationExpr()) { ns = (XPathLocationExpression*)left;	other = right; }
			else { ns = (XPathLocationExpression*)right; other = left; }

			//dalsze postepowanie zalezy od typu drugiego elementu
			if(other->isNumericExpr()) {
				//dla liczby nastepuje porownania z ktorymkolwiek wezlem
				double num = other->number(contextNode);
				NodeVec result = ns->evaluate(contextNode);
				for(NodeVec::iterator it = result.begin(); it!=result.end(); it++)
					if(compareNumbers((*it)->numberValue(),num))
						return true;

				return false;
			}
			//dla boolean robimy konwersje
			else if(other->isBooleanExpr()) {
				return ((type==EQ && ns->boolean(contextNode) == other->boolean(contextNode)) ||
						(type==NEQ && ns->boolean(contextNode)!= other->boolean(contextNode)));
			}
			else {//drugi operand to literal
				//compare(left)
				NodeVec result = ns->evaluate(contextNode);
				String str = other->string(contextNode);

				for(NodeVec::iterator it = result.begin(); it!=result.end(); it++)
					if(compareStrings((*it)->string(),str))
						return true;
				//jesli przejdzie po calym zbiorze i nie dopasuje zadnego stringa to
				return false;
			}

		}

	}

}

double XPathExpression::number(Node * contextNode) const {
	if(isArithmeticExpr()) {
		double l = left->number(contextNode);
		double r = right->number(contextNode);
		if(type == PLUS)
			return l+r;
		else if(type == MINUS)
			return l-r;
		else
			return l*r;
	}
	else if(isBooleanExpr()) {
		return boolean(contextNode)? 1.0 : 0.0;
	}
}

String XPathExpression::string(Node * contextNode) const {
	if(isBooleanExpr())
		return boolean(contextNode)?"true":"false";
	else if(isNumericExpr()) {
		double res = number(contextNode);
		if(isnan(res))
			return "NaN";
		else if(res==std::numeric_limits<double>::infinity())
			return "Infinity";
		else if(res*-1.0==std::numeric_limits<double>::infinity())
			return "-Infinity";

		std::ostringstream oss;
		oss << res;

		return oss.str();
	}

	//debug purposes, to nie powinno sie zdarzyc
	return "Nie rozpoznano stringa";
}

bool XPathNegationExpression::boolean(Node * contextNode) const {
	return !arg->boolean(contextNode);
}
String XPathNegationExpression::string(Node * contextNode) const {
	return boolean(contextNode)?"true":"false";
}
double XPathNegationExpression::number(Node * contextNode) const {
	return boolean(contextNode)? 1.0 : 0.0;
}

bool XPathNumericExpression::boolean(Node * contextNode) const {
	double d = getNumericValue(value);
	return d!=0.0;
	//TODO isnan nie rozpoznawana w tej wersji kompilatora
	//return !std::isnan(d) && d!=0.0;
}

String XPathNumericExpression::string(Node * contextNode) const {
	return value;
}
double XPathNumericExpression::number(Node * contextNode) const {
	return getNumericValue(value);
}

bool XPathLiteralExpression::boolean(Node * contextNode) const {
	return value.size()!=0;
}

String XPathLiteralExpression::string(Node * contextNode) const {
	return value;
}

double XPathLiteralExpression::number(Node * contextNode) const {
	return getNumericValue(value);
}

bool XPathLocationExpression::boolean(Node * contextNode) const {
	NodeVec r = evaluate(contextNode);
	return r.size() > 0;
}

String XPathLocationExpression::string(Node * contextNode) const {
	NodeVec v = evaluate(contextNode);

	if (v.size()==0)
		return "";
	else
		return v.at(0)->string();
}

double XPathLocationExpression::number(Node * contextNode) const {
	return getNumericValue(string(contextNode));
}

bool XPathLocationExpression::compareWithOtherExpr(XPathExpression* other, Node * contextNode) const {
	double num = other->number(contextNode);
	NodeVec result = this->evaluate(contextNode);

	for(NodeVec::iterator it = result.begin(); it!=result.end(); it++)
		if(compareNumbers((*it)->numberValue(),num))
			return true;

	return false;
}

bool XPathLocationExpression::match(Node * contextNode) const {
	for(std::vector<XPath>::const_iterator it=paths.begin(); it!=paths.end(); it++) {
		if(it->match(contextNode))
			return true;
	}
	return false;
}

void print2(NodeVec v) {
	for(unsigned i = 0;i<v.size();++i) {
		std::cout << v.at(i)->string() << " numer wezla w porzadku: " << v.at(i)->getNumber() << '\n';
	}
	std::cout.flush();
}


NodeVec XPathLocationExpression::evaluate(Node * contextNode) const {
	NodeVec ret;
	//oblicz rezultaty dla poszczegolnych sciezek i polacz je
	for(std::vector<XPath>::const_iterator it=paths.begin(); it!=paths.end(); it++) {
		NodeVec temp = it->evaluate(contextNode);
		ret = mergeSets(ret,temp);
	}

	return ret;
}

//zalozenie jest takie, ze oba zbiory wezlow sa posortowane przed wywolaniem funkcji
NodeVec XPathLocationExpression::mergeSets(const NodeVec& v1, const NodeVec& v2) const {
	NodeVec ret(v1.size()+v2.size());
	DocumentOrderComparator comp;

	NodeVec::iterator it;
	it = std::set_union(v1.begin(),v1.end(),v2.begin(),v2.end(),ret.begin(),comp);
	ret.resize(it-ret.begin());

	return ret;
}

//funkcja sprawdza czy mozna dopasowac wezel do sciezki
bool XPath::match(Node * contextNode) const {
	/* wezel pasuje do wzorca o ile istnieje kontekst dla ktorego
	 * po ewaluacji sciezki wynikowy zbior wezlow bedzie zawierac wezel testowany
	 *
	 * (kontekstem mozeby sam testowany wezel lub jego dowolny przodek)
	 */
	Node * base = contextNode;
	//	NodeVec result = evaluate(contextNode);
	while(base) {
		NodeVec result = evaluate(base);
		if (find(result.begin(),result.end(),contextNode) != result.end())
			return true;
		base = base->getParent();
	}
	return false;
}

void XPathLocationExpression::print()const {
	std::vector<XPath>::const_iterator it;
	for(it = paths.begin();it!=paths.end();++it)
		it->print();
	//	std::for_each(paths.begin(),paths.end(),std::mem_fun(&XPath::print));
}


void XPath::print() const {
	if(isDescStep(0))
		std::cout << "//";
	else if(isAbsolute)
		std::cout << "/";
	unsigned depth = 0;
	while(depth<path.size()) {
		path.at(depth).print();
		if(isDescStep(depth))
			std::cout << "//";
		else
			std::cout << '/';
		depth++;
	}
}

//funkcja zwraca wektor wezlow z do ktorych mozna dotrzec spacerujac sciezka
NodeVec XPath::evaluate(Node * contextNode) const {
	NodeVec ret;
	NodeVec matchedSoFar;

	unsigned depth = 0;
	if(!isAbsolute) {
		depth = 0;
		matchedSoFar.push_back(contextNode);
		//dalsza czesc jest wspolna dla obu przypadkow, wiec znajduje sie za blokiem else
	}
	else {//sciezka jest absolutna
		Node * base = contextNode->getRoot();

		/* przypadek: select="/"
		 * w specyfikacji XPath slash oznacza korzen tj wezel gdzie obok
		 * wlasciwego elementu glownego dokumentu jako dziecka wystepuja rowniez komentarze i
		 * instrukcje PI (<? />). Aplikacja ich nie interpretuje wiec zostaja one pominiete
		 * na etapie parsowania. Wezlem glownym mojej reprezentacji dokumentu XML
		 * jest wezel glownego elementu. Ta niespojnosc musi tutaj zostac rozwiazana
		 *
		 * Zatem nalezy zwrocic dopasowanie puste
		 */
		if(path.size()==0) {
			return NodeVec(1,base);
		}
		/* przypadek select="/para" lecz element glowny dokumentu ma inna nazwe
		 * nie porownujemy nazw lecz odwolujemy sie do funkcji,
		 * bo wezel w sciezce moze byc reprezentowany jako wildcard
		 *
		 * rowniez nie ma dopasowania
		 */
		else if(!path.at(0).matchElementNodeName(base->getName().string()) && isDescStep(0)) {
			return NodeVec();
		}
		/* przypadek: select="//para..."
		 * zaraz na poczatku wywolany zostaje modyfikator descendents-or-self (d-o-s)
		 * co oznacza ze dopasowania bedziemy szukac wsrod wszystkich zstepnych korzenia
		 *
		 * zatem dodajemy wszyskich zstepnych do zbioru dopasowan,
		 * nastepnie obcinamy te dla ktorych para nie uzyskalo dopasowania
		 *
		 * dalsza obsluga odbywa sie juz w petli
		 */
		if(isDescStep(0)) {
			matchedSoFar.push_back(contextNode->getRoot());
			matchedSoFar = descOrSelf(matchedSoFar);
			matchedSoFar = path.at(0).selectChildren(matchedSoFar);
			depth = 1;
		}
		/*przypadek: select = "/para"
		 * wszystko gra ustawiamy dopasowanie na wezel glowny dokumentu
		 * i kontynuujemy w petli		 *
		 */
		else {
			matchedSoFar.push_back(contextNode->getRoot());
			depth = 1;
		}
	}
	while(depth<path.size()) {
		//sprawdzamy czy nastepuje modyfikator zasiegu d-o-s
		if(isDescStep(depth))
			matchedSoFar = descOrSelf(matchedSoFar);
		//wykorzystujemy dotychczasowe dopasowania do znalezienia nowych, i usuniecia tych
		//ktore prowadzily do nikad
		matchedSoFar = path.at(depth).selectChildren(matchedSoFar);
		++depth;
	}

	matchedSoFar = pruneAndSort(matchedSoFar);

	return matchedSoFar;
}

//sprawdza czy krok ma zaglebiac sie do wszystkich zstepnych (true) czy tylko do dzieci (false)
bool XPath::isDescStep(int depth) const {
	//moge skorzystac z binarySearch bo wektor jest posortowany
	return std::binary_search(descendantOrSelfLevel.begin(),descendantOrSelfLevel.end(),depth);
}

NodeVec XPath::descOrSelf(const NodeVec& base) const {
	NodeVec ret;
	for(NodeVec::const_iterator it =base.begin();it!=base.end();++it) {
		NodeVec temp = (*it)->getDescendantsOrSelf();
		ret.insert(ret.end(),temp.begin(),temp.end());
	}

	ret = pruneAndSort(ret);

	//w funkcji sa duplikaty jesli jakiekolwiek dwa wezly z wektora base
	//sa z soba w relacji przodek-potomek
	//natomiast jesli wektor base jest posortowany zgodnie z porzadkiem dokumentu i
	//nie znajduja sie w nim elementy takie jw. to porzadek zostanie zachowany
	return ret;
}

//argument przyjmowany to wektor wezlow dla ktorych wykonamy nastepny krok po sciezce
NodeVec XPathNode::selectChildren(const NodeVec& base)const {
	NodeVec ret;
	for(NodeVec::const_iterator it =base.begin();it!=base.end();++it) {
		NodeVec temp;
		switch(type) {
		case SIMPLET: 	temp = (*it)->getChildrenByName(name);
		break;
		case CURRENTT:	temp = NodeVec(1,(*it));
		break;
		case PARENTT:	temp = NodeVec(1,(*it)->getParent());
		break;
		case ANYT:		temp = (*it)->getAllElementChildren();
		break;
		case ATTRT:		temp = (*it)->getAttrByName(name);
		break;
		case ANYATTRT:	temp = (*it)->getAllAttrs();
		break;
		default:		break;
		}
		ret.insert(ret.end(),temp.begin(),temp.end());
	}
	//TODO tutaj tez byc moze pruning


	//jedyne co moze zaburzyc porzadek dokumentu w zwracanym wektorze jest przypadek '..' ojcostwa

	return ret;
}

bool XPathNode::matchElementNodeName(const String &n) const{
	if(type==SIMPLET && name == n)
		return true;
	if(type==ANYT)
		return true;
	return false;
}

void XPathNode::print() const {
	switch(type) {
	case SIMPLET: 	std::cout << name;
	break;
	case CURRENTT:	std::cout << '.';
	break;
	case PARENTT:	std::cout << "..";
	break;
	case ANYT:		std::cout << "*";
	break;
	case ATTRT:		std::cout << '@' << name;
	break;
	case ANYATTRT:	std::cout << "@*";
	break;
	default:		break;
	}
}


//wykorzystywany do konstrukcji sciezki
void XPath::append(XPathNode n) {
	path.push_back(n);
}

//zaznacza miejsca w ktorych zastosowano kwalifikator ALL
void XPath::branchToAllDescendants() {
	descendantOrSelfLevel.push_back(path.size());
}

NodeVec XPath::pruneAndSort(const NodeVec &v) const {
	NodeVec ret;
	ret.assign(v.begin(),v.end());
	DocumentOrderComparator comp;
	std::sort(ret.begin(),ret.end(),comp);

	NodeVec::iterator it;
	it = std::unique(ret.begin(),ret.end());
	ret.resize(it-ret.begin());
	return ret;
}


}


