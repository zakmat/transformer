/* Parser plików XML oraz wykonanie przekształcenia XSLT
 * ILexer.h
 *
 * Autor: Mateusz Zak
 */

#ifndef ILEXER_H_
#define ILEXER_H_

#include <list>

#include "ISource.h"

typedef int TokSymbol;

struct Token {
	TokSymbol symbol;
	String lexeme;

	Token(TokSymbol _s=-1, String _l=String("")): symbol(_s),lexeme(_l) {};
};

//abstrakcyjna klasa analizatora leksykalnego
class ILexer {
	//powiazanie z wejsciem
	ISource * source;
	//bufor jest potrzebny na przechowanie cofnietych znakow
	std::list<Char> buffer;

protected:
	void rollbackChar(Char c);
	void rollbackChar(String & s);
	Char getChar();
	String getChars(int number);
	bool isWhiteSpace(const Char c) const;
	bool isLetter(const Char c) const;
	bool isDigit(const Char c) const;

public:
	ILexer(ISource * src) : source(src){ buffer = std::list<Char>(); };
	virtual ~ILexer() {};

	void Error(String le, String oe="") {
		if(le=="")
			source->Error(oe);
		else
			source->Error("Blad analizatora leksykalnego: "+le);
	}
	virtual int getErrSymbol() = 0;
	virtual Token matchNextToken() = 0;
};

#endif /* ILEXER_H_ */
