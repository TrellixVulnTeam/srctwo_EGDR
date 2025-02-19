/* Copyright 2016 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* From ppb_vpn_provider.idl modified Fri May  6 20:42:01 2016. */

#ifndef PPAPI_C_PPB_VPN_PROVIDER_H_
#define PPAPI_C_PPB_VPN_PROVIDER_H_

#include "ppapi/c/pp_bool.h"
#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_macros.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/pp_var.h"

#define PPB_VPNPROVIDER_INTERFACE_0_1 "PPB_VpnProvider;0.1" /* dev */
/**
 * @file
 * This file defines the <code>PPB_VpnProvider</code> interface.
 */


/**
 * @addtogroup Interfaces
 * @{
 */
/**
 * Use the <code>PPB_VpnProvider</code> interface to implement a VPN client.
 * Important: This API is available only on Chrome OS.
 *
 * This interface enhances the <code>chrome.vpnProvider</code> JavaScript API by
 * providing a high performance path for packet handling.
 *
 * Permissions: Apps permission <code>vpnProvider</code> is required for
 * <code>PPB_VpnProvider.Bind()</code>.
 *
 * Typical usage:
 * - Create a <code>PPB_VpnProvider</code> instance.
 * - Register the callback for <code>PPB_VpnProvider.ReceivePacket()</code>.
 * - In the extension follow the usual workflow for configuring a VPN connection
 *   via the <code>chrome.vpnProvider</code> API until the step for notifying
 *   the connection state as "connected".
 * - Bind to the previously created connection using
 *   <code>PPB_VpnProvider.Bind()</code>.
 * - Notify the connection state as "connected" from JavaScript using
 *   <code>chrome.vpnProvider.notifyConnectionStateChanged</code>.
 * - When the steps above are completed without errors, a virtual tunnel is
 *   created to the network stack of Chrome OS. IP packets can be sent through
 *   the tunnel using <code>PPB_VpnProvider.SendPacket()</code> and any packets
 *   originating on the Chrome OS device will be received using the callback
 *   registered for <code>PPB_VpnProvider.ReceivePacket()</code>.
 * - When the user disconnects from the VPN configuration or there is an error
 *   the extension will be notfied via
 *   <code>chrome.vpnProvider.onPlatformMessage</code>.
 */
struct PPB_VpnProvider_0_1 { /* dev */
  /**
   * Create() creates a VpnProvider instance.
   *
   * @param[in] instance A <code>PP_Instance</code> identifying the instance
   * with the VpnProvider.
   *
   * @return A <code>PP_Resource</code> corresponding to a VpnProvider if
   * successful.
   */
  PP_Resource (*Create)(PP_Instance instance);
  /**
   * IsVpnProvider() determines if the provided <code>resource</code> is a
   * VpnProvider instance.
   *
   * @param[in] resource A <code>PP_Resource</code> corresponding to a
   * VpnProvider.
   *
   * @return Returns <code>PP_TRUE</code> if <code>resource</code> is a
   * <code>PPB_VpnProvider</code>, <code>PP_FALSE</code> if the
   * <code>resource</code> is invalid or some type other than
   * <code>PPB_VpnProvider</code>.
   */
  PP_Bool (*IsVpnProvider)(PP_Resource resource);
  /**
   * Bind() binds to an existing configuration created from JavaScript by
   * <code>chrome.vpnProvider.createConfig</code>. All packets will be routed
   * via <code>SendPacket</code> and <code>ReceivePacket</code>. The user should
   * register the callback for <code>ReceivePacket</code> before calling
   * <code>Bind()</code>.
   *
   * @param[in] vpn_provider A <code>PP_Resource</code> corresponding to a
   * VpnProvider.
   *
   * @param[in] configuration_id A <code>PP_VARTYPE_STRING</code> representing
   * the configuration id from the callback of
   * <code>chrome.vpnProvider.createConfig</code>.
   *
   * @param[in] configuration_name A <code>PP_VARTYPE_STRING</code> representing
   * the configuration name as defined by the user when calling
   * <code>chrome.vpnProvider.createConfig</code>.
   *
   * @param[in] callback A <code>PP_CompletionCallback</code> called on
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   * Returns <code>PP_ERROR_INPROGRESS</code> if a previous call to
   * <code>Bind()</code> has not completed.
   * Returns <code>PP_ERROR_BADARGUMENT</code> if either
   * <code>configuration_id</code> or <code>configuration_name</code> are not of
   * type <code>PP_VARTYPE_STRING</code>.
   * Returns <code>PP_ERROR_NOACCESS</code> if the caller does the have the
   * required "vpnProvider" permission.
   * Returns <code>PP_ERROR_FAILED</code> if <code>connection_id</code> and
   * <code>connection_name</code> could not be matched with the existing
   * connection, or if the plugin originates from a different extension than the
   * one that created the connection.
   */
  int32_t (*Bind)(PP_Resource vpn_provider,
                  struct PP_Var configuration_id,
                  struct PP_Var configuration_name,
                  struct PP_CompletionCallback callback);
  /**
   * SendPacket() sends an IP packet through the tunnel created for the VPN
   * session. This will succeed only when the VPN session is owned by the
   * module and the connection is bound.
   *
   * @param[in] vpn_provider A <code>PP_Resource</code> corresponding to a
   * VpnProvider.
   *
   * @param[in] packet A <code>PP_VARTYPE_ARRAY_BUFFER</code> corresponding to
   * an IP packet to be sent to the platform.
   *
   * @param[in] callback A <code>PP_CompletionCallback</code> called on
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   * Returns <code>PP_ERROR_FAILED</code> if the connection is not bound.
   * Returns <code>PP_ERROR_INPROGRESS</code> if a previous call to
   * <code>SendPacket()</code> has not completed.
   * Returns <code>PP_ERROR_BADARGUMENT</code> if <code>packet</code> is not of
   * type <code>PP_VARTYPE_ARRAY_BUFFER</code>.
   */
  int32_t (*SendPacket)(PP_Resource vpn_provider,
                        struct PP_Var packet,
                        struct PP_CompletionCallback callback);
  /**
   * ReceivePacket() receives an IP packet from the tunnel for the VPN session.
   * This function only returns a single packet. This function must be called at
   * least N times to receive N packets, no matter the size of each packet. The
   * callback should be registered before calling <code>Bind()</code>.
   *
   * @param[in] vpn_provider A <code>PP_Resource</code> corresponding to a
   * VpnProvider.
   *
   * @param[out] packet The received packet is copied to provided
   * <code>packet</code>. The <code>packet</code> must remain valid until
   * ReceivePacket() completes. Its received <code>PP_VarType</code> will be
   * <code>PP_VARTYPE_ARRAY_BUFFER</code>.
   *
   * @param[in] callback A <code>PP_CompletionCallback</code> called on
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   * Returns <code>PP_ERROR_INPROGRESS</code> if a previous call to
   * <code>ReceivePacket()</code> has not completed.
   */
  int32_t (*ReceivePacket)(PP_Resource vpn_provider,
                           struct PP_Var* packet,
                           struct PP_CompletionCallback callback);
};
/**
 * @}
 */

#endif  /* PPAPI_C_PPB_VPN_PROVIDER_H_ */

