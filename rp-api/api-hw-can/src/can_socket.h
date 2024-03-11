/**
 * $Id: $
 *
 * @brief Red Pitaya CAN socket
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef _CAN_SOCKET_H_
#define _CAN_SOCKET_H_

auto socket_Open(rp_can_interface_t _interface) -> int;
auto socket_Close(rp_can_interface_t _interface) -> int;
auto socket_Send(rp_can_interface_t _interface, uint32_t _canId, unsigned char *_data, uint8_t _dataSize, bool _isExtended, bool _rtr, uint32_t _timeout) -> int;
auto socket_Read(rp_can_interface_t _interface, uint32_t _timeout,rp_can_frame_t *_frame) -> int;
auto socket_AddFilter(rp_can_interface_t _interface, uint32_t _filter, uint32_t _mask) -> int;
auto socket_RemoveFilter(rp_can_interface_t _interface, uint32_t _filter, uint32_t _mask) -> int;
auto socket_ClearFilter(rp_can_interface_t _interface) -> int;
auto socket_SetFilter(rp_can_interface_t _interface, bool _isJoinFilter) -> int;
auto socket_ShowErrorFrames(rp_can_interface_t _interface, bool _enable) -> int;

#endif // _CAN_SOCKET_H_