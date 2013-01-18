/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XMLLexer.cpp
 *
 * Autor: Mateusz Zak
 */


#include "XMLLexer.h"

bool XMLLexer::isWhiteSpaceChar(const Char c) const {
	return c == 0x20 || c==0x09 || c==0x0a || c==0x0d;
}

bool XMLLexer::isCharData(const Char c) const {
	return isalnum(c);// || c == '-' || c == ':';
}

Token XMLLexer::matchNextToken() {
	String seqSoFar("");
	Char c = getChar();

	//sprawdzamy czy dokument zrodlowy nie zostal juz w pelni zeskanowany
	if(c==0)
		return Token(MAXSYMXML,"");

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
		} while(isCharData(c)/*||c=='-'*/);
		rollbackChar(c);
		return Token(ID,seqSoFar);
	}
	else if((String(">?/]-")).find(c)<5)
		return recognizeMarkupEnding(c);
	switch(c) {
	case '<':	return recognizeMarkupOpening(getChar());
	case '=':	return Token(EQUALOP,String("="));
	case ':':	return Token(COLON,String(":"));
	case '"': 	while((c=getChar())!='"') {
					seqSoFar.append(1,c);
				}
				return Token(LITERAL,seqSoFar);
	default:	return Token(OTHER,String(1,c));
	}
}


//TODO write a test case to check if CDATA is working well
//TODO write a test case if comment is not screwed up
Token XMLLexer::recognizeMarkupOpening(Char c2) {
	String seqSoFar;
	switch(c2) {
	case '/':	return Token(OPENENDTAG,String("</"));
	case '?':	return Token(OPENPI,String("<?"));
	case '!':   seqSoFar.append(getChars(2));
				if(seqSoFar==String("--"))
					return Token(OPENCOMMENT,String("<!--"));
				seqSoFar.append(getChars(5));
				if(seqSoFar==String("[CDATA["))
					return Token(OPENCDATA,String("<![CDATA["));
				else {
					rollbackChar(seqSoFar);
					return Token(OTHER, String("<!"));
				}
	default:	rollbackChar(c2);
				return Token(OPENTAG,String("<"));
	}
}

Token XMLLexer::recognizeMarkupEnding(Char c) {
	if (c == '>')
		return Token(CLOSETAG,String(">"));

	Char c2 = getChar();
	if (c2 == '>') {
		if(c=='/')
			return Token(ENDEMPTYELEM,String("/>"));
		else if(c=='?')
			return Token(ENDPI,String("?>"));
	}
	Char c3 = getChar();

	if (c == ']' && c2 == ']' && c3 == '>')
		return Token(ENDCDATA,String("]]>"));
	else if(c == '-' && c2 == '-' && c3 == '>')
		return Token(ENDCOMMENT, String("-->"));
	else if(c == '-') {
		rollbackChar(c2);
		rollbackChar(c3);
		return Token(MINUS,String(1,c));
	}
	else {  //(c2!=']') {
		rollbackChar(c2);
		rollbackChar(c3);
		Error("Znacznik zamykajacy niekompletny");
		return Token(OTHER,String(1,c));
	}
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

