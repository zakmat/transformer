/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * IParser.cpp
 *
 * Autor: Mateusz Zak
 */

#include <cassert>
#include <cstdlib>
#include "IParser.h"


IParser::IParser(ILexer * l): lexer(l) { err = lexer->getErrSymbol();}

void IParser::getNextToken() {
	awaiting = lexer->matchNextToken();
	std::cout << "Token found: " << awaiting.lexeme << ' ' << awaiting.symbol << std::endl;
	symbol = awaiting.symbol;
}

void IParser::optional(const TokSymbol & t){
	if(symbol==t)
		accept(t);
}
void IParser::Error(const String & msg) const {
	lexer->Error("", msg );
}

void IParser::syntaxError(const String & msg, const TokSymbol & t) const {
	lexer->Error("",String("Blad skladniowy: ") + msg + tokenDescriptions[t]);
	//Cout << "Blad analizatora skladniowego: " << msg << tokenDescriptions[t] << '\n';
}

void IParser::semanticError(const String & msg) const {
	lexer->Error("", String("Blad semantyczny: ") + msg );
}

void IParser::accept(const TokSymbol & t) {
	if(awaiting.symbol==t) {
		accepted = awaiting;
		getNextToken();
	}
	else if(awaiting.symbol==err) {
		syntaxError("Parsowanie nieudane, sekwencja zrodlowa zakonczona przedwczesnie. Oczekiwano: ", t);
		exit(1);
	}
	else {
			syntaxError("Oczekiwany token: ", t);
			std::cout << "Blad fatalny. Parsowanie nie moze byc kontynuowane" << std::endl;
			exit(1);
	}
}
