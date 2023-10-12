#include <stdio.h>
#include <map>
#include <list>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <net/if.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <poll.h>
#include <time.h>


#include "rp_hw_can.h"
#include "can_socket.h"
#include "common.h"

std::map<rp_can_interface_t, int> g_sockets;
std::map<rp_can_interface_t, std::list<std::pair<uint32_t,uint32_t>>> g_filters;

// return clock in ms
auto getClock() -> double {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double)tp.tv_sec * 1000.f) + ((double)tp.tv_nsec / 1000000.f);
}

auto socket_Open(rp_can_interface_t _interface) -> int{
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
	struct sockaddr_can addr;
	struct ifreq ifr;

    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,"")) 
        return RP_HW_CAN_EUI;

    if (auto search = g_sockets.find(_interface); search != g_sockets.end()){
        return RP_HW_CAN_ESA;
    }

	auto s = socket(family, type, proto);
	if (s < 0) {
		return RP_HW_CAN_ESO;
	}

    addr.can_family = family;
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(s, SIOCGIFINDEX, &ifr)) {
		return RP_HW_CAN_ESO;
	}
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		return RP_HW_CAN_ESB;
	}

    g_sockets[_interface] = s;
    return RP_HW_CAN_OK;
}

auto socket_Close(rp_can_interface_t _interface) -> int {
    if (auto search = g_sockets.find(_interface); search != g_sockets.end()){
        auto s = g_sockets[_interface];
        if (close(s)){
            return RP_HW_CAN_ESC;
        }
        g_sockets.extract(_interface);
        return RP_HW_CAN_OK;
    }
    return RP_HW_CAN_ESN;
}

auto socket_Send(rp_can_interface_t _interface, uint32_t _canId, unsigned char *_data, uint8_t _dataSize, bool _isExtended, bool _rtr, uint32_t _timeout) -> int {
    struct can_frame frame;
    
    if (!_data){
        return RP_HW_CAN_ESD;
    }

    if (auto search = g_sockets.find(_interface); search == g_sockets.end()){
        return RP_HW_CAN_ESA;
    }
    auto s = g_sockets[_interface];
    auto dlc =0u;

    memset(&frame,0,sizeof(can_frame));
    frame.can_id = _canId;

    for (auto i = 0u; i < _dataSize; i++) {
		frame.data[dlc] = _data[i];
		dlc++;
		if (dlc == 8)
			break;
	}
	frame.can_dlc = dlc;

    if (_isExtended) {
		frame.can_id &= CAN_EFF_MASK;
		frame.can_id |= CAN_EFF_FLAG;
	} else {
		frame.can_id &= CAN_SFF_MASK;
	}

	if (_rtr)
		frame.can_id |= CAN_RTR_FLAG;

    while (true) {
		auto len = write(s, &frame, sizeof(frame));
		if (len == -1) {
			switch (errno) {
                case ENOBUFS: {
                    struct pollfd fds = {
                        .fd = s,
                        .events = POLLOUT,
                        .revents = 0
                    };

                    if (_timeout == 0) {
                        return RP_HW_CAN_ESBO;
                    }                   

                    auto ret = poll(&fds, 1, _timeout);
                    if (ret == -1 && errno != -EINTR) {
                        return RP_HW_CAN_ESPE;
                    }
                    if (ret == 0) {
                        return RP_HW_CAN_ESTE;
                    }
                    break;
                }
                case EINTR:	/* fallthrough */
                    break;
                default:
                    return RP_HW_CAN_ESE;
			}
		}else{
            break;         
        }
	}
    return RP_HW_CAN_OK;
}

auto socket_Read(rp_can_interface_t _interface, uint32_t _timeout,rp_can_frame_t *_frame) -> int {
    can_frame frame;
    int nbytes;

    if (auto search = g_sockets.find(_interface); search == g_sockets.end()){
        return RP_HW_CAN_ESA;
    }

    auto s = g_sockets[_interface];

    if (_timeout > 0){
        struct pollfd fds = {
                            .fd = s,
                            .events = POLLIN,
                            .revents = 0
                        };
        
        auto ret = poll(&fds, 1, _timeout);
        switch (ret) {
            case -1:
                if (errno != -EINTR) {
                    return RP_HW_CAN_ESPE;
                }
                break;
            case 0:
                return RP_HW_CAN_ESTE;
            default:                
                break;
        }
        nbytes = read(s, &frame, sizeof(struct can_frame));
    }else{
        nbytes = read(s, &frame, sizeof(struct can_frame));
    }

    if (nbytes < 0 || nbytes != sizeof(struct can_frame)){
        return RP_HW_CAN_ESR;
    }
    _frame->is_extended_format = frame.can_id & CAN_EFF_FLAG;
    if (_frame->is_extended_format){
		_frame->can_id  = frame.can_id & CAN_EFF_MASK;
    }
	else {
    	_frame->can_id = frame.can_id & CAN_SFF_MASK;
    }
    _frame->can_id_raw = frame.can_id;
    _frame->is_error_frame = frame.can_id & CAN_ERR_FLAG;
    _frame->is_remote_request = frame.can_id & CAN_RTR_FLAG;
    _frame->can_dlc = frame.can_dlc;
    for (auto i = 0u; i < frame.can_dlc; i++) {
        _frame->data[i] = frame.data[i];
    }
    return RP_HW_CAN_OK;
}

auto socket_AddFilter(rp_can_interface_t _interface, uint32_t _filter, uint32_t _mask) -> int{
    std::pair<uint32_t,uint32_t> newFilter = {_filter,_mask};
    if (auto search = g_filters.find(_interface); search != g_filters.end()){
        auto& filter = g_filters[_interface];
        auto it = find_if(filter.begin(), filter.end(), [newFilter] (auto s) { return s == newFilter; });
        if (it != filter.end()){
            return RP_HW_CAN_ESFA;
        }
        filter.push_back(newFilter);
        return RP_HW_CAN_OK;
    }else{
        g_filters[_interface] = {};
        g_filters[_interface].push_back(newFilter);
    }
    return RP_HW_CAN_OK;
}

auto socket_RemoveFilter(rp_can_interface_t _interface, uint32_t _filter, uint32_t _mask) -> int{
    std::pair<uint32_t,uint32_t> newFilter = {_filter,_mask};
    if (auto search = g_filters.find(_interface); search != g_filters.end()){
        auto& filter = g_filters[_interface];
        filter.remove(newFilter);
        return RP_HW_CAN_OK;
    }
    return RP_HW_CAN_OK;
}

auto socket_ClearFilter(rp_can_interface_t _interface) -> int{
    g_filters[_interface] = {};
    return RP_HW_CAN_OK;
}

auto socket_SetFilter(rp_can_interface_t _interface, bool _isJoinFilter) -> int{
   if (auto search = g_sockets.find(_interface); search != g_sockets.end()){
        auto s = g_sockets[_interface];


        if (auto search = g_filters.find(_interface); search != g_filters.end()){
            auto& filter = g_filters[_interface];
            if (filter.size()){
                auto c_filter = new can_filter[filter.size()];
                int i = 0;
                for (auto const& it : filter) {
                    c_filter[i].can_id = it.first;
                    c_filter[i].can_mask = it.second;
                }
                if (setsockopt(s, SOL_CAN_RAW, _isJoinFilter ? CAN_RAW_JOIN_FILTERS: CAN_RAW_FILTER, c_filter, sizeof(can_filter) * filter.size())) {
                    return RP_HW_CAN_ESFS;
                }
                delete[] c_filter;
            }else{
                if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0)) {
                    return RP_HW_CAN_ESFS;
                }
            }
        }
        return RP_HW_CAN_OK;
    }
    return RP_HW_CAN_ESN;
}

auto socket_ShowErrorFrames(rp_can_interface_t _interface, bool _enable) -> int{
    if (auto search = g_sockets.find(_interface); search != g_sockets.end()){
        auto s = g_sockets[_interface];
	    
        can_err_mask_t err_mask = _enable ? (CAN_ERR_TX_TIMEOUT | CAN_ERR_LOSTARB |
					CAN_ERR_CRTL | CAN_ERR_PROT |
					CAN_ERR_TRX | CAN_ERR_ACK | CAN_ERR_BUSOFF |
					CAN_ERR_BUSERROR) : 0;

        if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask,
			       sizeof(err_mask)) != 0) {
			return RP_HW_CAN_ESEF;
		}

        return RP_HW_CAN_OK;
    }
    return RP_HW_CAN_ESN;
}