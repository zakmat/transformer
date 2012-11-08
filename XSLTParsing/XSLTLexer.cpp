/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XSLTLexer.cpp
 *
 * Autor: Mateusz Zak
 */


#include "XSLTLexer.h"

namespace parsingXSLT {

//kolejnosc slow kluczowych zostala ustalona eksperymentalnie
XSLTLexer::KeywordMapping XSLTLexer::keywords[KEYWORDSNUM] = {
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
};

XSLTLexer::XSLTLexer(ISource * src): XMLLexer(src) {}


Token XSLTLexer::matchNextToken() {
	Token ret = XMLLexer::matchNextToken();
	if(ret.symbol==XMLMAXSYM)
		return Token(MAXSYM, "");
	else if(ret.symbol==ID)
		return tryToMatchKeyword(ret.lexeme);
	return ret;
}

Token XSLTLexer::tryToMatchKeyword(const String & sequence) {
	int hash = calculateHash(sequence);
	if (keywords[hash].key == sequence)
		return Token(keywords[hash].symbol, sequence);
	else return Token(ID, sequence);
}

int XSLTLexer::calculateHash(const String & sequence) const {
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
}

}
