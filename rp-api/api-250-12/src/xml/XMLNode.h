#pragma once

#include <vector>
#include "XMLAttribute.h"
#include "XMLString.h"

namespace XML
{
	class XMLNode;
	class XMLParser;
	class XMLWriter;

	typedef std::vector<XMLAttribute*> AttibuteVector;
	typedef std::vector<XMLNode*> NodeVector;

	class XMLNode
	{
	public:
		
	private:
		struct InnerTextStruct
		{
			std::wstring text;
			int indexChildText;
		};
		NodeVector					  * childNodes;
		AttibuteVector				  * attributes;
		std::vector<InnerTextStruct*> * inner_text_list;
		
								XMLNode() = delete;
								XMLNode(const XMLNode &v) = delete;
								XMLNode& operator = (XMLNode &v1) = delete;
	friend class XMLParser;
	friend class XMLWriter;
	public:
		XMLString			  nameFull;
		XMLNode			    * parent;
		
							  XMLNode(XMLString &_name);
		                     ~XMLNode();
		void                  SetAttributes(AttibuteVector* _attributes);
		const AttibuteVector* GetAttributes();
		XMLAttribute*   GetAttributesByName(XMLString &_name);
		const NodeVector    * GetChildNodes();
		void                  AddChild(XMLNode* _node);
		void				  ClearChildsWithoutDedtroy();
		std::wstring          GetInnerText();
		
	};

	class XMLProlog
	{
	public:
		AttibuteVector * attributes;
		XMLProlog();
		~XMLProlog();
		std::wstring ToWString();
	};
}

