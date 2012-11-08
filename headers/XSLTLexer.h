/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XSLTLexer.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XSLTLEXER_H_
#define XSLTLEXER_H_

#include "XMLLexer.h"

namespace parsingXSLT {

/*
 * atomy akceptowane przez analizator leksykalny XSLT, dziela sie na dwie grupy, pierwsza z nich to atomy wystepujace
 * rowniez w a.l. dla dokumentow XML, natomiast druga grupa to rozpoznawane slowa kluczowe procesora XSLT
 */

enum XSLTSymbol {
	OPENTAG, OPENENDTAG, OPENPI, COMMENT, CLOSETAG, ENDEMPTYELEM, ENDPI,
	EQUALOP, COLON, NAME, LITERAL, ID, WS, OTHER, XMLMAXSYM,
	STYLESHEET, TEMPLATE, APPLYTEMPLATES, VALUEOF, FOREACH, IFCLAUSE, CHOOSE, WHEN, OTHERWISE, SORT,
	TEMPLATENAME, MATCH, PRIORITY, SELECT, ORDER, DATATYPE, TEST, MAXSYM
};

const int KEYWORDSNUM = MAXSYM - OTHER - 1;

/*
 * analizator leksykalny dla procesora XSLT wykorzystuje fakt, ze XSLT jest w szczegolnosci
 * dokumentem XML.
 * Stad dziedziczenie, roznica pojawia sie podczas interpretacji identyfikatorow jako slow kluczowych
 */
class XSLTLexer: public parsingXML::XMLLexer {
	//statyczna tablica mapujaca symbole przyspiesza analize slow kluczowych
	struct KeywordMapping {
		const char * key;
		XSLTSymbol symbol;
	};

	static KeywordMapping keywords[KEYWORDSNUM];

	//dla identyfikatorow obliczany jest hash
	int calculateHash(const String & sequence) const;
	//funkcja probuje dopasowac odczytany identyfikator do slowa kluczowego
	Token tryToMatchKeyword(const String & sequence);
public:
	XSLTLexer(ISource * src);
	Token matchNextToken();
	int getErrSymbol() {return MAXSYM;}
};

}


#endif /* XSLTLEXER_H_ */
