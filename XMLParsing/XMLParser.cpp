/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XMLParser.cpp
 *
 * Autor: Mateusz Zak
 */


#include "XMLParser.h"

namespace parsingXML {

char const * xmlTokDesc[MAXSYM] = {
		"<", "</" , "<?", "comment", ">", "/>", "?>", "=", ";", "ident", "literal",
		"ident2", "whitespace", "unknown"
};


XMLParser::XMLParser(ILexer * l): IParser(l) {
	tokenDescriptions = xmlTokDesc;
}

ParsedObject * XMLParser::startParsing() {
	getNextToken();
	XMLTree * pt = new XMLTree(Document());
	return pt;
}

//Document -> Prolog { Comment } Element
Node * XMLParser::Document() {
	NodeVec attrs = Prolog();
	optionalComment();
	Node * elem = Element();
	optionalComment();
	NodeVec main(1,elem);
	Node * root = new ElementNode(Name("root"), attrs, main);
	root->saveOrder();
	return root;
}

//Prolog -> '<?' Name { Attribute } '?>'
NodeVec XMLParser::Prolog() {
	accept(OPENPI);
	Name n = ExpName();

	optional(WS);
	NodeVec attrs;
	while(symbol != ENDPI) {
		attrs.push_back(Attribute());
		optional(WS);
	}
	accept(ENDPI);
	return attrs;
}

// Element:= '<' Name { Attribute } ( '/>' | ('>' Content '</' Name '>'))
ElementNode * XMLParser::Element() {
	accept(OPENTAG);
	Name name = ExpName();
	optional(WS);

	NodeVec attrs;
	while(symbol!=ENDEMPTYELEM && symbol != CLOSETAG) {
		attrs.push_back(Attribute());
		optional(WS);
	}
	if(symbol==ENDEMPTYELEM) {
		accept(ENDEMPTYELEM);
		return new ElementNode(name,attrs);
	}
	else {
		accept(CLOSETAG);
		NodeVec content = Content();
		accept(OPENENDTAG);
		Name name2 = ExpName();
		if(name!=name2)
			semanticError(String("Znacznik zamykajacy ma nieodpowiednia nazwe: ")+name2.string());
		accept(CLOSETAG);
		return new ElementNode(name,attrs,content);
	}
}

//Attribute -> Name '=' Literal
AttributeNode * XMLParser::Attribute() {
	Name name = ExpName();
	optional(WS);
	accept(EQUALOP);
	optional(WS);
	String value = Literal();
	return new AttributeNode(name,value);
}

Name XMLParser::ExpName() {
	String first = SimpleName();
	if(symbol==COLON) {
		accept(COLON);
		String second = SimpleName();
		return Name(first,second);
	}
	else
		return Name(first);
}

String XMLParser::SimpleName() {
	String nameSoFar;
	while(symbol==ID || symbol==MINUS) {
		if(symbol==MINUS) {
			accept(MINUS);
		    nameSoFar.append("-");
		}
		else {
			accept(ID);
			nameSoFar.append(accepted.lexeme);
		}
	}

	return nameSoFar;
}

String XMLParser::Literal() {
	accept(LITERAL);
	return accepted.lexeme;
}

// Content -> { Element | Comment | Plain } ^'</'
//ogranicznikiem jest '</'
NodeVec XMLParser::Content() {
	NodeVec ret;
	while(symbol!=OPENENDTAG) {
		if(symbol==OPENTAG) {
			ret.push_back(Element());
		}
		else if(symbol==COMMENT) {
			ret.push_back(Comment());
		}
		else {
			TextNode * t = Plain();
			if(t!=0)
				ret.push_back(t);
		}
		optional(WS);
	}
	return ret;
}

//Plain -> ^{ '<' | '</' }
TextNode * XMLParser::Plain() {
	String ret;
	optional(WS);
	while(symbol != OPENTAG && symbol != OPENENDTAG) {
		accept(symbol);
		ret.append(accepted.lexeme);
	}
	if(ret.size()>0) {
		return new TextNode(ret);
	}
	else
		return 0;
}

//TODO sprawdzic w xml-reference ktore znaczniki sa zabronione w commencie,
//poki co zjada wszystko az do napotkania konca komentarza
CommentNode * XMLParser::Comment() {
	accept(OPENCOMMENT);
		String comment;
		while(symbol!=ENDCOMMENT) {
			accept(symbol);
			comment.append(accepted.lexeme);
		}
		accept(ENDCOMMENT);
		return new CommentNode(comment);
}

TextNode * XMLParser::CData() {
	accept(OPENCDATA);
	String cdata;
	while(symbol!=ENDCDATA) {
		accept(symbol);
		cdata.append(accepted.lexeme);
	}
	accept(ENDCDATA);
	return new TextNode(cdata);
}

// { Comment }
void XMLParser::optionalComment() {
	optional(WS);
	while(symbol==OPENCOMMENT) {
		delete Comment();
		//optional(COMMENT);
		optional(WS);
	}
}

}
