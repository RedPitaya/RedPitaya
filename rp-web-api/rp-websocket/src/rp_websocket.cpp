#include <jsoncpp/json/json.h>
#include <stdint.h>

#include "rp_websocket.h"

using namespace rp_websocket;

class CWEBServer::Impl {

};

CWEBServer::CWEBServer(){
    m_pimpl = new CWEBServer::Impl();
}

CWEBServer::~CWEBServer(){

}