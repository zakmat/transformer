/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * Transformer.h
 *
 * Autor: Mateusz Zak
 */

#ifndef TRANSFORMER_H_
#define TRANSFORMER_H_

#include "XMLStructs.h"
#include "XSLTStructs.h"

//Nadrzedna klasa aplikacja, spaja wszystkie moduly, to do jej metod odwolujemy sie w funkcji main
class Transformer {
	bool verboseMode;

	char * sourcefname;
	char * stylesfname;
	char * destfname;

	parsingXML::XMLTree * source;
	int sourceErrsDetected;
	parsingXSLT::XSLTStylesheet * transformations;
	int styleErrsDetected;

	std::vector<parsingXML::Node *> result;

	parsingXSLT::Context getInitialContext() const;
	parsingXML::XMLTree * parseXML(ISource * document) const;
	parsingXSLT::XSLTStylesheet * parseXSL(ISource * document) const;
public:
	Transformer();
	~Transformer();

	void isVerbose(bool b) { verboseMode = b;};
	bool errorsDetected () const { return styleErrsDetected>0 || sourceErrsDetected>0; };

	void loadXMLSource(char * filename);
	void loadStyleSheet(char * filename);
	void setDestination(char * filename);
	void writeResultToFile() { writeResultToFile(destfname);};
	void writeResultToFile(char * filename);

	//funkcja rozpoczynajaca proces przetwarzania wczytanego wczesniej dokumentu zrodlowego
	void generateResultTree();
};

#endif /* TRANSFORMER_H_ */
