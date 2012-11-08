/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * StringSource.cpp
 *
 * Autor: Mateusz Zak
 */

#include "StringSource.h"
#include <iostream>
#include <sstream>

String StringSource::printPos() const {
	std::ostringstream ss;
	ss << pos <<": ";
	return ss.str();
}

Char StringSource::getNextCharacter() {
	if(isFinished())
		return 0;
	return sequence.at(pos++);
}
void StringSource::rollbackCharacter(const Char) {
	--pos;
}
bool StringSource::isFinished() const {
	return pos==sequence.size();
}

void StringSource::Error(String msg) {
	errCounter++;
	errors.push_back(msg);
}

void StringSource::printErrors() const {
	for(std::vector<String>::const_iterator it = errors.begin(); it!=errors.end(); it++)
		std::cout << "XPathError:" << *it << '\n';
}

StringSource::StringSource(String seqToParse): sequence(seqToParse), pos(0) {}

StringSource::~StringSource() {}
