/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * FileStreamSource.cpp
 *
 * Autor: Mateusz Zak
 */

#include "FileStreamSource.h"

FileStreamSource::FileStreamSource(std::string fname, bool v):
		lineSoFar(""), verboseMode(v),
		lineNumber(1), colNumber(1), filename(fname), endOfSequence(false), isRolledBack(false) {
	in.open(filename.c_str(), std::ios_base::in);
	if(!in.is_open())
		std::cout << "Nie udalo sie otworzyc pliku" << std::endl;
	else
		std::cout << "Udalo sie otworzyc plik " << fname << std::endl;
}

FileStreamSource::~FileStreamSource() {
	in.close();
}

void FileStreamSource::updatePosition(Char c) {
	if(c!='\n') {
		++colNumber;
	}
	else {
		if(verboseMode) std::cout << lineNumber << ":\t" << lineSoFar;
		lineSoFar = "";

		colNumber = 1;
		++lineNumber;
	}
}

Char FileStreamSource::getNextCharacter() {
	if (isRolledBack) {
		isRolledBack = false;
		return rolledback;
	}
	Char c = in.get();
	if(in.eof())
		endOfSequence = true;
	else
		lineSoFar.push_back(c);
	updatePosition(c);
	return c;
}


void FileStreamSource::rollbackCharacter(const Char c) {
	isRolledBack = true;
	rolledback = c;
}

bool FileStreamSource::isFinished() const {
	return endOfSequence;
}
