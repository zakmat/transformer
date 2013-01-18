/*
 * Parser plików XML oraz wykonanie przekształcenia XSLT
 * XMLStructs.cpp
 *
 * Autor: Mateusz Zak
 */

#include "XMLStructs.h"
#include "XSLTAnalyzer.h"
#include <algorithm>
#include <functional>
#include <limits>
#include <cmath>

bool isNull(void * p) { return p==NULL; }


double getNumericValue(const String& s) {

	int integral = 0;
	int fractional = 0;
	unsigned i = 0;
	//	unsigned j = 0;
	bool m = false;

	//sprawdz czy na poczatku jest minus
	if(s[0] == '-') {
		m = true;
		i++;
	}
	//czesc calkowita
	while(i<s.size()&&isdigit(s[i])) {
		integral = 10 * integral + s[i] - '0';
		i++;
	}
	//w srodku moze sie pojawic tylko kropka
	if(i<s.size() && s[i]!='.')
		return std::numeric_limits<double>::quiet_NaN();
	if(i==s.size())
		return (double)(m ? -integral : integral);
	unsigned j = s.size()-1;
	//czesc ulamkowa
	while(i<j&&isdigit(s[j])) {
		fractional = 10 * fractional + s[j] - '0';
		j--;
	}
	if(i==j)
		return (fractional / pow(10,s.size()-1-j) + integral)*(m?-1.0:1.0);
	else
		return std::numeric_limits<double>::quiet_NaN();
}




template<typename T>
void deepCopy(const std::vector<T*> & from, std::vector<T*> & to) {
	to = std::vector<T*>(from.size(),NULL);
	std::transform(from.begin(),from.end(),to.begin(),std::mem_fun(&T::clone));
}


int Node::counter = 0;

//funkcja sprawdzajaca czy wartosc wezla jest numeryczna
double Node::numberValue()const {
	String s = string();
	std::cout << s;
	if(s.compare("Infinity")==0)
		return std::numeric_limits<double>::infinity();
	else if(s.compare("-Infinity")==0)
		return std::numeric_limits<double>::infinity()*-1.0;
	else if(s.compare("NaN")==0)
		return std::numeric_limits<double>::quiet_NaN();

	return getNumericValue(s);

}

Node * Node::getRoot() {
	Node * n = this;
	while(n->parent)
		n = n->parent;
	return n;
}

void Node::saveOrder() {
	number = counter++;
	NodeVec temp = getAllAttrs();
	std::for_each(temp.begin(),temp.end(),std::mem_fun(&Node::saveOrder));
	temp = getAllChildren();
	std::for_each(temp.begin(),temp.end(),std::mem_fun(&Node::saveOrder));
}


ElementNode::ElementNode(const Name& n, NodeVec a, NodeVec c) :
					name(n), attrs(a), children(c) {
	parent = NULL;
	//dla kazdego elementu wektora wywolujemy metode setParent z aktualnym wezlem jako argumentem
	std::for_each(attrs.begin(),attrs.end(), std::bind2nd(std::mem_fun(&Node::setParent), this));
	std::for_each(children.begin(),children.end(), std::bind2nd(std::mem_fun(&Node::setParent), this));

}


ElementNode::ElementNode(const ElementNode& rhs) {

	this->name = rhs.name;
	deepCopy(rhs.attrs,attrs);
	deepCopy(rhs.children,children);
}

ElementNode::~ElementNode() {
	std::for_each(attrs.begin(),attrs.end(), myDel());
	std::for_each(children.begin(),children.end(), myDel());
}

void indent(const int & depth, std::ostream& os) {
	for(int i=0;i<depth;++i)
		os << "    ";
}

void ElementNode::print(int depth, std::ostream& os) const{
	indent(depth, os); os << '<' << name.string();
	for(NodeVec::const_iterator it = attrs.begin();it!=attrs.end();++it) {
		os << " ";
		(*it)->print(0, os);
	}
	if(children.begin()==children.end()) {
		os << "/>\n";
		return;
	}
	else {
		os << ">\n";
	}
	for(NodeVec::const_iterator it = children.begin();it!=children.end();++it) {
		(*it)->print(depth+1, os);
	}
	indent(depth, os); os << "</" << name.string() << ">\n";
	os.flush();
}

void TextNode::print(int depth, std::ostream& os) const{
	indent(depth, os); os << text << '\n';
}

void CommentNode::print(int depth, std::ostream& os) const {
	indent(depth, os); os << "<!--" << comment << "-->\n";
}

void AttributeNode::print(int depth, std::ostream& os) const {
	indent(depth, os); os << name.string() << '=' <<'"' << value << '"';
}

String ElementNode::string() const {
	String s =

//			getName().string();
//	for(NodeVec::const_iterator it=attrs.begin(); it!=attrs.end();++it) {
//		s.append((*it)->string());
//		s.append(" ");
//	}
//	//wycinamy ostatnia spacje
//	s.erase(s.size()-1);
	return s;
}

String TextNode::string() const {
	return text;
}

String AttributeNode::string() const {
	return value;
}

NodeVec ElementNode::getDescendantsOrSelf() {
	NodeVec ret(1,this);
	for(NodeVec::const_iterator it=children.begin(); it!=children.end(); ++it) {
		NodeVec temp = (*it)->getDescendantsOrSelf();
		ret.insert(ret.end(),temp.begin(),temp.end());
	}
	return ret;
}

NodeVec ElementNode::getChildrenByName(const String& n) const {
	NodeVec ret;
	for(NodeVec::const_iterator it=children.begin(); it!=children.end(); it++)
		if((*it)->getName()==n)
			ret.push_back((*it));
	return ret;
}
NodeVec ElementNode::getAllChildren() const {
	return children;
}

NodeVec ElementNode::getAllElementChildren() const {
	NodeVec result;
	for(NodeVec::const_iterator it = children.begin(); it!=children.end(); ++it) {
		if((*it)->type()==ELEMENTTYPE)
			result.push_back(*it);
	}
	return children;
}

NodeVec ElementNode::getTextChildren() const {
	//TODO why do I need that
	return NodeVec();
}


NodeVec ElementNode::getAttrByName(const String& n) const {
	NodeVec ret;
	for(NodeVec::const_iterator it=attrs.begin(); it!=attrs.end(); it++)
		if((*it)->getName()==n)
			ret.push_back(*it);
	return ret;
}
NodeVec ElementNode::getAllAttrs() const {
	return attrs;
}
ElementNode * Node::getParent() const {
	return parent;
}
Node * Node::getCurrent() {
	return this;
}

void Node::pruneComments() {
	if(type()==ELEMENTTYPE) {
		NodeVec children = getAllChildren();
		for (NodeVec::iterator it = children.begin();it!=children.end();++it) {
			if ((*it)->type()==COMMENTTYPE) {
				delete (*it);
				(*it) = NULL;
			}
			else {
				(*it)->pruneComments();
			}
		}
		NodeVec::iterator nEnd = std::remove_if(children.begin(),children.end(),isNull);
		children.resize(nEnd - children.begin());
		setChildren(children);
	}

}

XSLTStylesheet * XMLTree::interpretAsStylesheet() {
	//TODO start recursively interpret tree
	XSLTAnalyzer p;
	getRoot()->pruneComments();
	return p.Document(root);
	//return parsingXSLT::XSLTParser::Stylesheet(root);
}


