/**
 * $Id$
 *
 * @brief Red Pitaya Web module
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef __RP_WEBSOCKET_H__
#define __RP_WEBSOCKET_H__


namespace rp_websocket{

class CWEBServer{

public:


    CWEBServer();
    ~CWEBServer();


private:

    CWEBServer(const CWEBServer &) = delete;
    CWEBServer(CWEBServer &&) = delete;
    CWEBServer& operator=(const CWEBServer&) =delete;
    CWEBServer& operator=(const CWEBServer&&) =delete;

    class Impl;
    // Pointer to the internal implementation
    Impl *m_pimpl;
};

}

#endif

