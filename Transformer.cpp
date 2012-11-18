/*
 * Transformer.cpp
 *
 *  Created on: 09-01-2012
 *      Author: krejziwan
 */

#include <fstream>

#include "Transformer.h"

#include "FileStreamSource.h"
#include "XMLLexer.h"
#include "XMLParser.h"

#include "XSLTParser.h"

using parsingXSLT::Context;

Transformer::Transformer(): source(0), transformations(0) {
}

Transformer::~Transformer() {
	if(source)
		delete source;
	if(transformations)
		delete transformations;
}

void Transformer::loadXMLSource(char * filename) {
	this->sourcefname = filename;
	ISource * xmldoc = new FileStreamSource(sourcefname,verboseMode);

	source = parseXML(xmldoc);
	sourceErrsDetected = xmldoc->countErrors();

	delete xmldoc;
}

parsingXML::XMLTree * Transformer::parseXML(ISource * document) const {
	ILexer * xmllex = new parsingXML::XMLLexer(document);
	IParser * xmlparser = new parsingXML::XMLParser(xmllex);

	std::cout << "Rozpoczeto parsowanie dokumentu zrodlowego" << std::endl;

	parsingXML::XMLTree * tree = (parsingXML::XMLTree *) xmlparser->startParsing();
	std::cout << "Zakonczono parsowanie dokumentu zrodlowego" << std::endl;

	//parsingXML::XMLTree * source = (parsingXML::XMLTree *) pt;

	delete xmllex;
	delete xmlparser;

	return tree;
}

//od teraz loadStyleSheet będzie w pierwszej kolejności korzystał właśnie z fcji loadXML
void Transformer::loadStyleSheet(char * filename) {
	this->stylesfname = filename;
	ISource * xsldoc = new FileStreamSource(stylesfname,verboseMode);

	transformations = recognizeStylesheet(parseXML(xsldoc));
	styleErrsDetected = xsldoc->countErrors();

	delete xsldoc;

	std::cout << "Zakonczono wczytywanie arkusza stylow" << std::endl;
}

parsingXSLT::XSLTStylesheet * Transformer::recognizeStylesheet(parsingXML::XMLTree * tree) {
	ILexer * xsllex = new parsingXSLT::XSLTLexer(xsldoc);
	IParser * xslparser = new parsingXSLT::XSLTParser(xsllex);

	tree->recognizeXSLKeywords();
	xslParser->validateStructure(tree);

	ParsedObject * pt = xslparser->startParsing();

	delete xsllex;
	delete xslparser;

	return (parsingXSLT::XSLTStylesheet *) pt;
}

void Transformer::setDestination(char * filename) {
	this->destfname = filename;
}

parsingXSLT::Context Transformer::getInitialContext() const {
	std::vector<parsingXML::Node *> temp(1,source->getRoot());
	return parsingXSLT::Context(temp, transformations);
}


void Transformer::generateResultTree() {
	Context c = getInitialContext();
	result = c.proccess();
	std::cout << "Zakonczono konstrukcje drzewa wynikowego" << std::endl;
}

void Transformer::writeResultToFile(char * fname) {
	std::ofstream myfile;
	myfile.open (destfname);
	std::vector<parsingXML::Node *>::iterator it;
	for(it= result.begin(); it!=result.end();++it) {
		(*it)->print(0,myfile);
		if(verboseMode)
			(*it)->print(0,std::cout);
	}
	myfile.close();
}
