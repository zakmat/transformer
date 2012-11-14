/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * StringSource.h
 *
 * Autor: Mateusz Zak
 */

#ifndef STRINGSOURCE_H_
#define STRINGSOURCE_H_

#include <vector>
#include "ISource.h"

//klasa wykorzystywana podczas parsowania wyrazen XPath, sekwencja zrodlowa jest w istocie stringiem
class StringSource: public ISource {
	std::vector<String> errors;
	String sequence;
	unsigned pos;
public:
	String printPos() const;
	void Error(String msg);
	void printErrors() const;
	virtual Char getNextCharacter();
	virtual void rollbackCharacter(const Char);
	virtual bool isFinished() const;
	StringSource(String seqToParse);
	virtual ~StringSource();
};

#endif /* STRINGSOURCE_H_ */
