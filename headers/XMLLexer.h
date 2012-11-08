/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XMLLexer.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XMLLEXER_H_
#define XMLLEXER_H_

#include "ILexer.h"

namespace parsingXML {

/*
 * Atomy leksykalne wykorzystywane podczas analizy pliku XML
 */
enum XMLSymbol {
	OPENTAG, OPENENDTAG, OPENPI, COMMENT, CLOSETAG, ENDEMPTYELEM, ENDPI,
	EQUALOP, COLON, NAME, LITERAL, ID, WS, OTHER, MAXSYM
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
	int getErrSymbol() {return MAXSYM;}
};

}

#endif /* XMLLEXER_H_ */
