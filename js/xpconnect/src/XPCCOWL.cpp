/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/Assertions.h"
#include "xpcprivate.h"
#include "xpcpublic.h"
#include "jsfriendapi.h"
#include "mozilla/dom/COWL.h"
#include "mozilla/dom/Label.h"
#include "mozilla/dom/Role.h"
#include "nsIContentSecurityPolicy.h"
#include "nsDocument.h"

using namespace xpc;
using namespace JS;
using namespace mozilla;
using namespace mozilla::dom;

namespace xpc {
namespace cowl {

#define COWL_CONFIG(compartment) \
    static_cast<CompartmentPrivate*>(JS_GetCompartmentPrivate((compartment)))->cowlConfig

NS_EXPORT_(void)
EnableCompartmentConfinement(JSCompartment *compartment)
{
  MOZ_ASSERT(compartment);

  if (IsCompartmentConfined(compartment))
    return;

  nsRefPtr<Label> privacy = new Label();
  MOZ_ASSERT(privacy);

  nsRefPtr<Label> trust = new Label();
  MOZ_ASSERT(trust);

  COWL_CONFIG(compartment).SetPrivacyLabel(privacy);
  COWL_CONFIG(compartment).SetTrustLabel(trust);

  // set privileges to compartment principal
  // we're not "copying" the principal since the principal may be a
  // null principal (iframe sandbox) and thus not a codebase principal
  nsCOMPtr<nsIPrincipal> privPrin = GetCompartmentPrincipal(compartment);
  nsRefPtr<Role> privRole = new Role(privPrin);
  ErrorResult aRv;
  nsRefPtr<Label> privileges = new Label(*privRole, aRv);
  MOZ_ASSERT(privileges);
  COWL_CONFIG(compartment).SetPrivileges(privileges);
}

NS_EXPORT_(bool)
IsCompartmentConfined(JSCompartment *compartment)
{
  MOZ_ASSERT(compartment);
  return COWL_CONFIG(compartment).isEnabled();
}

#define DEFINE_SET_LABEL(name)                                      \
  NS_EXPORT_(void)                                                  \
  SetCompartment##name(JSCompartment *compartment,                  \
                      mozilla::dom::Label *aLabel)                  \
  {                                                                 \
    MOZ_ASSERT(compartment);                                        \
    MOZ_ASSERT(aLabel);                                             \
                                                                    \
    NS_ASSERTION(IsCompartmentConfined(compartment),                \
                 "Must call EnableCompartmentConfinement() first"); \
    if (!IsCompartmentConfined(compartment))                        \
      return;                                                       \
                                                                    \
    ErrorResult aRv;                                                \
    nsRefPtr<Label> label = (aLabel)->Clone(aRv);                   \
                                                                    \
    MOZ_ASSERT(!(aRv).Failed());                                    \
    COWL_CONFIG(compartment).Set##name(label);                      \
  }

#define DEFINE_GET_LABEL(name)                                      \
  NS_EXPORT_(already_AddRefed<mozilla::dom::Label>)                 \
  GetCompartment##name(JSCompartment *compartment)                  \
  {                                                                 \
    MOZ_ASSERT(compartment);                                        \
    MOZ_ASSERT(cowl::IsCompartmentConfined(compartment));           \
    return COWL_CONFIG(compartment).Get##name();                    \
  }

// This function sets the compartment privacy label. It clones the given label.
// IMPORTANT: This function should not be exported to untrusted code.
// Untrusted code can only set the privacy label to a label that
// subsumes the "current label".
DEFINE_SET_LABEL(PrivacyLabel)
DEFINE_GET_LABEL(PrivacyLabel)

// This function sets the compartment trust label. It clones the given label.
// IMPORTANT: This function should not be exported to untrusted code.
// Untrusted code can only set the trust label to a label subsumed by
// the "current label".
DEFINE_SET_LABEL(TrustLabel)
DEFINE_GET_LABEL(TrustLabel)

// This function sets the compartment privacy clearance. It clones the given
// label.
// IMPORTANT: This function should not be exported to untrusted code.
// Untrusted code can only set the privacy clearance to a label that subsumes
// the privacy label.
DEFINE_SET_LABEL(PrivacyClearance)
DEFINE_GET_LABEL(PrivacyClearance)

// This function sets the compartment trust clearance. It clones the given
// label.
// IMPORTANT: This function should not be exported to untrusted code.
// Untrusted code can only set the trust clearance to a label subsumed by the
// trust label.
DEFINE_SET_LABEL(TrustClearance)
DEFINE_GET_LABEL(TrustClearance)


#undef DEFINE_SET_LABEL
#undef DEFINE_GET_LABEL

// This function gets a copy of the compartment privileges.
NS_EXPORT_(already_AddRefed<mozilla::dom::Label>)
GetCompartmentPrivileges(JSCompartment*compartment)
{
  ErrorResult aRv;

  nsRefPtr<Label> privs;

  if (cowl::IsCompartmentConfined(compartment)) {
    privs = COWL_CONFIG(compartment).GetPrivileges();
    if (!privs)
      return nullptr;
    privs = privs->Clone(aRv);
  }

  if (!privs || aRv.Failed())
    return nullptr;

  return privs.forget();
}

// Check if information can flow from the compartment to an object labeled with
// |privacy| and |trust| into the compartment.
NS_EXPORT_(bool)
GuardWrite(JSCompartment *compartment,
           Label &privacy, Label &trust, Label *aPrivs)
{
  ErrorResult aRv;

  if (!IsCompartmentConfined(compartment)) {
    NS_WARNING("Not in confined compartment");
    return false;
  }

  nsRefPtr<Label> privs = aPrivs ? aPrivs : new Label();
  nsRefPtr<Label> compPrivacy, compTrust;
  compPrivacy = GetCompartmentPrivacyLabel(compartment);
  compTrust   = GetCompartmentTrustLabel(compartment);

  // If any of the labels are missing, don't allow the information flow
  if (!compPrivacy || !compTrust) {
    NS_WARNING("Missing labels");
    return false;
  }


#if COWL_DEBUG
  {
    nsAutoString compPrivacyStr, compTrustStr, privacyStr, trustStr, privsStr;
    compPrivacy->Stringify(compPrivacyStr);
    compTrust->Stringify(compTrustStr);
    privacy.Stringify(privacyStr);
    trust.Stringify(trustStr);
    privs->Stringify(privsStr);

    printf("GuardWrite <%s,%s> to <%s,%s> | %s\n",
           NS_ConvertUTF16toUTF8(compPrivacyStr).get(),
           NS_ConvertUTF16toUTF8(compTrustStr).get(),
           NS_ConvertUTF16toUTF8(privacyStr).get(),
           NS_ConvertUTF16toUTF8(trustStr).get(),
           NS_ConvertUTF16toUTF8(privsStr).get());
  }
#endif


  // if not <compPrivacy,compTrust> [=_privs <privacy,trust>
  if (!(privacy.Subsumes(*privs, *compPrivacy) && compTrust->Subsumes(*privs, trust))) {
    NS_WARNING("Label not above current label");
    return false;
  }

  // <privacy,trust> [=_privs  <clrPrivacy, clrTrust>

  nsRefPtr<mozilla::dom::Label> clrPrivacy, clrTrust;
  clrPrivacy = GetCompartmentPrivacyClearance(compartment);
  clrTrust   = GetCompartmentTrustClearance(compartment);


  //  without clearance
  if (!clrPrivacy && !clrTrust) {
    return true;
  }
  // <privacy,trust> [=_privs <clrPrivacy,clrTrust>
  if (clrPrivacy->Subsumes(*privs, privacy) && trust.Subsumes(*privs, *clrTrust)) {
    return true;
  }

  NS_WARNING("Trying to write to object labeled above clearance");
  return false;
}

// Check if compartment can write to dst
NS_EXPORT_(bool)
GuardWrite(JSCompartment *compartment, JSCompartment *dst)
{
#if COWL_DEBUG
    {
        printf("GuardWrite :");
        {
            char *origin;
            uint32_t status = 0;
            GetCompartmentPrincipal(compartment)->GetOrigin(&origin);
            GetCompartmentPrincipal(compartment)->GetAppId(&status);
            printf(" %s [%x] to", origin, status);
            nsMemory::Free(origin);
        }
        {
            char *origin;
            uint32_t status = 0;
            GetCompartmentPrincipal(dst)->GetOrigin(&origin);
            GetCompartmentPrincipal(dst)->GetAppId(&status);
            printf("%s [%x] \n", origin, status);
            nsMemory::Free(origin);
        }
    }
#endif


  if (!IsCompartmentConfined(dst)) {
    NS_WARNING("Destination compartmetn is not confined");
    return false;
  }
  nsRefPtr<Label> privacy = GetCompartmentPrivacyLabel(dst);
  nsRefPtr<Label> trust   = GetCompartmentTrustLabel(dst);
  nsRefPtr<Label> privs   = GetCompartmentPrivileges(compartment);

  if (!privacy || !trust || !privs) {
    NS_WARNING("Missing privacy or trust labels");
    return false;
  }

  return GuardWrite(compartment, *privacy, *trust, privs);
}

// Check if information can flow from an object labeled with |privacy|
// and |trust| into the compartment. For this to hold, the compartment
// must preserve privacy, i.e., the compartment privacy label must
// subsume the object privacy labe, and not be corrupted, i.e., the
// object trust label must be at least as trustworthy as the
// compartment trust label.
NS_EXPORT_(bool)
GuardRead(JSCompartment *compartment,
          Label &privacy, Label &trust, Label *aPrivs, JSContext *cx)
{
  ErrorResult aRv;

  nsRefPtr<Label> privs = aPrivs ? aPrivs : new Label();
  nsRefPtr<Label> compPrivacy, compTrust;

  if (IsCompartmentConfined(compartment)) {
    compPrivacy = GetCompartmentPrivacyLabel(compartment);
    compTrust   = GetCompartmentTrustLabel(compartment);
  } else {
    // compartment is not confined
    nsCOMPtr<nsIPrincipal> privPrin = GetCompartmentPrincipal(compartment);
    nsRefPtr<Role> privRole = new Role(privPrin);
    compPrivacy = new Label(*privRole, aRv);
    compTrust   = new Label();
    if (aRv.Failed()) return false;
  }

  // If any of the labels are missing, don't allow the information flow
  if (!compPrivacy || !compTrust) {
    NS_WARNING("Missing labels!");
    return false;
  }


#if COWL_DEBUG
  {
    nsAutoString compPrivacyStr, compTrustStr, privacyStr, trustStr, privsStr;
    compPrivacy->Stringify(compPrivacyStr);
    compTrust->Stringify(compTrustStr);
    privacy.Stringify(privacyStr);
    trust.Stringify(trustStr);
    privs->Stringify(privsStr);

    printf("GuardRead <%s,%s> to <%s,%s> | %s\n",
           NS_ConvertUTF16toUTF8(privacyStr).get(),
           NS_ConvertUTF16toUTF8(trustStr).get(),
           NS_ConvertUTF16toUTF8(compPrivacyStr).get(),
           NS_ConvertUTF16toUTF8(compTrustStr).get(),
           NS_ConvertUTF16toUTF8(privsStr).get());
  }
#endif

  // <privacy,trust> [=_privs <compPrivacy,compTrust>
  if (compPrivacy->Subsumes(*privs, privacy) &&
      trust.Subsumes(*privs, *compTrust)) {
    return true;
  }

  NS_WARNING("Does not subsume");
  return false;
}

// Check if information can flow from compartment |source| to
// compartment |compartment|.
NS_EXPORT_(bool)
GuardRead(JSCompartment *compartment, JSCompartment *source, bool isGET)
{
  //isGET = true:  compartment is reading from source
  //               use compartment privs
  //isGET = false: source is writing to compartment
  //               use source privs
#if COWL_DEBUG
    {
        printf("GuardRead %s :", isGET ? "GET" : "SET");
        {
            char *origin;
            uint32_t status = 0;
            GetCompartmentPrincipal(source)->GetOrigin(&origin);
            GetCompartmentPrincipal(source)->GetAppId(&status);
            printf("%s [%x]", origin, status);
            nsMemory::Free(origin);
        }
        {
            char *origin;
            uint32_t status = 0;
            GetCompartmentPrincipal(compartment)->GetOrigin(&origin);
            GetCompartmentPrincipal(compartment)->GetAppId(&status);
            printf(" to %s [%x]\n", origin, status);
            nsMemory::Free(origin);
        }
    }
#endif



  nsRefPtr<Label> privacy, trust;

  if (IsCompartmentConfined(source)) {
    privacy = GetCompartmentPrivacyLabel(source);
    trust   = GetCompartmentTrustLabel(source);
  } else {
    privacy = new Label();
    trust   = new Label();
  }

  nsRefPtr<Label> privs = isGET ? GetCompartmentPrivileges(compartment)
                                : GetCompartmentPrivileges(source);


  if (!privacy || !trust) {
    NS_WARNING("Missing privacy or trust labels");
    return false;
  }

  return GuardRead(compartment, *privacy, *trust, privs);
}


#undef COWL_CONFIG

}; // cowl
}; // xpc
