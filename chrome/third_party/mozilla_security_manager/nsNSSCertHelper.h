/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ian McGreer <mcgreer@netscape.com>
 *   Javier Delgadillo <javi@netscape.com>
 *   John Gardiner Myers <jgmyers@speakeasy.net>
 *   Martin v. Loewis <martin@v.loewis.de>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef CHROME_THIRD_PARTY_MOZILLA_SECURITY_MANAGER_NSNSSCERTHELPER_H_
#define CHROME_THIRD_PARTY_MOZILLA_SECURITY_MANAGER_NSNSSCERTHELPER_H_

#include <cert.h>

#include <string>

#include "net/cert/cert_database.h"
#include "net/cert/cert_type.h"

namespace mozilla_security_manager {

std::string DumpOidString(SECItem* oid);
std::string GetOIDText(SECItem* oid);

std::string ProcessRDN(CERTRDN* rdn);
std::string ProcessName(CERTName* name);
std::string ProcessBasicConstraints(SECItem* extension_data);
std::string ProcessGeneralName(PRArenaPool* arena,
                               CERTGeneralName* current);
std::string ProcessGeneralNames(PRArenaPool* arena,
                                CERTGeneralName* name_list);
std::string ProcessAltName(SECItem* extension_data);
std::string ProcessSubjectKeyId(SECItem* extension_data);
std::string ProcessAuthKeyId(SECItem* extension_data);
std::string ProcessCrlDistPoints(SECItem* extension_data);
std::string ProcessAuthInfoAccess(SECItem* extension_data);
std::string ProcessIA5String(SECItem* extension_data);
std::string ProcessBMPString(SECItem* extension_data);
std::string ProcessNSCertTypeExtension(SECItem* extension_data);
std::string ProcessKeyUsageBitString(SECItem* bitstring, char sep);
std::string ProcessKeyUsageExtension(SECItem* extension_data);
std::string ProcessExtKeyUsage(SECItem* extension_data);
std::string ProcessExtensionData(CERTCertExtension* extension);
std::string ProcessSubjectPublicKeyInfo(CERTSubjectPublicKeyInfo* spki);

net::CertType GetCertType(CERTCertificate *cert);

}  // namespace mozilla_security_manager

#endif  // CHROME_THIRD_PARTY_MOZILLA_SECURITY_MANAGER_NSNSSCERTHELPER_H_
