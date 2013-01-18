/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XSLTStructs.cpp
 *
 * Autor: Mateusz Zak
 */

#include "XSLTStructs.h"



//kolejnosc slow kluczowych zostala ustalona eksperymentalnie
KeywordMapping keywords[KEYWORDSNUM] = {
		{"xsl:when", WHEN},
		{"xsl:if", IFCLAUSE},
		{"xsl:choose", CHOOSE},
		{"xsl:sort", SORT},
		{"xsl:stylesheet", STYLESHEET},
		{"priority", PRIORITY},
		{"name", TEMPLATENAME},
		{"xsl:template", TEMPLATE},
		{"xsl:for-each", FOREACH},
		{"select", SELECT},
		{"data-type", DATATYPE},
		{"order", ORDER},
		{"match", MATCH},
		{"xsl:apply-templates", APPLYTEMPLATES},
		{"test", TEST},
		{"xsl:value-of", VALUEOF},
		{"xsl:otherwise", OTHERWISE},
		{"text",TEXT},
		{"number", NUMBER},
		{"ascending", ASC},
		{"descending", DESC}
};

/*
Token tryToMatchKeyword(const String & sequence) {
	int hash = calculateHash(sequence);
	if (keywords[hash].key == sequence)
		return Token(keywords[hash].symbol, sequence);
	else return Token(ID, sequence);
}

int calculateHash(const String & sequence) const {
	int i;
	//udalo sie znalezc funkcje hasujaca zwracajaca 2 kolizje dla tablicy o rozmiarze 17
	//167, 179
	// if len(s)<7: i =0
    //else: i = 4
    //return (ord(s[i])*first + ord(s[i+1])*second + ord(s[i+2])) % 17
	if(sequence.size()<3)
		return 0;
	if(sequence.size()<7)
		i = 0;
	else
		i=4;
	int ret = (sequence.at(i)*167+sequence.at(i+1)*179 + sequence.at(i+2)) % 17;

	if(ret==13 && sequence.at(0)=='t')//przypadek test
			return 14;
	if(ret==16 && sequence.at(0)=='m')//przypadek match
			return 12;
	return ret;
}*/

//funkcja sluzy do glebokiego kopiowania kontenerow zawierajacych polimorficzne wskazniki
template<typename T>
void deepCopy(const std::vector<T*> & from, std::vector<T*> & to) {
	to = std::vector<T*>(from.size(),NULL);
	std::transform(from.begin(),from.end(),to.begin(),std::mem_fun(&T::clone));
}


XSLTTemplate::XSLTTemplate(LocExpr * match, String str, double p, InstructionVec l):
										pattern(match), name(str), priority(p), instructions(l) {}

XSLTTemplate::XSLTTemplate(const XSLTTemplate & rhs) {
	priority = rhs.priority;
	name = rhs.name;
	if(rhs.pattern)
		pattern = new LocExpr(*rhs.pattern);
	else
		pattern = NULL;
	instructions = InstructionVec();
	deepCopy(rhs.instructions, instructions);
}

XSLTTemplate::~XSLTTemplate() {
	if(pattern)
		delete pattern;
	std::for_each(instructions.begin(),instructions.end(),myDel());
}

void XSLTTemplate::print() {
	std::cout << "TEMPLATE: priority: " << priority << " name: " << name
			<< " pattern: "; pattern->print();
			pattern->print();
	for(InstructionVec::iterator it = instructions.begin();it!=instructions.end();++it) {
		(*it)->print(1);
		std::cout << '\n';
	}
}

//sprawdza czy sciezka pasuje do wezla
bool XSLTTemplate::isMatching(Node * n) const {
	return pattern->match(n);
}

XSLTStylesheet::XSLTStylesheet(const std::vector<XSLTTemplate *>& t): templates(t) {
	InstructionVec l;
	l.push_back(new XSLApplyTemplates());
	defTemplate = new XSLTTemplate(NULL,"defTemplate",-0.5,l);
}

XSLTStylesheet::~XSLTStylesheet() {
	if(defTemplate)
		delete defTemplate;
	std::for_each(templates.begin(),templates.end(),myDel());
}

void XSLTStylesheet::print() {
	for(TemplateVec::iterator it = templates.begin(); it!=templates.end();it++) {
		(*it)->print();
	}
}

//funkcja wybiera 'najlepiej' pasujacy wzorzec, tj taki o najwyzszym priorytecie, w przypadku rownego bierze ostatni
//napotkany
XSLTTemplate * XSLTStylesheet::findBestFittingTemplate(Node * contextNode) const {
	XSLTTemplate * ret = NULL;

	for(TemplateVec::const_iterator it = templates.begin(); it!=templates.end(); it++) {
		if((*it)->isMatching(contextNode)) {
			if(ret==NULL)
				ret = *it;
			else if(ret->getPriority()<=(*it)->getPriority()){
				ret = *it;
			}
		}
	}
	if(ret==NULL) {//jesli nie znaleziono zadnego szablonu pasujacego do kontekstu wezla, to wywolujemy wzorzec domyslny
		ret = getDefaultTemplate();
	}
	return ret;
}

XSLTTemplate * XSLTStylesheet::getDefaultTemplate() const{
	return defTemplate;
}

void XSLApplyTemplates::print(int d) {
	indent(d); std::cout << "APPLY "; selected->print();
}

XSLApplyTemplates::XSLApplyTemplates(const XSLApplyTemplates& rhs) {
	if(rhs.selected)
		selected = new LocExpr(*rhs.selected);
	else
		selected = NULL;
	deepCopy(rhs.sorts,sorts);
}
XSLApplyTemplates::~XSLApplyTemplates() {
	if(selected)
		delete selected;
	std::for_each(sorts.begin(),sorts.end(),myDel());
}

void XSLBranch::print(int d) {
	indent(d); std::cout <<"choose " << (isDefaultDefined?"tak":"nie") << '\n';
	for(ConditionalVec::iterator it = conditionalBranches.begin();it!=conditionalBranches.end();++it) {
		(*it)->print(d+1);
		std::cout << '\n';
	}
	std::cout << "otherwise:\n";
	for(InstructionVec::iterator it = defBranch.begin();it!=defBranch.end();++it) {
		(*it)->print(d+1);
		std::cout << '\n';
	}
}

XSLBranch::XSLBranch(ConditionalVec cB, InstructionVec dB):
	conditionalBranches(cB), defBranch(dB) {
	isDefaultDefined = (dB.size()!=0);
}

XSLBranch::XSLBranch(const XSLBranch& rhs) {
	isDefaultDefined = rhs.isDefaultDefined;
	deepCopy(rhs.conditionalBranches, conditionalBranches);
	deepCopy(rhs.defBranch, defBranch);
}

XSLBranch::~XSLBranch() {
	std::for_each(conditionalBranches.begin(),conditionalBranches.end(),myDel());
	std::for_each(defBranch.begin(),defBranch.end(),myDel());
}

void XSLConditional::print(int d) {
	indent(d); std::cout <<"CONDITIONAL" << expression << '\n';
	//expression->print();
	for(InstructionVec::iterator it = instructions.begin();it!=instructions.end();++it) {
		(*it)->print(d+1);
		std::cout << '\n';
	}
}

XSLConditional::XSLConditional(const XSLConditional& rhs) {
	if(rhs.expression)
		expression = new XPathExpr(*rhs.expression);
	else
		expression = NULL;
	deepCopy(rhs.instructions,instructions);
}

XSLConditional::~XSLConditional() {
	if(expression)
		delete expression;
	std::for_each(instructions.begin(),instructions.end(),myDel());
}

void XSLComplex::print(int d) {
	indent(d); std::cout <<"COMPLEX " << name.string() << '\n';
	for(InstructionVec::iterator it = children.begin();it!=children.end();++it) {
		(*it)->print(d+1);
//std::cout << '\n';
	}
}
XSLComplex::XSLComplex(const XSLComplex& rhs) {
	name = rhs.name;
	deepCopy(rhs.attrs,attrs);
	deepCopy(rhs.children,children);
}

XSLComplex::~XSLComplex() {
	std::for_each(attrs.begin(),attrs.end(),myDel());
	std::for_each(children.begin(),children.end(),myDel());

}

void XSLRepetition::print(int d) {
	indent(d); std::cout <<"REPETITION "; selected->print(); std::cout << '\n';
	for(InstructionVec::iterator it = body.begin(); it!=body.end(); ++it) {
		(*it)->print(d+1);
		//std::cout << '\n';
	}
}

XSLRepetition::XSLRepetition(const XSLRepetition& rhs) {
	if(rhs.selected)
		selected = new LocExpr(*rhs.selected);
	else
		selected = NULL;

	deepCopy(rhs.sorts,sorts);
	deepCopy(rhs.body,body);
}

XSLRepetition::~XSLRepetition() {
	std::for_each(body.begin(),body.end(),myDel());
	std::for_each(sorts.begin(),sorts.end(),myDel());
	if(selected)
		delete selected;
}

void XSLSort::print(int d) {
	indent(d); std::cout << "SORT "<< criterion << '\n';
}

XSLSort::XSLSort(const XSLSort& rhs) {
	order = rhs.order;
	type = rhs.type;
	if(rhs.criterion)
		criterion = new XPathExpr(*rhs.criterion);
	else
		criterion = NULL;
}

XSLSort::~XSLSort() {
	if(criterion)
		delete criterion;
}

void XSLText::print(int d) {
	indent(d); std::cout << "TEXT "<< text << '\n';
}

void XSLValueOf::print(int d) {
	indent(d); std::cout << "VALUE-OF " << selected << '\n';
}

XSLValueOf::XSLValueOf(const XSLValueOf& rhs) {
	if(rhs.selected)
		selected = new XPathExpr(*rhs.selected);
	else selected = NULL;
}

//apply templates tworzy nowy kontekst na podstawie wyrazenia selected.
//Domyslnie na nowej liscie wezlow oczekujacych na przetworzenie laduja wszystkie dzieci biezacego wezla
//Po obliczeniu listy wezlow rozpoczyna sie przetwarzanie nowego kontekstu
NodeVec XSLApplyTemplates::evaluate(const Context& context)const{
	NodeVec chosen;
	if(selected)
		chosen = context.select(selected);
	else {
		chosen = context.getCurrent()->getAllChildren();
	}
	Context newContext(chosen,context.getStylesheet());
	newContext.sort(sorts);
	return newContext.proccess();
}

//ewaluacja choose polega na sprawdzaniu warunkow kolejnych instrukcji when
//po natrafieniu na warunek prawdziwy wykonane zostaja instrukcje z ciala when
//a nastepne warunki nie sa juz sprawdzane
//jesli w choose wystepuje instrukcja otherwise to wywolujemy ja w sytuacji gdy zaden z warunkow when
//nie zostal dopasowany
NodeVec XSLBranch::evaluate(const Context& context) const {
	for(unsigned i=0;i<conditionalBranches.size();++i) {
		if(context.test(conditionalBranches.at(i)->getExpression())) {
			return context.instantiate(conditionalBranches.at(i)->getBody());
		}
	}
	if(isDefaultDefined)
		return context.instantiate(defBranch);
	return NodeVec();
}

//ewaluacja instrukcji if-clause polega na sprawdzeniu warunku i jesli jest prawdziwy wykonaniu jej ciala
NodeVec XSLConditional::evaluate(const Context& context) const {
	if(context.test(expression)) {
		return context.instantiate(instructions);
	}
	else
		return NodeVec();
}

//elementy neutralne przepisywane sa bez zmian, natomiast, instrukcje sa ewaluowane
NodeVec XSLComplex::evaluate(const Context& context)const{
	NodeVec retAttrs;
	deepCopy(attrs,retAttrs);

	NodeVec retChildren = context.instantiate(children);

	NodeVec result;
	result.push_back(new ElementNode(this->name, retAttrs, retChildren));
	return result;
}

//najpierw oblicza zbior nowych wezlow kontekstu, sortuje je zgodnie z zadanymi instrukcjami sort
//a nastepnie wywoluje cialo instrukcji for-each kolejno dla kazdego z nich
NodeVec XSLRepetition::evaluate(const Context & context)const{
	NodeVec chosen = context.select(selected);
	Context newContext(chosen, context.getStylesheet());
	newContext.sort(sorts);
	return newContext.instantiateForAll(body);
}


//zwraca posortowana liste wezlow kontekstu
NodeVec XSLSort::evaluate(const Context & context)const{

	NodeVec vec2sort = context.getCurrentList();
	Context temp(vec2sort,context.getStylesheet());

	std::vector<String> keys = extractKeys(temp);

	//sortujemy wskazniki do wezlow, ale porownujemy klucze, dlatego opakowujemy te pare w strukture
	std::vector<SortWrapper> wrappedVec;
	for(unsigned i = 0; i< vec2sort.size(); ++i)
		wrappedVec.push_back(SortWrapper(keys.at(i),vec2sort.at(i)));

	XSLSort::Comparator c(this);

	/*
	 * stable_sort zapewnia stalosc sortowania,
	 * tzn. kolejnosc elementow o rownych kluczach nie ulegnie zmianie
	 * wlasnosc ta pozwala sortowac po kilku kluczach jednoczesnie
	 */
	std::stable_sort(wrappedVec.begin(),wrappedVec.end(),c);

	NodeVec result;
	for(unsigned i = 0; i< vec2sort.size(); ++i)
		result.push_back(wrappedVec.at(i).n);

	return result;
}

//Comparator jest funktorem binarnym, korzystam z niego przy sortowaniu, jako 3 argumentu do stable_sort
bool XSLSort::Comparator::operator()(const XSLSort::SortWrapper& l, const XSLSort::SortWrapper& r) const {
	String a = l.val;
	String b = r.val;

	return sort->compare(a,b);
}

//oblicza klucze sortowania dla poszczegolnych wezlow, ktore chcemy posortowac
std::vector<String> XSLSort::extractKeys(Context & c)const {

	std::vector<String> ret;

	/*
	 * jesli parametr select sie nie pojawil to domyslnie stosowany jest '.',
	 * czyli jako klucz stosujemy string-value biezacego kontekstu
	 */
	if(criterion == NULL) {
		for(int i=0;i<c.getSize();++i) {
			c.setPosition(i);
			ret.push_back(c.getCurrent()->string());
		}
	}
	/*
	 * w przeciwnym wypadku obliczamy string-value na podstawie parametru
	 */
	else {
		for(int i=0;i<c.getSize();++i) {
			c.setPosition(i);
			ret.push_back(criterion->string(c.getCurrent()));
		}
	}
	c.setPosition(0);
	return ret;
}

//funkcja porownuje dwa klucze, wykorzystujac przy tym parametry order i data-type
bool XSLSort::compare(const String & a, const String &b)const {
	if(order == ASC && type == TEXT) {
		return a.compare(b)<0;
	}
	else if((order == DESC) && (type==TEXT)) {
		return b < a;
	}
	else if((order == ASC) && (type == NUMBER)) {
		return getNumericValue(a) < getNumericValue(b);
	}
	else {
		return getNumericValue(b) < getNumericValue(a);
	}
}

//czysty tekst z arkusza stylow jest przenoszony do drzewa wynikowego bez zmian
NodeVec XSLText::evaluate(const Context & context) const {
	NodeVec ret;

	Node * n = new TextNode(this->text);
	ret.push_back(n);
	return ret;
}

NodeVec XSLValueOf::evaluate(const Context & context) const{
	/*
	 * select ewaluuje wyrazenie i oblicza jego string-value
	 */
	String result = selected->string(context.getCurrent());
	if(result.size()==0)
		return NodeVec();

	NodeVec ret;
	ret.push_back(new TextNode(result));
	return ret;
}



Node * Context::getCurrent() const {
	return currentList.at(position);
}
int Context::getPosition() const {
	return position;
}
int Context::getSize() const {
	return currentList.size();
}
NodeVec Context::getCurrentList() const {
	return currentList;
}

//funkcja oblicza zbior wezlow do ktorych mozna dotrzec z wezla biezacego kontekstu
//podazajac sciezka s
NodeVec Context::select (LocExpr * s) const {
	return s->evaluate(getCurrent());
}

//funkcja oblicza wartosc boolowska wyrazenia majac na uwadze biezacy kontekst
bool Context::test (XPathExpr * e) const {
	return e->boolean(getCurrent());
}

//funkcja sortuje zbior wezlow zgodnie z wytycznymi dostarczonymi przez wektor instrukcji typu XSLSort
void Context::	sort(const SortVec& sorts) {
	//uzywam stable_sort i przechodze przez sorts od konca
	//co pozwala sortowac zbior wezlow po kilku kluczach np. najpierw first, potem last name
	for(SortVec::const_reverse_iterator rit = sorts.rbegin(); rit<sorts.rend(); rit++) {
		NodeVec temp = (*rit)->evaluate(*this);
		setCurrentList(temp);
	}
}

//funkcja wykonuje sekwencje instrukcji v, dla biezacego kontekstu
NodeVec Context::instantiate(const InstructionVec & v) const {
	NodeVec ret;
	for(InstructionVec::const_iterator it = v.begin(); it != v.end(); it++) {
		NodeVec temp = (*it)->evaluate(*this);
		ret.insert(ret.end(),temp.begin(),temp.end());
	}
	return ret;
}

//funkcja wykorzystywana przez for-each,
//wykonuje sekvencje instrukcji v dla kazdego wezla z biezacej listy
NodeVec Context::instantiateForAll(const InstructionVec &v) {
	std::cout << "wypisuje nowy template";
	for(InstructionVec::const_iterator it = v.begin();it!=v.end();++it) {
		(*it)->print(0);
	}
	NodeVec ret;
	position = 0;
	while (position<currentList.size()) {

		NodeVec temp = instantiate(v);
		std::cout << "wypisuje temp aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa " << temp.size();
//		for(NodeVec::iterator it = temp.begin();it!=temp.end();++it) {
//				(*it)->print(0,std::cout);
//		}
		ret.insert(ret.end(),temp.begin(),temp.end());
		position++;
	}
	return ret;
}

//funkcja tworzy fragment drzewa wynikowego poprzez dopasowanie najlepszego wzorca,
//do kolejnych wezlow z biezacego kontekstu
//i wywolanie instrukcji z jego ciala
NodeVec Context::proccess() {
	NodeVec resultTree;

	while(position<getSize()) {
		XSLTTemplate * t = stylesheet->findBestFittingTemplate(getCurrent());
		NodeVec temp = instantiate(t->getBody());

		resultTree.insert(resultTree.end(),temp.begin(),temp.end());
		++position;
	}
	return resultTree;
}


