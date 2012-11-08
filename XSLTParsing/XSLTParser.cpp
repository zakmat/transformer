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

void XSLTParser::closeElement(XSLTSymbol s) {
	optional(WS);
	accept(OPENENDTAG);
	optional(WS);
	accept(s);
	optional(WS);
	accept(CLOSETAG);
	optional(WS);
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
	accept(OPENTAG);
	accept(STYLESHEET);
	optional(WS);
	matchVersion();
	while(symbol != CLOSETAG) {
		Attribute();
		optional(WS);
	}
	accept(CLOSETAG);
	XMLParser::optionalComment();

	std::vector<XSLTTemplate *> ret;

	while(symbol!=OPENENDTAG) {
		ret.push_back(Template());
		//ret.back()->print();
		XMLParser::optionalComment();
	}
	closeElement(STYLESHEET);
	return new XSLTStylesheet(ret);
}

// Template -> '<' TEMPLATE (MATCH | NAME) '=' LITERAL ('priority' '=' NUMBER)? '>' Content '</ TEMPLATE '>'
XSLTTemplate * XSLTParser::Template() {
	String str = "";
	LocExpr * ns = 0;
	int priority = 0;

	accept(OPENTAG);
	accept(TEMPLATE);
	optional(WS);
	switch(symbol) {
	case MATCH:
		ns = matchTemplatePattern();
		break;
	case TEMPLATENAME:
		str = matchNamedAttr("name", NAME);
		break;
	default:
		syntaxError("Nieoczekiwany symbol: ", symbol);
		return 0;
	}
	optional(WS);
	if(symbol==PRIORITY)
		priority = getNumericVal(matchNamedAttr("priority",PRIORITY));
	optional(WS);
	accept(CLOSETAG);
	optional(WS);
	InstructionVec l = Content();
	optional(WS);
	closeElement(TEMPLATE);
	return new XSLTTemplate(ns, str, priority, l);
}


// Content -> { Comment | '<' TopLevelInstruction | Text }
InstructionVec XSLTParser::Content() {
	InstructionVec ret;
	optional(WS);
	while(symbol!=OPENENDTAG) {
		switch(symbol) {
		case OPENTAG:	accept(OPENTAG);
						ret.push_back(TopLevelInstruction());
		break;
		case COMMENT:	Comment();
		break;
		default:		ret.push_back(Text());
		break;
		}
		optional(WS);
	}
	return ret;
}


String XSLTParser::Comment() {
	accept(COMMENT);
	return accepted.lexeme;
}

XSLText * XSLTParser::Text() {
	String s;
	while(symbol != OPENTAG && symbol != OPENENDTAG) {
		accept(symbol);
		s.append(accepted.lexeme);
	}
	if(accepted.symbol==WS)
		s.erase(s.size()-accepted.lexeme.size());
	return new XSLText(s);
}

// TLI -> ApplyTemplates | If | Choose | ForEach | ValueOf
Instruction * XSLTParser::TopLevelInstruction() {
	optional(WS);
	switch(symbol) {
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
	accept(APPLYTEMPLATES);
	optional(WS);
	LocExpr * selected = NULL;
	SortVec sorts;
	if(symbol==SELECT) {
		selected = matchSelectNodeSet();
		optional(WS);
	}
	if(symbol==ENDEMPTYELEM) {//zastosuj dla wszystkich dzieci
		accept(ENDEMPTYELEM);
		return new XSLApplyTemplates(selected);
	}
	else if(symbol==CLOSETAG) {
		accept(CLOSETAG);
		optional(WS);
		while(symbol!=OPENENDTAG) {
			accept(OPENTAG);
			optional(WS);
			sorts.push_back(Sort());
			optional(WS);
		}
		closeElement(APPLYTEMPLATES);
		return new XSLApplyTemplates(selected, sorts);
	}
	else {
		syntaxError("Nieobslugiwany typ instrukcji", symbol);
		return new XSLApplyTemplates(selected);
	}
}
// Sort -> SORT (SelectE)? ( Order | Datatype )? '/>'
XSLSort * XSLTParser::Sort() {
	optional(WS);
	accept(SORT);
	optional(WS);
	XPathExpr * selected = NULL;
	if(symbol==SELECT) {
		selected = matchSelectExpression();
		optional(WS);
	}
	OrderVal order = ASCENDING;
	DataType type = TEXT;
	optional(WS);
	while(symbol!=ENDEMPTYELEM) {
		switch(symbol) {
		case ORDER:
			order = matchOrderAttr();
			break;
		case DATATYPE:
			type = matchDataTypeAttr();
			break;
		default:
			syntaxError("Nieoczekiwany symbol: ", symbol);
			return 0;
		}
		optional(WS);
	}
	accept(ENDEMPTYELEM);
	return new XSLSort(selected,order,type);
}

// If -> IF TestE '>' Content '</' IF '>'
XSLConditional * XSLTParser::IfClause() {
	accept(IFCLAUSE);
	accept(WS);
	XPathExpr* expr = matchCondition();
	accept(CLOSETAG);
	InstructionVec body = Content();
	closeElement(IFCLAUSE);
	return new XSLConditional(expr, body);
}

// Choose -> CHOOSE '>' {'<' When } ('<' Otherwise)? '</' CHOOSE '>'
XSLBranch * XSLTParser::Choose() {
	ConditionalVec conds;
	InstructionVec otherwise;
	accept(CHOOSE);
	accept(CLOSETAG);
	optional(WS);
	while(symbol!=OPENENDTAG) {
		accept(OPENTAG);
		if(symbol==WHEN)
			conds.push_back(When());
		else {
			otherwise = Otherwise();
			closeElement(CHOOSE);
			return new XSLBranch(conds, otherwise);
		}
	}
	closeElement(CHOOSE);
	return new XSLBranch(conds);
}

//When -> WHEN TestE '>' Content '</' WHEN '>'
XSLConditional * XSLTParser::When() {
	accept(WHEN);
	accept(WS);
	XPathExpr * expr = matchCondition();
	accept(CLOSETAG);
	InstructionVec body = Content();
	closeElement(WHEN);
	return new XSLConditional(expr, body);
}

// Otherwise -> OTHERWISE '>' Content '</' OTHERWISE '>'
InstructionVec XSLTParser::Otherwise() {
	accept(OTHERWISE);
	optional(WS);
	accept(CLOSETAG);
	InstructionVec ret = Content();
	closeElement(OTHERWISE);
	return ret;
}

//ForEach -> FOREACH SelectNS '>' ForEachContent '</' FOREACH '>'
XSLRepetition * XSLTParser::ForEach() {
	accept(FOREACH);
	accept(WS);
	LocExpr * xPath = matchSelectNodeSet();
	optional(WS);
	accept(CLOSETAG);
	optional(WS);
	std::pair<SortVec, InstructionVec> vecs = ForEachContent();
	closeElement(FOREACH);
	return new XSLRepetition(xPath, vecs.first, vecs.second);
}

//FEC -> { '<' Sort | Comment } Content
std::pair<SortVec, InstructionVec> XSLTParser::ForEachContent() {
	std::pair<SortVec, InstructionVec> ret;
	bool endOfSorts = false;
	while(symbol!=OPENENDTAG && !endOfSorts) {
		switch(symbol) {
		case OPENTAG:
			accept(OPENTAG);
			optional(WS);
			if(symbol==SORT)
				ret.first.push_back(Sort());
			else {
				ret.second.push_back(TopLevelInstruction());
				endOfSorts = true;
			}
			break;
		case COMMENT:	Comment();
		break;
		default:		ret.second.push_back(Text());
		break;
		}
		optional(WS);
	}

	//kolejne instrukcje to juz na pewno nie sort.
	//nie wywoluje Content jawnie bo dodanie pierwszej
	//instrukcji na poczatku wektora, trwa liniowo w stosunku do jego dlugosci

	optional(WS);
	while(symbol!=OPENENDTAG) {
		switch(symbol) {
		case OPENTAG:	ret.second.push_back(TopLevelInstruction());
		break;
		case COMMENT:	Comment();
		break;
		default:		ret.second.push_back(Text());
		break;
		}
		optional(WS);
	}
	return ret;
}

// ValueOf -> VALUEOF SelectE '/>'
XSLValueOf * XSLTParser::ValueOf() {
	accept(VALUEOF);
	accept(WS);
	XPathExpr * xPath = matchSelectExpression();
	optional(WS);
	accept(ENDEMPTYELEM);
	return new XSLValueOf(xPath);
}

//Complex -> Name { Attribute } ( '/>' | '>' Content '</' Name '>')
XSLComplex * XSLTParser::ComplexInstruction() {
	//accept(OPENTAG);
	accept(ID);
	String name = accepted.lexeme;
	optional(WS);
	NodeVec attrs;
	while(symbol!=ENDEMPTYELEM && symbol != CLOSETAG) {
		attrs.push_back(XMLParser::Attribute());
		optional(WS);
	}
	if(symbol==ENDEMPTYELEM) {
		accept(ENDEMPTYELEM);
		return new XSLComplex(name,attrs);
	}
	else {
		accept(CLOSETAG);
		InstructionVec content = Content();
		accept(OPENENDTAG);
		String name2 = XMLParser::Name();
		if(name!=name2)
			semanticError(String("Znacznik zamykajacy ma nieodpowiednia nazwe: ")+=name2);
		accept(CLOSETAG);
		return new XSLComplex(name,attrs,content);
	}

}

std::pair<String, String> XSLTParser::Attribute() {
	std::pair<String, String> ret;
	ret.first = XMLParser::Name();
	optional(WS);
	accept(EQUALOP);
	optional(WS);
	accept(LITERAL);
	ret.second = accepted.lexeme;
	return ret;
}


}

