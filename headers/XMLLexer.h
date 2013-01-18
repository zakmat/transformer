/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XMLLexer.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XMLLEXER_H_
#define XMLLEXER_H_

#include "ILexer.h"


/*
 * Atomy leksykalne wykorzystywane podczas analizy pliku XML
 */
enum XMLSymbol {
	OPENTAG, OPENENDTAG, OPENPI, OPENCDATA, OPENCOMMENT, CLOSETAG, ENDEMPTYELEM, ENDPI,
	ENDCDATA, ENDCOMMENT, EQUALOP, MINUS, COLON, LITERAL, ID, WS, OTHER, MAXSYMXML
};


//Analizator leksykalny plikow w formacie XML
class XMLLexer: public ILexer {
	bool isWhiteSpaceChar(const Char c) const;
	bool isCharData(const Char c) const;
	String matchComment();
	Token recognizeMarkupOpening(Char secondChar);
	Token recognizeMarkupEnding(Char firstChar);
public:
	XMLLexer(ISource * src): ILexer(src) {};
	virtual Token matchNextToken();
	int getErrSymbol() {return MAXSYMXML;}
};

#endif /* XMLLEXER_H_ */
