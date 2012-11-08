/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * IParser.h
 *
 * Autor: Mateusz Zak
 */

#ifndef IPARSER_H_
#define IPARSER_H_

#include "ILexer.h"
#include <vector>
#include <algorithm>


//mala pomocnicza struktura wykorzystywana podczas dealokacji zasobow
struct myDel {
	template<typename T>
	void operator()(T&pX) {
		if(pX)
			delete pX;
		pX = NULL;
	}
};

//Potrzebny mi jest obiekt tego typu aby wszystkie parsery mialy spojne API
class ParsedObject {
public:
	virtual ~ParsedObject() {}
};

//klasa abstrakcyjna zapewniajaca jednolity interfejs obslugi wszystkich parserow
class IParser {
	//wskaznik na analizator leksykalny
	ILexer * lexer;
protected:
	//opis slowny zadeklarowanych tokenow
	const char** tokenDescriptions;
	//awaiting oznacza, ze w istocie realizujemy parser LR(1)
	Token accepted;
	Token awaiting;
	TokSymbol symbol;
	int err;

	void getNextToken();
	void accept(const TokSymbol & t);
	//pozwala zignorowac token na wejsciu i kontynuowac parsowanie
	void optional(const TokSymbol & t);
	void Error(const String & msg)const;
	void syntaxError(const String & s, const TokSymbol & t) const;
	void semanticError(const String & s) const;
	IParser(ILexer * l);
public:
	virtual ~IParser() {}
	virtual ParsedObject * startParsing() = 0;
};


#endif /* IPARSER_H_ */
