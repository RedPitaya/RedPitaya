#include <stddef.h>
#include <iostream>
#include "XMLNode.h"
#include "XMLCommon.h"
#include "debug.h"

namespace XML
{
	XMLNode::XMLNode(XMLString &_name){
		nameFull = std::move(_name);
		attributes = NULL;
		childNodes = new NodeVector();
		inner_text_list = new std::vector<InnerTextStruct*>();
		parent = NULL;
		DEBUG_OUT_NODE(L"Create node: " + nameFull.toWString());
	}

	XMLNode::~XMLNode(){
		if (inner_text_list != NULL) DestoryVector<std::vector<InnerTextStruct*>>(inner_text_list);
		inner_text_list = NULL;

		if (attributes!=NULL) DestoryVector<AttibuteVector>(attributes);
		attributes = NULL;

		if (childNodes!= NULL) DestoryVector<NodeVector>(childNodes);
		childNodes = NULL;

		DEBUG_OUT_NODE(L"Destroy node: " + nameFull.toWString());
	}

	void XMLNode::SetAttributes(AttibuteVector* _attributes){
		if (attributes!= NULL) DestoryVector<AttibuteVector>(attributes);
		attributes = _attributes;
	}

	const AttibuteVector* XMLNode::GetAttributes(){
		return attributes;
	}

    XMLAttribute*  XMLNode::GetAttributesByName(XMLString &_name){
        for (auto attr : *GetAttributes()){
            if (attr->NameXS() == _name)
                return attr;
        }
        return nullptr;
	}

	const NodeVector * XMLNode::GetChildNodes(){
		return childNodes;
	}

	void XMLNode::AddChild(XMLNode * _node){
		_node->parent = this;		
		childNodes->push_back(_node);
	}

	void XMLNode::ClearChildsWithoutDedtroy(){
		childNodes->clear();
	}

	std::wstring XMLNode::GetInnerText(){
		if (inner_text_list!= NULL){
			std::wstring s = L"";
			for (unsigned int i = 0; i < inner_text_list->size(); i++){
					s += inner_text_list->at(i)->text;
			}
			return s;
		}
		return std::wstring();
	}

	XMLProlog::XMLProlog(){}

	XMLProlog::~XMLProlog(){
		DestoryVector<AttibuteVector>(attributes);
	}

	std::wstring XMLProlog::ToWString(){
		std::wstring s = L"<?xml ";
		for (unsigned int i = 0 ; i <  attributes->size();i++){
			s += attributes->at(i)->toWString() + L" ";
		}
		s += L"?>";
		return s;
	}
}