/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XPathLexer.cpp
 *
 * Autor: Mateusz Zak
 */


#include "XPathLexer.h"

namespace parsingXPath {

Token XPathLexer::matchNextToken() {

	String seqSoFar("");
	Char c = getChar();

	//analizator doszedl do konca sekwencji
	if(c==0)
		return Token(MAXSYM,"");

	if(isWhiteSpace(c)) {
		seqSoFar.append(1,c);
		while(isWhiteSpace(c=getChar()))
			seqSoFar.append(1,c);
		rollbackChar(c);
		//return Token(WS,seqSoFar);
		//w ten sposob pomijam biale znaki
		return matchNextToken();
	}

	if(isDigit(c))
		return recognizeNumber(c);
	else if(isLetter(c)) {
		do {
			seqSoFar.append(1,c);
			c = getChar();
		} while(isLetter(c));
		rollbackChar(c);
		if(seqSoFar=="or")
			return Token(OR,"or");
		else if(seqSoFar=="and")
			return Token(AND,"and");
		else if(seqSoFar=="not")
			return Token(NOT,"not");
		return Token(NODE,seqSoFar);
	}

	switch(c) {
	case '(': return Token(LBRACK,"-");
	case ')': return Token(RBRACK,"-");

	case '-': return Token(MINUS,"-");
	case '+': return Token(PLUS, "+");
	case '=': return Token(EQ, "=");
	case '|': return Token(NODEOR,"|");
	case '!': {
		c = getChar();
		if(c=='=')
			return Token(NEQ,"!=");
		else {
			rollbackChar(c);
			return Token(OTHER,"!");
		}
	}
	case '*': return Token(ASTERISK,"*");
	case '@': return Token(ATTR,"@");
	case '.': return recognizeDot();
	case '&': return recognizeEscapedSequence();
	case '/': return recognizeSlash();
	case '\'':return recognizeLiteral();
	default:  return Token(OTHER,String(1,c));

	}
}

Token XPathLexer::recognizeDot() {
	Char c = getChar();
	if(c=='.')
		return Token(PARENT,"..");
	else {
		rollbackChar(c);
		return Token(CURRENT,".");
	}
}
Token XPathLexer::recognizeEscapedSequence() {
	String seqSoFar=String("&");

	seqSoFar.push_back(getChar());
	seqSoFar.push_back(getChar());
	seqSoFar.push_back(getChar());

	if(seqSoFar=="&gt;") {
		Char c=getChar();

		if(c=='=')
			return Token(GTE,"&gt;=");
		else {
			rollbackChar(c);
			return Token(GT, "&gt;");
		}

	}
	else if(seqSoFar=="&lt;") {
		Char c=getChar();
		if(c=='=')
			return Token(LTE,"&lt;=");
		else {
			rollbackChar(c);
			return Token(LT, "&lt;");
		}
	}
	else {
		rollbackChar(seqSoFar);
		return Token(OTHER, "&");
	}
}

Token XPathLexer::recognizeNumber(Char c_){
	String num;
	Char c = c_;
	//	Char c2;
	do {
		num.push_back(c);
		c = getChar();
	} while(isDigit(c));

	if(c=='.') {
		c=getChar();
		if(!isDigit(c)) {
			Error("Nieprawidlowy format liczby");
			return Token(NUM,num);
		}
		else {
			num.push_back('.');
			do {
				num.push_back(c);
				c = getChar();
			} while(isDigit(c));
			rollbackChar(c);
			return Token(NUM,num);
		}
	}
	else {
		rollbackChar(c);
		return Token(NUM,num);
	}
}

Token XPathLexer::recognizeSlash() {
	Char c = getChar();
	if(c=='/')
		return Token(ALL,"//");
	else {
		rollbackChar(c);
		return Token(SLASH,"/");
	}
}

Token XPathLexer::recognizeLiteral() {
	String seqSoFar;
	Char c;
	while((c=getChar())!='\'') {
		seqSoFar.append(1,c);
	}
	return Token(LITERAL,seqSoFar);
}

}

