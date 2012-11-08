/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * main.cpp
 *
 * Autor: Mateusz Zak
 */

#include <cstring>
#include "Transformer.h"


void printUsage(char * progname) {
	std::cout << "Sposob uruchomienia: \n";
	std::cout << progname << " [--v --h] --xml plik_zrodlowy --xslt arkusz_stylow --dest plik_wynikowy" << std::endl;
	std::cout << "\t--v: tryb rozwlekly,"<< std::endl
			  << "\t     wyprowadza kolejne linie parsowanych plikow i drzewo wynikowe na konsole" << std::endl;
	std::cout << "\t--h: wyswietla te wiadomosc." << std::endl;
}

int main(int argc, char * argv[]) {
	bool xsltFounded = false;
	char * xsltfilename;
	bool xmlFounded = false;
	char * xmlfilename;
	bool destFounded = false;
	char * destfilename;
	bool verboseMode = false;

	int i = 1;
	while(i<argc) {
		if(strcmp(argv[i],"--v")==0) {
			verboseMode = true;
			i++;
			continue;
		}
		else if(strcmp(argv[i],"--h")==0) {
			printUsage(argv[0]);
			return 0;
		}
		else if(strcmp(argv[i],"--xml")==0) {
			i++;
			xmlFounded = true;
			xmlfilename = argv[i++];
			continue;
		}
		else if(strcmp(argv[i],"--xslt")==0) {
			i++;
			xsltFounded = true;
			xsltfilename = argv[i++];
			continue;
		}
		else if(strcmp(argv[i],"--dest")==0) {
			i++;
			destFounded = true;
			destfilename = argv[i++];
			continue;
		}
		else {
			std::cout << "Nieprawidlowy parametr: " << argv[i] << std::endl;
			printUsage(argv[0]);
			return 1;
		}
	}
	if(!xmlFounded || !xsltFounded || !destFounded) {
		std::cout << "Opcje --xml, --xslt oraz --dest sa obowiazkowe" << std::endl;
		printUsage(argv[0]);
		return 1;
	}


	Transformer transformer;
	transformer.isVerbose(verboseMode);
	transformer.loadXMLSource(xmlfilename);
	transformer.loadStyleSheet(xsltfilename);
	transformer.setDestination(destfilename);

	if(!transformer.errorsDetected()) {
		transformer.generateResultTree();
		transformer.writeResultToFile(argv[3]);
	}

	return 0;
}


