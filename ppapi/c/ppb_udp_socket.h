/* Copyright 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* From ppb_udp_socket.idl modified Tue Mar 17 11:47:56 2015. */

#ifndef PPAPI_C_PPB_UDP_SOCKET_H_
#define PPAPI_C_PPB_UDP_SOCKET_H_

#include "ppapi/c/pp_bool.h"
#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_macros.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/pp_var.h"

#define PPB_UDPSOCKET_INTERFACE_1_0 "PPB_UDPSocket;1.0"
#define PPB_UDPSOCKET_INTERFACE_1_1 "PPB_UDPSocket;1.1"
#define PPB_UDPSOCKET_INTERFACE_1_2 "PPB_UDPSocket;1.2"
#define PPB_UDPSOCKET_INTERFACE PPB_UDPSOCKET_INTERFACE_1_2

/**
 * @file
 * This file defines the <code>PPB_UDPSocket</code> interface.
 */


/**
 * @addtogroup Enums
 * @{
 */
/**
 * Option names used by <code>SetOption()</code>.
 */
typedef enum {
  /**
   * Allows the socket to share the local address to which it will be bound with
   * other processes. Value's type should be <code>PP_VARTYPE_BOOL</code>.
   * This option can only be set before calling <code>Bind()</code>.
   */
  PP_UDPSOCKET_OPTION_ADDRESS_REUSE = 0,
  /**
   * Allows sending and receiving packets to and from broadcast addresses.
   * Value's type should be <code>PP_VARTYPE_BOOL</code>.
   * On version 1.0, this option can only be set before calling
   * <code>Bind()</code>. On version 1.1 or later, there is no such limitation.
   */
  PP_UDPSOCKET_OPTION_BROADCAST = 1,
  /**
   * Specifies the total per-socket buffer space reserved for sends. Value's
   * type should be <code>PP_VARTYPE_INT32</code>.
   * On version 1.0, this option can only be set after a successful
   * <code>Bind()</code> call. On version 1.1 or later, there is no such
   * limitation.
   *
   * Note: This is only treated as a hint for the browser to set the buffer
   * size. Even if <code>SetOption()</code> succeeds, the browser doesn't
   * guarantee it will conform to the size.
   */
  PP_UDPSOCKET_OPTION_SEND_BUFFER_SIZE = 2,
  /**
   * Specifies the total per-socket buffer space reserved for receives. Value's
   * type should be <code>PP_VARTYPE_INT32</code>.
   * On version 1.0, this option can only be set after a successful
   * <code>Bind()</code> call. On version 1.1 or later, there is no such
   * limitation.
   *
   * Note: This is only treated as a hint for the browser to set the buffer
   * size. Even if <code>SetOption()</code> succeeds, the browser doesn't
   * guarantee it will conform to the size.
   */
  PP_UDPSOCKET_OPTION_RECV_BUFFER_SIZE = 3,
  /**
   * Specifies whether the packets sent from the host to the multicast group
   * should be looped back to the host or not. Value's type should be
   * <code>PP_VARTYPE_BOOL</code>.
   * This option can only be set before calling <code>Bind()</code>.
   *
   * This is only supported in version 1.2 of the API (Chrome 43) and later.
   */
  PP_UDPSOCKET_OPTION_MULTICAST_LOOP = 4,
  /**
   * Specifies the time-to-live for packets sent to the multicast group. The
   * value should be within 0 to 255 range. The default value is 1 and means
   * that packets will not be routed beyond the local network. Value's type
   * should be <code>PP_VARTYPE_INT32</code>.
   * This option can only be set before calling <code>Bind()</code>.
   *
   * This is only supported in version 1.2 of the API (Chrome 43) and later.
   */
  PP_UDPSOCKET_OPTION_MULTICAST_TTL = 5
} PP_UDPSocket_Option;
PP_COMPILE_ASSERT_SIZE_IN_BYTES(PP_UDPSocket_Option, 4);
/**
 * @}
 */

/**
 * @addtogroup Interfaces
 * @{
 */
/**
 * The <code>PPB_UDPSocket</code> interface provides UDP socket operations.
 *
 * Permissions: Apps permission <code>socket</code> with subrule
 * <code>udp-bind</code> is required for <code>Bind()</code>; subrule
 * <code>udp-send-to</code> is required for <code>SendTo()</code>.
 * For more details about network communication permissions, please see:
 * http://developer.chrome.com/apps/app_network.html
 */
struct PPB_UDPSocket_1_2 {
  /**
   * Creates a UDP socket resource.
   *
   * @param[in] instance A <code>PP_Instance</code> identifying one instance of
   * a module.
   *
   * @return A <code>PP_Resource</code> corresponding to a UDP socket or 0
   * on failure.
   */
  PP_Resource (*Create)(PP_Instance instance);
  /**
   * Determines if a given resource is a UDP socket.
   *
   * @param[in] resource A <code>PP_Resource</code> to check.
   *
   * @return <code>PP_TRUE</code> if the input is a <code>PPB_UDPSocket</code>
   * resource; <code>PP_FALSE</code> otherwise.
   */
  PP_Bool (*IsUDPSocket)(PP_Resource resource);
  /**
   * Binds the socket to the given address.
   *
   * @param[in] udp_socket A <code>PP_Resource</code> corresponding to a UDP
   * socket.
   * @param[in] addr A <code>PPB_NetAddress</code> resource.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   * <code>PP_ERROR_NOACCESS</code> will be returned if the caller doesn't have
   * required permissions. <code>PP_ERROR_ADDRESS_IN_USE</code> will be returned
   * if the address is already in use.
   */
  int32_t (*Bind)(PP_Resource udp_socket,
                  PP_Resource addr,
                  struct PP_CompletionCallback callback);
  /**
   * Gets the address that the socket is bound to. The socket must be bound.
   *
   * @param[in] udp_socket A <code>PP_Resource</code> corresponding to a UDP
   * socket.
   *
   * @return A <code>PPB_NetAddress</code> resource on success or 0 on failure.
   */
  PP_Resource (*GetBoundAddress)(PP_Resource udp_socket);
  /**
   * Receives data from the socket and stores the source address. The socket
   * must be bound.
   *
   * @param[in] udp_socket A <code>PP_Resource</code> corresponding to a UDP
   * socket.
   * @param[out] buffer The buffer to store the received data on success. It
   * must be at least as large as <code>num_bytes</code>.
   * @param[in] num_bytes The number of bytes to receive.
   * @param[out] addr A <code>PPB_NetAddress</code> resource to store the source
   * address on success.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return A non-negative number on success to indicate how many bytes have
   * been received; otherwise, an error code from <code>pp_errors.h</code>.
   */
  int32_t (*RecvFrom)(PP_Resource udp_socket,
                      char* buffer,
                      int32_t num_bytes,
                      PP_Resource* addr,
                      struct PP_CompletionCallback callback);
  /**
   * Sends data to a specific destination. The socket must be bound.
   *
   * @param[in] udp_socket A <code>PP_Resource</code> corresponding to a UDP
   * socket.
   * @param[in] buffer The buffer containing the data to send.
   * @param[in] num_bytes The number of bytes to send.
   * @param[in] addr A <code>PPB_NetAddress</code> resource holding the
   * destination address.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return A non-negative number on success to indicate how many bytes have
   * been sent; otherwise, an error code from <code>pp_errors.h</code>.
   * <code>PP_ERROR_NOACCESS</code> will be returned if the caller doesn't have
   * required permissions.
   * <code>PP_ERROR_INPROGRESS</code> will be returned if the socket is busy
   * sending. The caller should wait until a pending send completes before
   * retrying.
   */
  int32_t (*SendTo)(PP_Resource udp_socket,
                    const char* buffer,
                    int32_t num_bytes,
                    PP_Resource addr,
                    struct PP_CompletionCallback callback);
  /**
   * Cancels all pending reads and writes, and closes the socket. Any pending
   * callbacks will still run, reporting <code>PP_ERROR_ABORTED</code> if
   * pending IO was interrupted. After a call to this method, no output
   * parameters passed into previous <code>RecvFrom()</code> calls will be
   * accessed. It is not valid to call <code>Bind()</code> again.
   *
   * The socket is implicitly closed if it is destroyed, so you are not
   * required to call this method.
   *
   * @param[in] udp_socket A <code>PP_Resource</code> corresponding to a UDP
   * socket.
   */
  void (*Close)(PP_Resource udp_socket);
  /**
   * Sets a socket option on the UDP socket.
   * Please see the <code>PP_UDPSocket_Option</code> description for option
   * names, value types and allowed values.
   *
   * @param[in] udp_socket A <code>PP_Resource</code> corresponding to a UDP
   * socket.
   * @param[in] name The option to set.
   * @param[in] value The option value to set.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   */
  int32_t (*SetOption)(PP_Resource udp_socket,
                       PP_UDPSocket_Option name,
                       struct PP_Var value,
                       struct PP_CompletionCallback callback);
  /**
   * Joins the multicast group with address specified by <code>group</code>
   * parameter, which is expected to be a <code>PPB_NetAddress</code> object.
   *
   * @param[in] udp_socket A <code>PP_Resource</code> corresponding to a UDP
   * socket.
   * @param[in] group A <code>PP_Resource</code> corresponding to the network
   * address of the multicast group.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   */
  int32_t (*JoinGroup)(PP_Resource udp_socket,
                       PP_Resource group,
                       struct PP_CompletionCallback callback);
  /**
   * Leaves the multicast group with address specified by <code>group</code>
   * parameter, which is expected to be a <code>PPB_NetAddress</code> object.
   *
   * @param[in] udp_socket A <code>PP_Resource</code> corresponding to a UDP
   * socket.
   * @param[in] group A <code>PP_Resource</code> corresponding to the network
   * address of the multicast group.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   */
  int32_t (*LeaveGroup)(PP_Resource udp_socket,
                        PP_Resource group,
                        struct PP_CompletionCallback callback);
};

typedef struct PPB_UDPSocket_1_2 PPB_UDPSocket;

struct PPB_UDPSocket_1_0 {
  PP_Resource (*Create)(PP_Instance instance);
  PP_Bool (*IsUDPSocket)(PP_Resource resource);
  int32_t (*Bind)(PP_Resource udp_socket,
                  PP_Resource addr,
                  struct PP_CompletionCallback callback);
  PP_Resource (*GetBoundAddress)(PP_Resource udp_socket);
  int32_t (*RecvFrom)(PP_Resource udp_socket,
                      char* buffer,
                      int32_t num_bytes,
                      PP_Resource* addr,
                      struct PP_CompletionCallback callback);
  int32_t (*SendTo)(PP_Resource udp_socket,
                    const char* buffer,
                    int32_t num_bytes,
                    PP_Resource addr,
                    struct PP_CompletionCallback callback);
  void (*Close)(PP_Resource udp_socket);
  int32_t (*SetOption)(PP_Resource udp_socket,
                       PP_UDPSocket_Option name,
                       struct PP_Var value,
                       struct PP_CompletionCallback callback);
};

struct PPB_UDPSocket_1_1 {
  PP_Resource (*Create)(PP_Instance instance);
  PP_Bool (*IsUDPSocket)(PP_Resource resource);
  int32_t (*Bind)(PP_Resource udp_socket,
                  PP_Resource addr,
                  struct PP_CompletionCallback callback);
  PP_Resource (*GetBoundAddress)(PP_Resource udp_socket);
  int32_t (*RecvFrom)(PP_Resource udp_socket,
                      char* buffer,
                      int32_t num_bytes,
                      PP_Resource* addr,
                      struct PP_CompletionCallback callback);
  int32_t (*SendTo)(PP_Resource udp_socket,
                    const char* buffer,
                    int32_t num_bytes,
                    PP_Resource addr,
                    struct PP_CompletionCallback callback);
  void (*Close)(PP_Resource udp_socket);
  int32_t (*SetOption)(PP_Resource udp_socket,
                       PP_UDPSocket_Option name,
                       struct PP_Var value,
                       struct PP_CompletionCallback callback);
};
/**
 * @}
 */

#endif  /* PPAPI_C_PPB_UDP_SOCKET_H_ */

