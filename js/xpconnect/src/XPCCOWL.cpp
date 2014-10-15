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


#undef COWL_CONFIG

}; // cowl
}; // xpc
