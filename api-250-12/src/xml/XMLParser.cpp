#include <string>
#include <iostream>
#include <wchar.h>
#include <stdio.h>
#include "XMLParser.h"
#include "XMLCommon.h"
#include "debug.h"

namespace XML
{
	XMLParser::XMLParser(){
		nodeoflist = new NodeVector();
		namespacevector = NULL;
	}

	XMLParser::~XMLParser(){
		ErrorList.clear();
		nodes_stek.clear();
		if (nodeoflist!= NULL) delete nodeoflist;
		if (namespacevector!= NULL) delete namespacevector;
	}

	int XMLParser::GetError(){
		return Error;
	}

	std::vector<std::wstring>& XMLParser::GetErrorList(){
		return ErrorList;
	}
	
	XML::XMLDocument* XMLParser::ParseXML(char *_buffer, int BufferLenght){
		if (namespacevector) DestoryVector<NameSpaceVector>(namespacevector);
		
		namespacevector = new NameSpaceVector();
		nodes_stek.clear();

		XMLDocument *document = new XMLDocument();
		try
		{
			Error = 0;
			buffer = new UTF8Buffer(_buffer, BufferLenght);
			if (!buffer->ReadBOM()){
				delete buffer;
				buffer = new Buffer(_buffer, BufferLenght);
			}
						
			// Check prolog <?xml version="1.0" encoding="UTF-8"?>
			XMLProlog* prolog = ParseProlog();
			if (prolog){
				CheckProlog(prolog);
				document->SetProlog(prolog);
			}

			XMLNode* node = ParseXMLRootNode();			
	
			if (Error != 0){
				for (int i = 0; i < (int)nodeoflist->size(); i++){
					nodeoflist->at(i)->ClearChildsWithoutDedtroy();
					delete nodeoflist->at(i);
				}
				delete nodeoflist;
				nodeoflist = NULL;
				DestoryVector(namespacevector);
				namespacevector = NULL;
			}
			else{
				document->SetRoot(node);
				document->AddNameSpaces(namespacevector);
			}
		}
		catch(std::exception &ex)
		{
			AddException(ex, 1000);
		}
		delete buffer;
		return document;
	}

	XMLProlog*  XMLParser::ParseProlog(){
		XMLProlog *prolog = NULL;

		buffer->ReadNextSkipChars();
		char *prologBegin = buffer->ReadANSI(5);
		if (strncmp(prologBegin, "<?xml", 5) == 0){
			buffer->MoveNext(5);
			int posendprolog = buffer->FindSubANSIString("?>", 2);

			if (posendprolog == -1){
				Error = 3;
				ErrorList.emplace_back(L"Error get end of XML prolog");
				buffer->End();
			}
			else{
				int attributelenght = posendprolog - buffer->GetPosition();
				char *attributbuf = buffer->GetSubBuffer(buffer->GetPosition(), attributelenght);

				AttibuteVector *attributes = ParseAttribute(attributbuf, attributelenght);

				if (attributes){
					prolog = new XMLProlog();
					prolog->attributes = attributes;
					buffer->MoveNext(attributelenght+2);
				}
				else{
					Error = 3;
					ErrorList.emplace_back(L"Error get attrubutes in XML prolog");
					buffer->End();
				}
				delete[] attributbuf;
			}
		}
		delete[] prologBegin;
		return prolog;
	}

	void XMLParser::CheckProlog(XMLProlog *prolog){
		if (prolog->attributes->size()!= 2){
			Error = 3;
			ErrorList.emplace_back(L"Error get attrubutes in XML prolog: Count of attibutes is wrong");
		}

		bool found_version = false;
		bool found_encoding = false;

		for(unsigned int i = 0 ; i < prolog->attributes->size(); ++i){
			auto str = prolog->attributes->at(i)->Name()+L"\0";
			auto ch = str.c_str();
			if (wcscmp(ch, L"version") == 0) found_version = true;
			if (wcscmp(ch, L"encoding") == 0) found_encoding = true;
		}
		
		if (!found_encoding || !found_version){
			Error = 3;
			ErrorList.push_back(L"Error get attrubutes in XML prolog: Don't find encoding or version");
		}
	}
	
	AttibuteVector* XMLParser::ParseAttribute(char *_buffer, int BufferLenght){
		Buffer attributebuf(_buffer, BufferLenght);
		attributebuf.TrimRight();
		if (attributebuf.FindSubANSIChar('<') != -1){
			Error = 2;
			ErrorList.push_back(L"Error parse xml attribute name: Unexpected char <");
			return NULL;
		}

		AttibuteVector* attributes = new AttibuteVector();
		while (!attributebuf.IsEnd()){
//			int lenghtname = 0;
			XMLString *Name = ParseAttributName(attributebuf);
			if (!Name){
				DestoryVector<AttibuteVector>(attributes);
				Error = 2;
				ErrorList.emplace_back(L"Error parse xml attribute name");
				return nullptr;
			}
			
			if (IsPunctuationChar(Name->toWString()[0]) || IsNumber(Name->getText()[0]) || !CheckFormatWithXMLNS(*Name)){
				DestoryVector<AttibuteVector>(attributes);
				Error = 101;
				ErrorList.push_back(L"Error parse xml attribute name: " + Name->toWString() + L" wrong char");
				delete Name;
				return nullptr;
			}

			char Quote = 0;
			XMLString *Value = ParseAttributValue(attributebuf,Quote);
			if (!Value){
				DestoryVector<AttibuteVector>(attributes);
				Error = 2;
				ErrorList.emplace_back(L"Error parse xml attribute value");
				delete Name;
				return nullptr;
			}

			if (CheckPrefixForXMLNS(*Name)){
				XMLNameSpace *xmlns = new XMLNameSpace(*Name,*Value, Quote);
				namespacevector->push_back(xmlns);
				delete Name;
				delete Value;
				DEBUG_OUT_ATTR(xmlns->toWString());
			}
			else{
				if (CheckPrefixForXML(*Name)){
					Error = 201;
					ErrorList.push_back(L"Error parse xml name: " + Name->toWString() + L" find prefix xml");
					DestoryVector<AttibuteVector>(attributes);
					delete Value;
					delete Name;
					return nullptr;
				}

				XMLAttribute *attributeitem = new XMLAttribute(*Name, *Value, Quote);
				delete Name;
				delete Value;
				attributes->push_back(attributeitem);
				DEBUG_OUT_ATTR(attributeitem->toWString());
			}
		}
		return attributes;
	}

	XMLString* XMLParser::ParseAttributName(Buffer &buffer){
		int EndName = buffer.FindSubANSIString("=", 1);
		if (EndName == -1){
			Error = 6;
			ErrorList.emplace_back(L"Error parse attribute name: Don't find '='");
			return nullptr;
		}
		
		int LenghtName = EndName - buffer.GetPosition();
		char *Name = new char[LenghtName];

		try
		{
			buffer.WriteToBuffer(Name, LenghtName);
		}
		catch (std::exception &ex)
		{
			AddException(ex, 61);
			delete[] Name;
			return nullptr;
		}

		char *TrimName = Trim(Name, LenghtName, LenghtName);
		delete[] Name;
		auto str = new XMLString(TrimName, LenghtName);
        delete[](TrimName);
		return  str;
	}

	XMLString * XMLParser::ParseAttributValue(Buffer &buffer, char &Quote){
		char ch =  buffer.ReadANSI();
		if (ch != '=') return nullptr;

		unsigned int BeginQuote1 = buffer.FindSubANSIString("\"", 1);
		unsigned int BeginQuote2 = buffer.FindSubANSIString("'" , 1);
		
		Quote = '\"';
        unsigned int  StartQuote = BeginQuote1;
		if (BeginQuote1 > BeginQuote2){
			Quote = '\'';
			StartQuote = BeginQuote2;
		}
		
		buffer.Seek(StartQuote + 1);
		int EndQuote = buffer.FindSubANSIChar(Quote);

		if (EndQuote == -1){
			Error = 76;
			ErrorList.emplace_back(L"Error parse attribute: Don't find end quote");
			return nullptr;
		}

		int LenghtValue = EndQuote - buffer.GetPosition();
		char *Value = new char[LenghtValue];

		try
		{
			buffer.WriteToBuffer(Value, LenghtValue);
		}
		catch (std::exception &ex)
		{
			AddException(ex, 77);
			delete[] Value;
			return nullptr;
		}
		buffer.MoveNext(1);
		auto str = new XMLString(Value,LenghtValue);
		delete[] Value;
        return  str;
	}

	char* XMLParser::Trim(char *Buffer, int BufferLenght, int &NewBufferLenght){
		NewBufferLenght = 0;
		for (int i = 0; i < BufferLenght; i++){
			if (!IsSpaceChar(Buffer[i])){
				NewBufferLenght++;
			}
		}

		char *newBuffer = new char[NewBufferLenght];
		NewBufferLenght = 0;
		for (int i = 0; i < BufferLenght; i++){
			if (!IsSpaceChar(Buffer[i])){
				newBuffer[NewBufferLenght++] = Buffer[i];
			}
		}
		return newBuffer;
	}

	XMLNode* XMLParser::ParseXMLRootNode(){
		int   countRootsNode = 0;
		XMLNode* rootNode = nullptr;

		while (!buffer->IsEnd()){
			buffer->ReadNextSkipChars();
			char *TAG = buffer->ReadANSI(4);
			if (strncmp(TAG, "<!--", 4) == 0){
				ParseXMLComment();
			}
			else
				if (strncmp(TAG, "<", 1) == 0){
					rootNode = ParseXMLNode(NULL);
					if (rootNode!=NULL) nodeoflist->push_back(rootNode);
					countRootsNode++;
				}

			if (countRootsNode > 1){
				Error = 4;
				ErrorList.push_back(L"Error parse XML: More then 1 root node");
				delete TAG;
				break;
			}
			delete[] TAG;
		}
		return rootNode;
	}

	XMLNode* XMLParser::ParseXMLNode(XMLNode* _parentNode){
		XMLNode *node = nullptr;

		while (!buffer->IsEnd()){
			buffer->ReadNextSkipChars();
			char *TAG = buffer->ReadANSI(9);
//			if (TAG[0] != '\0')
			{
				if (strncmp(TAG, "<![CDATA[", 9) == 0){
					std::wstring cdataString = ParseXMLReadCDATA();
					if (_parentNode == nullptr){
						Error = 5;
						ErrorList.push_back(L"Error parse XML: CDATA present in root xml");
						buffer->End();
					}
					else{
						XMLNode::InnerTextStruct *its = new XMLNode::InnerTextStruct;
						its->text = cdataString;
						its->indexChildText = -1;
						_parentNode->inner_text_list->push_back(its);
					}
				}
				else
					if (strncmp(TAG, "<!--", 4) == 0){
						ParseXMLComment();
					}
					else
						if (strncmp(TAG, "</", 2) == 0){
							auto end_node_text = ParseXMLEndNode();
							if (end_node_text != nodes_stek.back()){
								Error = 5;
								ErrorList.push_back(L"Error parse XML: Not end " + end_node_text + L" of " + nodes_stek.back());
								buffer->End();
							}
							else{
								if (_parentNode != nullptr){
									nodes_stek.pop_back();
									DEBUG_OUT_TREE(L"</" + end_node_text + L">");
									DEBUG_OUT_TREE(L"Stek size = " << nodes_stek.size());
								}
								delete[] TAG;
								break;
							}
						}
						else
							if (strncmp(TAG, "<", 1) == 0){
								node = ParseXMLNodeName(_parentNode);
								if (node == nullptr){
									Error = 6;
									ErrorList.emplace_back(L"Error parse XML: Error in node");
									buffer->End();
								}
								else
									if (_parentNode != nullptr){
										_parentNode->AddChild(node);
										nodeoflist->push_back(node);

										auto *its = new XMLNode::InnerTextStruct;
										its->indexChildText = _parentNode->GetChildNodes()->size()-1;
										_parentNode->inner_text_list->push_back(its);
									}
								if (nodes_stek.size() == 0){
									delete[] TAG;
									break;
								}
							}
							else{
								if (_parentNode == nullptr){
									Error = 5;
									ErrorList.emplace_back(L"Error parse XML: Don't find xml text in root document");
									buffer->End();
								}
								else{
									auto *its = new XMLNode::InnerTextStruct;
									its->text = ParseXMLInnerText();
									its->indexChildText = -1;
									_parentNode->inner_text_list->push_back(its);
									DEBUG_OUT_INNER(_parentNode->GetInnerText());
								}
							}
			}
			delete[] TAG;
		}
		return node;
	}

	XMLNode * XMLParser::ParseXMLNodeName(XMLNode * _parentNode){
		buffer->MoveNext(1);
		int endName = -1;
		int i = buffer->GetPosition();
		while (!buffer->IsEnd()){
			char ch = buffer->ReadNextANSI();
			if (IsSpaceChar(ch) || ch == '/' || ch == '>'){
				endName = buffer->GetPosition()-1;
				buffer->Seek(i);
				break;
			}			

			if (ch == '<' || ch == '&'  ){
				Error = 6;
				ErrorList.emplace_back(L"Error parse XML: Unexpected char < or &");
				buffer->End();
				return nullptr;
			}
		}

		if (endName == -1){
			Error = 6;
			ErrorList.emplace_back(L"Error parse XML: Don't find end node name");
			buffer->End();
			return nullptr;
		}

		int lenght = endName - buffer->GetPosition();
		char *node_name_char = new char[lenght];
		try
		{
			buffer->WriteToBuffer(node_name_char, lenght);
		}
		catch (std::exception &ex)
		{
			AddException(ex, 6);
			delete[] node_name_char;
			return nullptr;
		}
		auto *node_name = new XMLString(node_name_char, lenght);
		delete[](node_name_char);
		if (IsPunctuationChar(node_name->toWString()[0]) || IsNumber(node_name->getText()[0]) || CheckPrefixForXML(*node_name) || !CheckFormatWithXMLNS(*node_name)){
			Error = 102;
			if (CheckPrefixForXML(*node_name)){
				ErrorList.push_back(L"Error parse xml name: " + node_name->toWString() + L" find prefix xml");
			}
			else{
				ErrorList.push_back(L"Error parse xml name: " + node_name->toWString() + L" wrong char");
			}
			delete node_name;
			return nullptr;
		}

		auto* node = new XMLNode(*node_name);
		delete node_name;
		
		DEBUG_OUT_TREE(L"<" + node->nameFull.toWString() + L">");

		buffer->ReadNextSkipChars();

		char *ch = buffer->ReadANSI(2);
		if (ch[0] !=  '/' && ch[0] != '>'){
			int posendattr1 = buffer->FindSubANSIString("/>", 2);
			int posendattr2 = buffer->FindSubANSIChar('>');
			int posendattr = (posendattr1 != -1? posendattr1: posendattr2);
			if (posendattr2 != -1 && posendattr2 < posendattr)
				posendattr = posendattr2;

			if (posendattr == -1){
				Error = 6;
				ErrorList.push_back(L"Error parse XML: Don't find end of " + node->nameFull.toWString());
				buffer->End(); 
			}
			else{
				int attributelenght = posendattr - buffer->GetPosition();
				char *attributbuf = buffer->GetSubBuffer(buffer->GetPosition(), attributelenght);

				AttibuteVector *attributes = ParseAttribute(attributbuf, attributelenght);

				if (attributes){					
					node->SetAttributes(attributes);
					buffer->MoveNext(attributelenght);
				}
				else{
					Error = 6;
					ErrorList.push_back(L"Error parse XML: Error get attribute of " + node->nameFull.toWString());
					buffer->End();
				}

				delete[] attributbuf;
			}
		}
		delete[] ch;
		
		buffer->ReadNextSkipChars();
		char* ch_node_inner = buffer->ReadANSI(2);
		if (ch_node_inner[0] == '>'){
			buffer->MoveNext(1);
			nodes_stek.push_back(node->nameFull.toWString());
			DEBUG_OUT_TREE(L"Stek size = " << nodes_stek.size());			
			ParseXMLNode(node);
		}
		if (strncmp(ch_node_inner, "/>", 2) == 0){
			buffer->MoveNext(2);
		}
		delete[] ch_node_inner;
		return node;
	}

	bool   XMLParser::ParseXMLComment(){
		buffer->MoveNext(4);
		int endComment = buffer->FindSubANSIString("-->", 3);
		if (endComment == -1){
			Error = 5;
			ErrorList.emplace_back(L"Error parse XML: Don't find end comment");
			buffer->End();
			return false;
		}

#ifdef _DEBUG_XMLCOMMENT
		int lenght = endComment - buffer->GetPosition();
		char *Name = new char[lenght];

		try
		{
			buffer->WriteToBuffer(Name, lenght);
		}
		catch (std::exception &ex)
		{
			AddException(ex, 5);
			delete Name;
			return false;
		}

		DEBUG_OUT_COMMENT(L"Comment block: " << GetWString(Name, lenght));
		delete Name;
#endif // _DEBUG_XMLCOMMENT

		buffer->Seek(endComment + 3);
		return true;
	}

	std::wstring XMLParser::ParseXMLReadCDATA(){
		
		int endCDATA = buffer->FindSubANSIString("]]>", 3);

		if (endCDATA == -1){
			Error = 6;
			ErrorList.push_back(L"Error parse XML: Don't find end CDATA");
			buffer->End();
			return NULL;
		}

		endCDATA += 3;

		int lenght = endCDATA - buffer->GetPosition();
		char *Name = new char[lenght];

		try
		{
			buffer->WriteToBuffer(Name, lenght);
		}
		catch (std::exception &ex)
		{
			AddException(ex, 6);
			delete Name;
			return NULL;
		}

		std::wstring wstr = GetWString(Name, lenght);
		buffer->Seek(endCDATA);

		delete Name;

		DEBUG_OUT_CDATA(L"CDATA block: " << wstr);
		
		return wstr;
	}

	std::wstring XMLParser::ParseXMLInnerText(){
		int EndName = buffer->FindSubANSIString("<", 1);
		if (EndName == -1){
			Error = 8;
			ErrorList.emplace_back(L"Error parse end inner text: Don't find <");
			return L"";
		}

		int LenghtName = EndName - buffer->GetPosition();
		char *Name = new char[LenghtName];

		try
		{
			buffer->WriteToBuffer(Name, LenghtName);
		}
		catch (std::exception &ex)
		{
			AddException(ex, 8);
			delete[] Name;
			return L"";
		}
		std::wstring ret_string = GetWString(Name, LenghtName);
		delete[] Name;
		return ret_string;
	}
	
	std::wstring XMLParser::ParseXMLEndNode(){
		buffer->MoveNext(2);
		int EndName = buffer->FindSubANSIString(">", 1);
		if (EndName == -1){
			Error = 7;
			ErrorList.emplace_back(L"Error parse node end name: Don't find >");
			return L"";
		}

		int LenghtName = EndName - buffer->GetPosition();
		char *Name = new char[LenghtName];

		try
		{
			buffer->WriteToBuffer(Name, LenghtName);
		}
		catch (std::exception &ex)
		{
			AddException(ex, 7);
			delete[] Name;
			return L"";
		}

		char *TrimName = Trim(Name, LenghtName, LenghtName);
		delete[] Name;
		buffer->MoveNext(1);
		std::wstring str_ret = GetWString(TrimName, LenghtName);
		delete[] TrimName;
		return str_ret;
	}

	void XMLParser::AddException(std::exception &_ex, int _errorCode){
		Error = _errorCode;
		std::string s(_ex.what());
		std::wstring ws;
		ws.assign(s.begin(), s.end());
		ErrorList.push_back(ws);
	}


	bool XMLParser::CheckPrefixForXML(XMLString &_text){
		if (_text.Lenght() < 3) return false;
		const char *text = _text.getText();
		if (std::tolower(text[0], std::locale()) == 'x' &&
			std::tolower(text[1], std::locale()) == 'm' &&
			std::tolower(text[2], std::locale()) == 'l')
			return true;
		return false;
	}

	bool XMLParser::CheckPrefixForXMLNS(XMLString &_text){
		if (_text.Lenght() < 5) return false;
		const char *text = _text.getText();
		if (std::tolower(text[0], std::locale()) == 'x' &&
			std::tolower(text[1], std::locale()) == 'm' &&
			std::tolower(text[2], std::locale()) == 'l'	&&
			std::tolower(text[3], std::locale()) == 'n'	&&
			std::tolower(text[4], std::locale()) == 's')
			return true;
		return false;
	}

	bool XMLParser::CheckFormatWithXMLNS(XMLString &_text)
	{
		if (_text.Lenght() < 2) return true;
		
		const char *text = _text.getText();
		int position = -1;
		for (int i = 0; i < _text.Lenght(); i++){
			if (text[i] == ':'){
				position = i;
				break;
			}
		}

		if (position == -1) return true;

		if (_text.Lenght() <= position + 1){
			return false;
		}
		return true;
	}
}