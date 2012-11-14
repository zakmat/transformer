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
	Node * root = new ElementNode("root", attrs, main);
	root->saveOrder();
	return root;
}

//Prolog -> '<?' Name { Attribute } '?>'
NodeVec XMLParser::Prolog() {
	accept(OPENPI);
	String n = Name();

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
	String name = Name();
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
		String name2 = Name();
		if(name!=name2)
			semanticError(String("Znacznik zamykajacy ma nieodpowiednia nazwe: ")+=name2);
		accept(CLOSETAG);
		return new ElementNode(name,attrs,content);
	}
}

//Attribute -> Name '=' Literal
AttributeNode * XMLParser::Attribute() {
	String name, value;
	name = Name();
	optional(WS);
	accept(EQUALOP);
	optional(WS);
	value = Literal();
	return new AttributeNode(name,value);
}

String XMLParser::Name() {
	String nameSoFar;
	accept(ID);
	nameSoFar.append(accepted.lexeme);
	if(symbol==MINUS) {
		accept(MINUS);
	    nameSoFar.append("-");
	}
	if(symbol==COLON) {
		accept(COLON);
		accept(ID);
		nameSoFar.append(":");
		nameSoFar.append(accepted.lexeme);
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

CommentNode * XMLParser::CData() {
	accept(OPENCDATA);
	String cdata;
	while(symbol!=ENDCDATA) {
		accept(symbol);
		cdata.append(accepted.lexeme);
	}
	accept(ENDCDATA);
	return new CommentNode(cdata);
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
