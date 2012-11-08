/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * ISource.h
 *
 * Autor: Mateusz Zak
 */

#ifndef ISOURCE_H_
#define ISOURCE_H_

#include <string>
#include <iostream>

typedef std::string String;
typedef char Char;


//klasa zapewnia interfejs dostepu do sekwencji zrodlowej
class ISource {
protected:
	int errCounter;
public:
	ISource() : errCounter(0) {};
	virtual ~ISource() {};
	int countErrors() const { return errCounter;};
	virtual void Error(String msg) {
		errCounter++;
		std::cout << printPos() << msg;
	}
	virtual Char getNextCharacter() = 0;
	virtual void rollbackCharacter(const Char) = 0;
	virtual bool isFinished() const = 0;
	virtual String printPos() const = 0;
};


#endif /* ISOURCE_H_ */
