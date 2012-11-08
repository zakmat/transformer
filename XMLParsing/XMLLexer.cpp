/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XMLLexer.cpp
 *
 * Autor: Mateusz Zak
 */


#include "XMLLexer.h"

namespace parsingXML {

bool XMLLexer::isWhiteSpaceChar(const Char c) const {
	return c == 0x20 || c==0x09 || c==0x0a || c==0x0d;
}

bool XMLLexer::isCharData(const Char c) const {
	return isalnum(c) || c == '-' || c == ':';
}

Token XMLLexer::matchNextToken() {
	String seqSoFar("");
	Char c = getChar();

	//sprawdzamy czy dokument zrodlowy nie zostal juz w pelni zeskanowany
	if(c==0)
		return Token(MAXSYM,"");

	if(isWhiteSpace(c)) {
		seqSoFar.append(1,c);
		while(isWhiteSpace(c=getChar()))
			seqSoFar.append(1,c);
		rollbackChar(c);
		return Token(WS,seqSoFar);
	}
	else if(isCharData(c)) {
		do {
			seqSoFar.append(1,c);
			c = getChar();
		} while(isCharData(c)||c=='-');
		rollbackChar(c);
		return Token(ID,seqSoFar);
	}
	switch(c) {
	case '<':	return recognizeMarkupOpening(getChar());
	case '>':	return Token(CLOSETAG,String(1,c));
	case '?':	return recognizeMarkupEnding(c);
	case '/':	return recognizeMarkupEnding(c);
	case '=':	return Token(EQUALOP,String("="));
	case ':':	return Token(COLON,String(":"));
	case '"': 	while((c=getChar())!='"') {
					seqSoFar.append(1,c);
				}
				return Token(LITERAL,seqSoFar);
	default:	return Token(OTHER,String(1,c));
	}
}

Token XMLLexer::recognizeMarkupOpening(Char c2) {
	String seqSoFar;
	switch(c2) {
	case '/':	return Token(OPENENDTAG,String("</"));
	case '?':	return Token(OPENPI,String("<?"));
	case '!':   seqSoFar.append(1, getChar());
				seqSoFar.append(1, getChar());
				if(seqSoFar==String("--"))
					return Token(COMMENT,matchComment());
				else {
					rollbackChar(seqSoFar);
					return Token(OTHER, String("<!"));
				}
	default:	rollbackChar(c2);
				return Token(OPENTAG,String("<"));
	}
}

Token XMLLexer::recognizeMarkupEnding(Char c) {
	Char c2 = getChar();
	if(c2!='>') {
		rollbackChar(c2);
		Error("Znacznik zamykajacy niekompletny");
		return Token(OTHER,String(1,c));
	}
	else if(c=='/')
		return Token(ENDEMPTYELEM,String("/>"));
	else //c=='?'
		return Token(ENDPI,String("?>"));
}

String XMLLexer::matchComment() {
	String comment="";
//	bool commentEndFound = false;
	Char c='-', c2;

	while(c!=0) {
		comment.push_back(getChar());
		comment.push_back(getChar());
		c = getChar();

		if(c=='>') {
			if(comment.at(comment.size()-1)=='-' && comment.at(comment.size()-2)=='-') {
				comment.erase(comment.size()-2);
				return comment;
			}
		}
		else if(c=='-') {
			if(comment.at(comment.size()-1)=='-') {
				if(getChar()!='>')
					Error("Znacznik zamykajacy komentarz niekompletny");
				comment.erase(comment.size()-1);
				return comment;
			}
			else if ((c2 = getChar())=='-') {
				if(getChar()!='>')
					Error("Znacznik zamykajacy komentarz niekompletny");
				return comment;
			}
		}
		comment.append(1,c);
	}
	Error("Znacznik zamykajacy komentarz nie zostal znaleziony");
	return comment;
}

}
