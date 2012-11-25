/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XPathParser.cpp
 *
 * Autor: Mateusz Zak
 */


#include "XSLTParser.h"
#include "XPathParser.h"

namespace parsingXSLT {


char const * xsltTokDesc[MAXSYM] = {
		"<", "</" , "<?", "comment", ">", "/>", "?>", "=", ":", "ident", "literal",
		"ident2", "whitespace", "unknown",
		"xsl:stylesheet", "xsl:template","xsl:apply-templates", "xsl:value-of",
		"xsl:for-each", "xsl:if", "xsl:choose", "xsl:when", "xsl:otherwise", "xsl:sort"
		"name", "match", "priority", "select", "order", "data-type", "test"
};


XSLTParser::XSLTParser(ILexer * l): parsingXML::XMLParser(l) {
	tokenDescriptions = xsltTokDesc;
}

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


String XSLTParser::matchNamedAttr(const String & name, TokSymbol s_) {
	String ret;
	if(awaiting.lexeme!=name) {
		semanticError(String("Oczekiwano: ") + name + " a natrafiono na: " + awaiting.lexeme);
	}
	accept(s_);
	optional(WS);
	accept(EQUALOP);
	optional(WS);
	accept(LITERAL);
	ret = accepted.lexeme;
	optional(WS);
	return ret;
}

LocExpr * XSLTParser::matchTemplatePattern() {
	String pattern = matchNamedAttr("match", MATCH);
	return parseXPath(pattern);
}

LocExpr * XSLTParser::matchSelectNodeSet() {
	String pattern = matchNamedAttr("select",SELECT);
	return parseXPath(pattern);
}


XPathExpr * XSLTParser::matchSelectExpression() {
	String pattern = matchNamedAttr("select",SELECT);

	return parseXPathExpr(pattern);
}

XPathExpr * XSLTParser::matchCondition() {
	String expr = matchNamedAttr("test",TEST);
	//check if expr is proper

	return parseXPathExpr(expr);
}

LocExpr * XSLTParser::parseXPath(const String& sequence) {
	ISource * src = new StringSource(sequence);
	ILexer * lex = new parsingXPath::XPathLexer(src);
	LocExpr * pe = parsingXPath::XPathParser(lex).Path();
	if(src->countErrors() > 0) {
		Error("Blad parsera XPath:");
		((StringSource*)src)->printErrors();
	}
	delete lex;
	delete src;

	return pe;
}

XPathExpr * XSLTParser::parseXPathExpr(const String& sequence) {
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

// 'order=' '"' (ASCENDING | DESCENDING) '"'
OrderVal XSLTParser::matchOrderAttr() {
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
DataType XSLTParser::matchDataTypeAttr() {
	String t = matchNamedAttr("data-type", DATATYPE);
	if(t=="text")
		return TEXT;
	else if(t=="number")
		return NUMBER;
	else {
		semanticError("Nieobslugiwany typ danych");
		//zamias tego zwraca wartosc domyslna
		return TEXT;
	}
}

String XSLTParser::matchVersion() {
	return matchNamedAttr("version", ID);
}

ParsedObject * XSLTParser::startParsing() {
	getNextToken();
	XSLTStylesheet * set = Document();
	return set;
}

// Document -> Prolog {Comment} Stylesheet
XSLTStylesheet * XSLTParser::Document() {
	XMLParser::Prolog();
	XMLParser::optionalComment();
	return Stylesheet();
}

// Stylesheet -> '<' STYLESHEET Version { Attribute } '>' { Comment } { Template { Comment} }'</' STYLESHEET '>'
XSLTStylesheet * XSLTParser::Stylesheet() {
	matchXSLKeyword("stylesheet");
	matchAttribute("version");

	TemplateVec ret;
	for(Node c: all_children)
		ret.push_back(Template());

	return new XSLTStylesheet(ret);
}

// Template -> '<' TEMPLATE (MATCH | NAME) '=' LITERAL ('priority' '=' NUMBER)? '>' Content '</ TEMPLATE '>'
XSLTTemplate * XSLTParser::Template() {
	String str = "";
	LocExpr * ns = NULL;
	int priority = 0;


	matchXSLKeyword("template");
	String pattern = matchAttribute("match");
	if (pattern != "")
		ns = parseXPath(pattern);
	else if (matchAttribute("name")!="")
	{}
	else {
		syntaxError("Nieoczekiwany symbol: ", symbol);
		return 0;
	}
	String priority_str = matchAttribute("priority");
	priority = getNumericVal(priority_str);
	InstructionVec l = Content();
	return new XSLTTemplate(ns, str, priority, l);
}


// Content -> { Comment | '<' TopLevelInstruction | Text }
InstructionVec XSLTParser::Content() {
	InstructionVec ret;

	for(child c: all_children) {
		if(c.isXSLInstruction())
			ret.push_back(TopLevelInstruction());
		else
			ret.push_back(Text());
	}
	return ret;
}


XSLText * XSLTParser::Text() {
	String s = node.value();
	return new XSLText(s);
}

// TLI -> ApplyTemplates | If | Choose | ForEach | ValueOf
Instruction * XSLTParser::TopLevelInstruction() {
	TokSymbol kw = matchXSLKeyword();
	switch(kw) {
	case APPLYTEMPLATES: 	return ApplyTemplates(); break;
	case IFCLAUSE:			return IfClause(); break;
	case CHOOSE:			return Choose(); break;
	case FOREACH:			return ForEach(); break;
	case VALUEOF:			return ValueOf(); break;
	default:				return ComplexInstruction();//instruction with hierarchy
	}
}

//ApplyTemplates -> APPLYTEMPLATES (SELECT '=' '"' Path '"')? ( '/>' | '>' {'<' Sort} '</' APPLYTEMPLATES '>' )
XSLApplyTemplates * XSLTParser::ApplyTemplates() {
	matchXSLKeyword("apply-templates");
	LocExpr * selected = NULL;
	SortVec sorts;

	String selected_str = matchAttribute("select");
	if (selected_str!="")
		selected = parseXPath(selected_str);
	if (no_children_at_all)
		return new XSLApplyTemplates(selected);
	else {
		while(matchXSLKeyword("sort")!=0) {
			matchSorts();
			sorts.push_back(Sort());//w petli
		}
		//TODO jesli nie sort to syntax_error
				//syntaxError("Nieobslugiwany typ instrukcji", symbol);
				//return new XSLApplyTemplates(selected);
		return new XSLApplyTemplates(selected,sorts);
	}
}
// Sort -> SORT (SelectE)? ( Order | Datatype )? '/>'
XSLSort * XSLTParser::Sort() {
	matchXSLKeyword("sort");
	XPathExpr * selected = NULL;
	String selected_str = matchAttribute("select");
	if (selected_str!="")
		selected = parseXPath(selected_str);

	OrderVal order = ASCENDING;
	DataType type = TEXT;
	//TODO dwa atrybuty w dowolnej kolejności, zaden inny
	matchAttribute("order");
	matchAttribute("data-type");
	//		syntaxError("Nieoczekiwany symbol: ", symbol);
	//return 0;

	return new XSLSort(selected,order,type);
}

// If -> IF TestE '>' Content '</' IF '>'
XSLConditional * XSLTParser::IfClause() {
	matchXSLKeyword("if");
	XPathExpr* expr = matchCondition();

	InstructionVec body = Content();

	return new XSLConditional(expr, body);
}

// Choose -> CHOOSE '>' {'<' When } ('<' Otherwise)? '</' CHOOSE '>'
XSLBranch * XSLTParser::Choose() {
	ConditionalVec conds;
	InstructionVec otherwise;

	matchXSLKeyword("choose");
	for(child c: all_children) {
		if(c.matchXSLKeyword("when")!=0)
			conds.push_back(When());
		else {
			otherwise = Otherwise();
			return new XSLBranch(conds, otherwise);
		}
	}
	return new XSLBranch(conds);
}

//When -> WHEN TestE '>' Content '</' WHEN '>'
XSLConditional * XSLTParser::When() {
	matchXSLKeyword("when");
	XPathExpr * expr = matchCondition();
	InstructionVec body = Content();
	return new XSLConditional(expr, body);
}

// Otherwise -> OTHERWISE '>' Content '</' OTHERWISE '>'
InstructionVec XSLTParser::Otherwise() {
	matchXSLKeyword("otherwise");
	InstructionVec ret = Content();
	return ret;
}

//ForEach -> FOREACH SelectNS '>' ForEachContent '</' FOREACH '>'
XSLRepetition * XSLTParser::ForEach() {
	matchXSLKeyword("for-each");
	LocExpr * xPath = matchSelectNodeSet();
	std::pair<SortVec, InstructionVec> vecs = ForEachContent();
	return new XSLRepetition(xPath, vecs.first, vecs.second);
}

//FEC -> { '<' Sort | Comment } Content
std::pair<SortVec, InstructionVec> XSLTParser::ForEachContent() {
	std::pair<SortVec, InstructionVec> ret;
	bool endOfSorts = false;
	while(still_has_children && !endOfSorts) {
		//TODO obsluz instrukcje wezlow potomnych najpierw sorty, pozniej dowolne
		if(c.matchXSLKeyword("sort")==0)//czyli wreszcie nie sort
			break;
		ret.first.push_back(Sort());
		//po drodze nalezy ignorowac komenty
		//TODO zastanowic się gdzie tu miejsce dla Text();
	}
	//nastepnie dopasowac pozostale instrukcje
	InstructionVec ret = Content();

	return ret;
}

// ValueOf -> VALUEOF SelectE '/>'
XSLValueOf * XSLTParser::ValueOf() {
	matchXSLKeyword("value-of");
	XPathExpr * xPath = matchSelectExpression();
	return new XSLValueOf(xPath);
}

//Complex -> Name { Attribute } ( '/>' | '>' Content '</' Name '>')
XSLComplex * XSLTParser::ComplexInstruction() {
	//TODO ewentualnie sprawdzic czy nie ma slow zakazanych, wpp przepisac bez zmian
	//accept(OPENTAG);
	if(no_children_at_all)
		return new XSLComplex(node.name,node.attrs);
	else {
		InstructionVec content = Content();
		return new XSLComplex(node.name,node.attrs,content);
	}
}


}

