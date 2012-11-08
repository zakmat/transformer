
CXX = g++

HEADERS = headers
FLAGS   = -Wall -pedantic


CXXFLAGS= ${FLAGS} -I${HEADERS}

APP_NAME = transformer

DIR_XML_PARSING          = XMLParsing
DIR_XPATH_PARSING        = XPathParsing
DIR_XSLT_PARSING         = XSLTParsing
DIR_INTERFACES           = Interfaces

XML_PARSING_CPP                   = $(wildcard $(DIR_XML_PARSING)/*.cpp)
XML_PARSING_OBJ                       = $(XML_PARSING_CPP:.cpp=.o)

XPATH_PARSING_CPP             = $(wildcard $(DIR_XPATH_PARSING)/*.cpp)
XPATH_PARSING_OBJ             = $(XPATH_PARSING_CPP:.cpp=.o)

XSLT_PARSING_CPP                      = $(wildcard $(DIR_XSLT_PARSING)/*.cpp)
XSLT_PARSING_OBJ                      = $(XSLT_PARSING_CPP:.cpp=.o)

INTERFACES_CPP                      = $(wildcard $(DIR_INTERFACES)/*.cpp)
INTERFACES_OBJ                      = $(INTERFACES_CPP:.cpp=.o)
	
MAIN                            = main.o
STRING_SOURCE_OBJ		= StringSource.o
FILE_STREAM_SOURCE_OBJ		= FileStreamSource.o
TRANSFORMER_OBJ			= Transformer.o

STRING_SOURCE_CPP		= StringSource.cpp
FILE_STREAM_SOURCE_CPP		= FileStreamSource.cpp
TRANSFORMER_CPP			= Transformer.cpp

BASE                            = $(XML_PARSING_OBJ) $(XPATH_PARSING_OBJ) $(XSLT_PARSING_OBJ) $(INTERFACES_OBJ) $(STRING_SOURCE_OBJ) $(FILE_STREAM_SOURCE_OBJ) $(TRANSFORMER_OBJ)


all: $(APP_NAME)


$(APP_NAME): $(BASE) $(MAIN)
	$(CXX) $(CXXFLAGS) -o $(APP_NAME) $(BASE) $(MAIN) 

DIR_XML_PARSING/%.o:       DIR_XML_PARSING/%.cpp 
DIR_XPATH_PARSING/%.o:       DIR_XPATH_PARSING/%.cpp
DIR_XSLT_PARSING/%.o:       DIR_XSLT_PARSING/%.cpp
DIR_INTERFACES/%.o:       DIR_INTERFACES/%.cpp

$(STRING_SOURCE_OBJ): $(STRING_SOURCE_CPP)
	$(CXX) $(CXXFLAGS) -c -o $(STRING_SOURCE_OBJ) $(STRING_SOURCE_CPP) 

$(FILE_STREAM_SOURCE_OBJ): $(FILE_STREAM_SOURCE_CPP)
	$(CXX) $(CXXFLAGS) -c -o $(FILE_STREAM_SOURCE_OBJ) $(FILE_STREAM_SOURCE_CPP)

$(TRANSFOMER_OBJ): $(TRANSFORMER_CPP)
	$(CXX) $(CXXFLAGS) -c -o $(TRANSFORMER_CPP) $(TRANSFORMER_CPP)
	
$(MAIN): main.cpp
	$(CXX) $(CXXFLAGS) -c -o $(MAIN) main.cpp

clean:
	find -iname "*.o" -or -name $(APP_NAME) | xargs rm -f

