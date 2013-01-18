/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XPathParser.cpp
 *
 * Autor: Mateusz Zak
 */


#include "XSLTAnalyzer.h"
#include "XPathParser.h"


char const * xslTokDesc[MAXSYM] = {
		"xsl:stylesheet", "xsl:template","xsl:apply-templates", "xsl:value-of",
		"xsl:for-each", "xsl:if", "xsl:choose", "xsl:when", "xsl:otherwise", "xsl:sort",
		"name", "match", "priority", "select", "order", "data-type", "test", "text",
		"number", "ascending", "descending"
};



int getNumericVal(const String & s) {
	int tmp = 0;
	unsigned i = 0;
	bool m = false;
	if(s[0] == '-') {
		m = true;
		i++;
	}
	for(; i < s.size(); i++)
		tmp = 10 * tmp + s[i] - '0';
	return m ? -tmp : tmp;
}

/*
String XSLTAnalyzer::matchAttribute(const parsingXML::Name& n) {
	//TODO error handling
	return node->getAttrByName(n.string());
}

String XSLTAnalyzer::matchAttribute(const String& n) {
	//TODO error handling
	return node->getAttrByName(n);
}*/

bool XSLTAnalyzer::validateName(const Node * n, XSLSymbol t) {
	if ( n->getName().string() != xslTokDesc[t]) {
		xsltError("Blad skladniowy XSL: "+ t, n);
		return false;
	}
	return true;
}

XSLSymbol XSLTAnalyzer::matchXSLKeyword(const Name& n) {
	//TODO
	std::cout << "cycki" << n.string() << '\n';
	for (int i = 0; i<KEYWORDSNUM;i++) {
		if (keywords[i].key==n.string())
			return keywords[i].symbol;
	}
	return MAXSYM;
}

void XSLTAnalyzer::xsltError(const String& msg, const Node * n) const {
	std::cout << msg << ' ';
	n->string();
}

String XSLTAnalyzer::requiredAttribute(const Node * n, XSLSymbol t) {
	NodeVec result = n->getAttrByName(xslTokDesc[t]);
	if (result.size()!=1) {
		//TODO syntax error
		xsltError(String("Blad skladniowy XSL, wymagany atrybut: ") + xslTokDesc[t], n);
		return String("");
	}
	return result[0]->string();
}

String XSLTAnalyzer::optionalAttribute(const Node *n, XSLSymbol t, String def_val) {
	std::cout << "optional: " << xslTokDesc[t] << ' ' << t;
	NodeVec result = n->getAttrByName(xslTokDesc[t]);
		if (result.size()==0) {
			std::cout << "domyslna: " << def_val << '\n';
			return def_val;
		}
		std::cout << result[0]->string();
		return result[0]->string();
	}



LocExpr * XSLTAnalyzer::parseXPath(const String& sequence) {
	ISource * src = new StringSource(sequence);
	ILexer * lex = new parsingXPath::XPathLexer(src);
	LocExpr * pe = parsingXPath::XPathParser(lex).Path();
	if(src->countErrors() > 0) {
		//TODO Error("Blad parsera XPath:");
		((StringSource*)src)->printErrors();
	}
	delete lex;
	delete src;

	return pe;
}

XPathExpr * XSLTAnalyzer::parseXPathExpr(const String& sequence) {
	ISource * src = new StringSource(sequence);
	ILexer * l = new parsingXPath::XPathLexer(src);

	XPathExpr * pe = parsingXPath::XPathParser(l).Expression(parsingXPath::MAXSYM);
	if(src->countErrors() > 0) {
		((StringSource*)src)->printErrors();
	}

	delete l;
	delete src;

	return pe;
}

/*
// 'order=' '"' (ASCENDING | DESCENDING) '"'
OrderVal XSLTAnalyzer::matchOrderAttr() {
	String o = matchNamedAttr("order",ORDER);
	if(o=="ascending")
		return ASCENDING;
	else if(o=="descending")
		return DESCENDING;
	else {
		semanticError("Nieobslugiwany porzadek sortowania");
		//zwrocona wartosc domyslna
		return ASCENDING;
	}
}

// 'data-type=' '"' (TEXT | NUMBER) '"'
DataType XSLTAnalyzer::matchDataTypeAttr() {
	String t = matchNamedAttr("data-type", DATATYPE);
	if(t=="text")
		return TEXT;
	else if(t=="number")
		return NUMBER;
	else {
		semanticError("Nieobslugiwany typ danych");
		//zwraca wartosc domyslna
		return TEXT;
	}
}
*/


// Document -> Prolog {Comment} Stylesheet
XSLTStylesheet * XSLTAnalyzer::Document(const Node * n) {

	return Stylesheet(n->getAllChildren()[0]);
}

// Stylesheet -> '<' STYLESHEET Version { Attribute } '>' { Comment } { Template { Comment} }'</' STYLESHEET '>'
XSLTStylesheet * XSLTAnalyzer::Stylesheet(const Node * n) {
	validateName(n,STYLESHEET);
	//TODO matchAttribute(parsingXML::Name("version"));

	NodeVec templates = n->getAllChildren();
	TemplateVec ret;

	for(NodeVec::iterator it = templates.begin();it != templates.end();++it)
		ret.push_back(Template(*it));

	return new XSLTStylesheet(ret);
}

// Template -> '<' TEMPLATE (MATCH | NAME) '=' LITERAL ('priority' '=' NUMBER)? '>' Content '</ TEMPLATE '>'
XSLTTemplate * XSLTAnalyzer::Template(const Node * n) {
	String str = "";
	LocExpr * ns = NULL;
	int priority = 0;

	validateName(n, TEMPLATE);
	String pattern = optionalAttribute(n, MATCH, "");
	if (pattern != "")
		ns = parseXPath(pattern);
	else if (optionalAttribute(n, TEMPLATENAME, "")!="")
	{}
	else {
		//TODO xsltError(przynajmniej jeden z argumentow musi zostac dopasowany);
		return NULL;
	}
	String priority_str = optionalAttribute(n,PRIORITY, "0.0");
	priority = getNumericVal(priority_str);
	InstructionVec l = Content(n);
	return new XSLTTemplate(ns, str, priority, l);
}



InstructionVec XSLTAnalyzer::Content(const Node * n) {
	NodeVec nodes = n->getAllChildren();
	InstructionVec ret;
//	InstructionVec ret(nodes.size(), NULL);

//	std::transform(nodes.begin(), nodes.end(),
//					ret.begin(), std::mem_fun(&XSLTAnalyzer::TLI));

	for(NodeVec::iterator it = nodes.begin();it != nodes.end();++it)
		ret.push_back(TLI(*it));


	//prune NULL pointers (generated by ignored comment nodes)
	//InstructionVec::iterator newEnd = std::remove_if(ret.begin(),ret.end(),isNull);
	//ret.resize(newEnd - ret.begin());

	return ret;
}


XSLText * XSLTAnalyzer::Text(const Node * n) {
	String s = n->string();
	return new XSLText(s);
}

// TLI -> ApplyTemplates | If | Choose | ForEach | ValueOf
Instruction * XSLTAnalyzer::TLI(const Node * n) {
	if (n->type() == TEXTTYPE)
		return Text(n);

	XSLSymbol kw = matchXSLKeyword(n->getName());
	switch(kw) {
	case APPLYTEMPLATES: 	return ApplyTemplates(n);
	case IFCLAUSE:			return IfClause(n);
	case CHOOSE:			return Choose(n);
	case FOREACH:			return ForEach(n);
	case VALUEOF:			return ValueOf(n);
	case SORT:				return Sort(n);
	default:				return ComplexInstruction(n);//instruction with hierarchy
	}
}

//ApplyTemplates -> APPLYTEMPLATES (SELECT '=' '"' Path '"')? ( '/>' | '>' {'<' Sort} '</' APPLYTEMPLATES '>' )
XSLApplyTemplates * XSLTAnalyzer::ApplyTemplates(const Node * n) {
	LocExpr * selected = NULL;
	SortVec sorts;

	String val = optionalAttribute(n,SELECT,"");
	//if (selected_str!="")
	selected = parseXPath(val);
	NodeVec nodes = n->getAllChildren();
	if (nodes.size() == 0)
		return new XSLApplyTemplates(selected);
	for (int i = 0; i<nodes.size();++i) {
		//TODO xsltError("Nieobslugiwany typ instrukcji", symbol);
		sorts.push_back(Sort(nodes[i]));
	}
	return new XSLApplyTemplates(selected,sorts);
}


// Sort -> SORT (SelectE)? ( Order | Datatype )? '/>'
XSLSort * XSLTAnalyzer::Sort(const Node * n) {
	validateName(n,SORT);

	String val = optionalAttribute(n, SELECT, ".");
	XPathExpr * selected = parseXPath(val);

	XSLSymbol order= matchXSLKeyword(Name(optionalAttribute(n, ORDER, "ascending")));
	XSLSymbol type = matchXSLKeyword(Name(optionalAttribute(n, DATATYPE, "text")));
	//TODO		xsltError("Nieoczekiwany symbol: ", symbol);
	//return 0;

	return new XSLSort(selected,order,type);
}

// If -> IF TestE '>' Content '</' IF '>'
XSLConditional * XSLTAnalyzer::IfClause(const Node * n) {
	//matchXSLKeyword("if");
	validateName(n,IFCLAUSE);
	String val = requiredAttribute(n, TEST);
	XPathExpr* expr = parseXPathExpr(val);

	InstructionVec body = Content(n);

	return new XSLConditional(expr, body);
}

// Choose -> CHOOSE '>' {'<' When } ('<' Otherwise)? '</' CHOOSE '>'
XSLBranch * XSLTAnalyzer::Choose(const Node * n) {
	ConditionalVec conds;
	InstructionVec otherwise;

	//matchXSLKeyword("choose");
	validateName(n, CHOOSE);

	NodeVec nodes = n->getAllChildren();
	for(int i = 0; i<nodes.size();++i) {
		if(validateName(nodes[i],WHEN))
			conds.push_back(When(nodes[i]));
		else if(i == nodes.size()-1 && validateName(nodes[i],OTHERWISE))
			otherwise = Otherwise(nodes[i]);
		else {
			//TODO syntax error niedozwolony element xsl
		}
	}

	return new XSLBranch(conds,otherwise);
}

//When -> WHEN TestE '>' Content '</' WHEN '>'
XSLConditional * XSLTAnalyzer::When(const Node * n) {
	validateName(n, WHEN);
	String val = requiredAttribute(n, TEST);
	XPathExpr * expr = parseXPathExpr(val);
	InstructionVec body = Content(n);
	return new XSLConditional(expr, body);
}

// Otherwise -> OTHERWISE '>' Content '</' OTHERWISE '>'
InstructionVec XSLTAnalyzer::Otherwise(const Node * n) {
	validateName(n, OTHERWISE);
	InstructionVec ret = Content(n);
	return ret;
}

bool isNotSort(const Instruction* i) {
	return i->type()!=SORT;
}

bool isSort(const Instruction* i) {
	return !isNotSort(i);
}

//ForEach -> FOREACH SelectNS '>' ForEachContent '</' FOREACH '>'
XSLRepetition * XSLTAnalyzer::ForEach(const Node * n) {
	validateName(n, FOREACH);
	String val = requiredAttribute(n, SELECT);
	LocExpr * xPath = parseXPath(val);

	NodeVec nodes = n->getAllChildren();

	InstructionVec list = Content(n);

	//znajduje pierwsza instrukcje nie bedaca sortem
	InstructionVec::iterator it = std::find_if(list.begin(),list.end(),isNotSort);

	int size = it - list.begin();
	SortVec sorts;//(size,NULL);
	//std::copy(list.begin(),it,sorts.begin());
	for(InstructionVec::iterator it2 = list.begin();it2!=it;++it2)
		sorts.push_back((XSLSort *)*it2);


	//sprawdza czy po napotkaniu takiej instrukcji pojawia sie jeszcze kolejny sort
	if(std::find_if(it,list.end(),isSort)!=list.end()) {
		//xsltError wszystkie sorty powinny pojawic sie przed innymi instrukcjami
	}
	size = list.end()-it;
	InstructionVec others(size, NULL);
	std::copy(it,list.end(),others.begin());


	return new XSLRepetition(xPath, sorts, others);
}

// ValueOf -> VALUEOF SelectE '/>'
XSLValueOf * XSLTAnalyzer::ValueOf(const Node * n) {
	validateName(n, VALUEOF);
	String val = requiredAttribute(n, SELECT);
	//XPathExpr * xPath = matchExpression("select");
	return new XSLValueOf(parseXPathExpr(val));
}

//Complex -> Name { Attribute } ( '/>' | '>' Content '</' Name '>')
XSLComplex * XSLTAnalyzer::ComplexInstruction(const Node * n) {
	//TODO ewentualnie sprawdzic czy nie ma slow zakazanych, wpp przepisac bez zmian
	InstructionVec content = Content(n);
	return new XSLComplex(n->getName(), n->getAllAttrs(), content);
}



