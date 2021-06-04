#pragma once
#include "XMLNode.h"
#include "XMLNameSpace.h"
#include "XMLString.h"


namespace XML
{
	class XMLWriter;

	typedef std::vector<XMLNameSpace*>NameSpaceVector;

	class XMLDocument
	{
		XMLProlog *prolog;
		XMLNode   *root;
		NameSpaceVector *namespaces;
		friend class XMLWriter;
	public:
		XMLDocument();
		~XMLDocument();
		void SetProlog(XMLProlog *Prolog);		
		void SetRoot(XMLNode* _root);
		XMLNode* Root();

		bool    AddNameSpaces(NameSpaceVector *_vector);
		//bool IsContain(XMLNode *_name, const char* name, int nameLenght);

		XMLNode* FindFirstNodeByName(XMLString &node_name);
	};
}

