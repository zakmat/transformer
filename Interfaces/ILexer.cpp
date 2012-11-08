/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * ILexer.cpp
 *
 * Autor: Mateusz Zak
 */

#include "ILexer.h"

Char ILexer::getChar() {
	if(buffer.empty()) {
		if(source->isFinished())
			return 0;
		return source->getNextCharacter();
	}
	else {
		Char c = buffer.front();
		buffer.pop_front();
		return c;
	}
}

void ILexer::rollbackChar(Char c) {
	buffer.push_back(c);
}

void ILexer::rollbackChar(String & s) {
	unsigned i = 0;
	while(i<s.size())
		rollbackChar(s.at(i++));
}

bool ILexer::isWhiteSpace(const Char c) const {
	return c == 0x20 || c==0x09 || c==0x0a || c==0x0d;
}

bool ILexer::isLetter(const Char c) const{
	return isalnum(c);
}
bool ILexer::isDigit(const Char c) const {
	return isdigit(c);
}
