#pragma once

//#define _DEBUG_XMLATTRIBUTE
//#define _DEBUG_XMLNODE
//#define _DEBUG_XMLCOMMENT
//#define _DEBUG_XMLCDATA
//#define _DEBUG_XMLINNER
//#define _DEBUG_XMLTREE



#ifdef  _DEBUG_XMLATTRIBUTE
#define DEBUG_OUT_ATTR(s) std::wcout << s << std::endl;
#else
#define DEBUG_OUT_ATTR(s) ;
#endif

#ifdef  _DEBUG_XMLNODE
#define DEBUG_OUT_NODE(s) std::wcout << s << std::endl;
#else
#define DEBUG_OUT_NODE(s) ;
#endif

#ifdef  _DEBUG_XMLCOMMENT
#define DEBUG_OUT_COMMENT(s) std::wcout << s << std::endl;
#else
#define DEBUG_OUT_COMMENT(s) ;
#endif

#ifdef  _DEBUG_XMLCDATA
#define DEBUG_OUT_CDATA(s) std::wcout << s << std::endl;
#else
#define DEBUG_OUT_CDATA(s) ;
#endif

#ifdef  _DEBUG_XMLINNER
#define DEBUG_OUT_INNER(s) std::wcout << s << std::endl;
#else
#define DEBUG_OUT_INNER(s) ;
#endif

#ifdef  _DEBUG_XMLTREE
#define DEBUG_OUT_TREE(s) std::wcout << s << std::endl;
#else
#define DEBUG_OUT_TREE(s) ;
#endif