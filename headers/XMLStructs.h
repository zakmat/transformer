/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XMLStructs.h
 *
 * Autor: Mateusz Zak
 */

#ifndef XMLSTRUCTS_H_
#define XMLSTRUCTS_H_

#include "IParser.h"
#include <vector>


double getNumericValue(const String& s);

namespace parsingXML {

struct Name {
	String namespaceName;
	String localName;

	Name(const String& n, const String& l):namespaceName(n),localName(l) {};
	Name(const String& l=""):namespaceName(""),localName(l) {};
	Name(const Name& n): namespaceName(n.namespaceName), localName(n.localName){};

	bool operator==(const Name& other) const { return namespaceName == other.namespaceName && localName == other.localName; };
	bool operator!=(const Name& other) const { return !(*this==other); };
	String string() const { if (namespaceName == "") return localName; else return namespaceName+":"+"localName";}
};

class Node;

typedef std::vector<Node *> NodeVec;

class ElementNode;

enum NodeType {ELEMENT, ATTRIBUTE, TEXT, COMMENT};
enum InstructionType {STYLESHEET = 1, TEMPLATE = 2, APPLYTEMPLATES = 4, VALUEOF = 8,
	FOREACH = 16, IFCLAUSE = 32, CHOOSE = 64, WHEN = 128, OTHERWISE = 256, SORT = 512};

class Node {
	static int counter;
protected:
	int number;
	ElementNode * parent;

public:
	double numberValue() const;
	virtual ~Node() {};
	void saveOrder();
	int getNumber() const { return number;};
	virtual NodeVec getDescendantsOrSelf() { return NodeVec(1,this); };
	virtual NodeVec getChildrenByName(const String& n) const {return NodeVec();};
	virtual NodeVec getTextChildren() const {return NodeVec();};
	virtual NodeVec getAllChildren() const {return NodeVec();};
	virtual NodeVec getAllElementChildren() const {return NodeVec();};
	virtual NodeVec getAttrByName(const String& n) const {return NodeVec();};
	virtual NodeVec getAllAttrs() const {return NodeVec(); };
	ElementNode * getParent() const;
	Node * getCurrent();

	void setParent(ElementNode * p) {parent = p;};
	virtual void print(int d, std::ostream& os) const = 0;
	virtual Name getName() const { return Name("");};
	virtual NodeType type() const = 0;
//	virtual bool isElement() const { return false; };
//	virtual bool isText() const { return false; };
//	virtual bool isElement() const { return false; };
	Node * getRoot();

//	virtual XSLType* recognizeXSLElement() { return NULL;};

	virtual String string() const = 0;
	virtual Node * clone() const = 0;
};


class ElementNode : public Node{
	Name name;
	NodeVec attrs;
	NodeVec children;
public:
	NodeType type() const { return ELEMENT; };

	ElementNode(const Name& n, NodeVec a = NodeVec(), NodeVec c = NodeVec());
	ElementNode(const ElementNode& rhs);
	virtual ~ElementNode();
	void print(int d, std::ostream& os) const;

//	XSLType* recognizeXSLElement();
	String string() const;
	Name getName() const { return name; };
	ElementNode* clone() const { return new ElementNode(*this);};

	NodeVec getDescendantsOrSelf();
	NodeVec getChildrenByName(const String& n) const;
	NodeVec getTextChildren() const;
	NodeVec getAllChildren() const;
	NodeVec getAllElementChildren() const;
	NodeVec getAttrByName(const String& n) const;
	NodeVec getAllAttrs() const;

};

class AttributeNode : public Node{
	Name name;
	String value;
public:
	AttributeNode(const Name& n, const String& v): name(n), value(v) {};
	void print(int d, std::ostream& os) const;
	String string() const;
	Name getName() const  { return name; };
	AttributeNode * clone() const { return new AttributeNode(*this);};
//	XSLType* recognizeXSLElement();

	NodeType type() const { return ATTRIBUTE; };

};

class TextNode : public Node{
	String text;
public:
	TextNode(const String& t): text(t) {};
	void print(int d, std::ostream& os) const;
	String string() const;
//	XSLType* recognizeXSLElement(){};
	TextNode * clone() const { return new TextNode(*this);};

	NodeType type() const { return TEXT; };

};

class CommentNode : public Node {
	String comment;
public:
	CommentNode(const String& c): comment(c) {};
	void print(int d, std::ostream& os) const;
	String string() const { return String();};
	CommentNode * clone() const { return new CommentNode(*this);};

	NodeType type() const { return COMMENT; };
};

class XSLTTemplate;
typedef std::vector<XSLTTemplate *> TemplateVec;

class XMLTree : public ParsedObject {
	Node * root;

public:
	TemplateVec interpretAsStylesheet();
	//void recognizeXSLElement();
	//void recognizeXSLKeywords();
	Node * getRoot() const {return root;};
	XMLTree(Node * r = NULL):root(r) {};
	virtual ~XMLTree() { delete root;};
	//for debug purposes
	void printToConsole() {
		root->print(0,std::cout);
		std::cout << std::endl;
	}
};

}

#endif /* XMLSTRUCTS_H_ */
