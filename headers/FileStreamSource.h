/*
 * Procesor transformacji XSLT
 * Autor Mateusz Zak
 */

#ifndef FILESTREAMSOURCE_H_
#define FILESTREAMSOURCE_H_

#include <fstream>
#include <sstream>
#include <vector>

#include "ISource.h"

class FileStreamSource : public ISource{
	String lineSoFar;
	bool verboseMode;
	int lineNumber;
	int colNumber;
	std::string filename;

	std::ifstream in;
	bool endOfSequence;
	bool isRolledBack;
	Char rolledback;

	String printPos() const {
		std::stringstream a;
		a << lineNumber << ", " <<colNumber <<":";
		return a.str();
	}
	void updatePosition(Char c);
	void Error(String msg) {
		errCounter++;
		std::cout << lineNumber << ":\t" << lineSoFar <<'\n';
		std::cout << "Err:" << printPos() << msg << '\n';
	}
public:
	FileStreamSource(std::string filename, bool verbosity);
	virtual ~FileStreamSource();
	Char getNextCharacter();
	bool isFinished() const;
	void rollbackCharacter(const Char c);
};


#endif /* FILESTREAMSOURCE_H_ */
