#include "XMLDocument.h"
#include "XMLCommon.h"

namespace XML
{
	XMLDocument::XMLDocument(){
		prolog = nullptr;
		root = nullptr;
		namespaces = new NameSpaceVector();
	}

	XMLDocument::~XMLDocument(){
		delete prolog;
		prolog = nullptr;

		delete root;
		root = nullptr;

		if (namespaces) DestoryVector<NameSpaceVector>(namespaces);
		namespaces = nullptr;
	}

	void XMLDocument::SetProlog(XMLProlog *Prolog){
		delete prolog;
		prolog = Prolog;
	}

	void XMLDocument::SetRoot(XMLNode* _root){
		delete root;
		root = _root;
	}

	XMLNode* XMLDocument::Root(){
		return root;
	}

	bool XMLDocument::AddNameSpaces(NameSpaceVector *_vector){
		namespaces->insert(namespaces->end(), _vector->begin(), _vector->end());
		return true;
	}

	XMLNode* FindNode(XMLNode *node,XMLString &str){
	    if (node->nameFull == str) return  node;
        auto nodes = node->GetChildNodes();
        for (auto child : *nodes){
            XMLNode *sub_node = FindNode(child,str);
            if (sub_node != nullptr)
                return  sub_node;
        }
        return nullptr;
	}

    XMLNode* XMLDocument::FindFirstNodeByName(XMLString &node_name){
        return  FindNode(Root(),node_name);
	}
}