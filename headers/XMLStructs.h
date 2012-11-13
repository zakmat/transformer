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

class Node;

typedef std::vector<Node *> NodeVec;

class ElementNode;

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
	virtual String getName() const { return String();};
	virtual bool isElementNode() const { return false; };

	Node * getRoot();

	virtual String string() const = 0;
	virtual Node * clone() const = 0;
};


class ElementNode : public Node{
	String name;
	NodeVec attrs;
	NodeVec children;
public:
	bool isElementNode() const { return true; };

	ElementNode(String n, NodeVec a = NodeVec(), NodeVec c = NodeVec());
	ElementNode(const ElementNode& rhs);
	virtual ~ElementNode();
	void print(int d, std::ostream& os) const;
	String string() const;
	String getName() const { return name; };
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
	String name;
	String value;
public:
	AttributeNode(const String& n, const String& v): name(n), value(v) {};
	void print(int d, std::ostream& os) const;
	String string() const;
	String getName() const  { return name; };
	AttributeNode * clone() const { return new AttributeNode(*this);};
};

class TextNode : public Node{
	String text;
public:
	TextNode(const String& t): text(t) {};
	void print(int d, std::ostream& os) const;
	String string() const;
	TextNode * clone() const { return new TextNode(*this);};

};

class CommentNode : public Node {
	String comment;
public:
	CommentNode(const String& c): comment(c) {};
	void print(int d, std::ostream& os) const;
	String string() const { return String();};
	CommentNode * clone() const { return new CommentNode(*this);};
};
/*
class XMLNode {
	friend class XMLTree;
	XMLNode * parent;
	int nodeType;
	String name;
	std::vector<AttrType> attributes;
	std::vector<XMLNode *> children;
	void initParent();
public:
	XMLNode(String n, std::vector<AttrType> a = std::vector<AttrType>(), std::vector<XMLNode *>c = std::vector<XMLNode *>());
	XMLNode(String n, int t) {
		*this = XMLNode(n);
		nodeType = t;
	};
	virtual ~XMLNode();
	void print(int depth);
	std::vector<XMLNode *> getChildrenByName(String n) const;
	std::vector<XMLNode *> getAllChildren() const;
	std::vector<AttrType> getAttrByName(String n) const;
	std::vector<AttrType> getAllAttrs() const;
	XMLNode * getParent() const;
	XMLNode * getCurrent() const;

	XMLNode * getRoot() const {
		XMLNode * n = this;
		while(n->parent)
			n = n->parent;
		return n;
	}
};
*/
class XMLTree : public ParsedObject {
	Node * root;

public:
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
