/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/COWL.h"
#include "mozilla/dom/COWLBinding.h"
#include "mozilla/dom/RoleBinding.h"
#include "mozilla/dom/LabelBinding.h"
#include "mozilla/dom/PrivilegeBinding.h"
#include "xpcprivate.h"

namespace mozilla {
namespace dom {

#define COWL_CONFIG(compartment) \
    static_cast<xpc::CompartmentPrivate*>(JS_GetCompartmentPrivate((compartment)))->cowlConfig


NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(COWL)
NS_IMPL_CYCLE_COLLECTING_ADDREF(COWL)
NS_IMPL_CYCLE_COLLECTING_RELEASE(COWL)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(COWL)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

COWL::COWL()
{
}

COWL::~COWL()
{
}

JSObject*
COWL::WrapObject(JSContext* aCx)
{
  return COWLBinding::Wrap(aCx, this);
}

COWL*
COWL::GetParentObject() const
{
  return nullptr; //TODO: return something sensible here
}

void
COWL::Enable(const GlobalObject& global, JSContext *cx)
{
  if (IsEnabled(global)) return;

  JSCompartment *compartment =
    js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);
  xpc::cowl::EnableCompartmentConfinement(compartment);
  // Start using COWL wrappers:
  js::RecomputeWrappers(cx, js::AllCompartments(), js::AllCompartments());
}

bool 
COWL::IsEnabled(const GlobalObject& global)
{
  JSCompartment *compartment =
    js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);
  return xpc::cowl::IsCompartmentConfined(compartment);
}

void
COWL::GetPrincipal(const GlobalObject& global, JSContext* cx, nsString& retval)
{
  retval = NS_LITERAL_STRING("");
  JSCompartment *compartment =
    js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);

  nsIPrincipal* prin = xpc::GetCompartmentPrincipal(compartment);
  if (!prin) return;

  char *origin = NULL;
  if (NS_FAILED(prin->GetOrigin(&origin)))
    return;
  AppendASCIItoUTF16(origin, retval);
  NS_Free(origin);
}

// Helper macro for retriveing the privacy/trust label/clearance
#define GET_LABEL(name)                                                  \
  do {                                                                   \
    JSCompartment *compartment = js::GetObjectCompartment(global.Get()); \
    MOZ_ASSERT(compartment);                                             \
    nsRefPtr<Label> l = xpc::cowl::GetCompartment##name(compartment);    \
                                                                         \
    if (!l) return nullptr;                                              \
    ErrorResult aRv;                                                     \
    nsRefPtr<Label> clone = l->Clone(aRv);                               \
    if (aRv.Failed()) return nullptr;                                    \
    return clone.forget();                                               \
  } while(0);

already_AddRefed<Label>
COWL::GetPrivacyLabel(const GlobalObject& global, JSContext* cx,
                      ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  if (!IsEnabled(global)) {
    JSErrorResult(cx, aRv, "COWL is not enabled.");
    return nullptr;
  }
  GET_LABEL(PrivacyLabel);
}

void
COWL::SetPrivacyLabel(const GlobalObject& global, JSContext* cx, 
                    Label& aLabel, ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  Enable(global, cx);

  JSCompartment *compartment = js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);


  nsRefPtr<Label> privs = xpc::cowl::GetCompartmentPrivileges(compartment);

  nsRefPtr<Label> currentLabel = GetPrivacyLabel(global, cx, aRv);
  if (aRv.Failed()) return;
  if (!currentLabel) {
    JSErrorResult(cx, aRv, "Failed to get current privacy label.");
    return;
  }

  if (!aLabel.Subsumes(*privs, *currentLabel)) {
    JSErrorResult(cx, aRv, "Label is not above the current label.");
    return;
  }

  nsRefPtr<Label> currentClearance = GetPrivacyClearance(global, cx, aRv);
  if (aRv.Failed()) return;
  if (currentClearance && !currentClearance->Subsumes(aLabel)) {
    JSErrorResult(cx, aRv, "Label is not below the current clearance.");
    return;
  }

  xpc::cowl::SetCompartmentPrivacyLabel(compartment, &aLabel);
  // This affects cross-compartment communication. Adjust wrappers:
  js::RecomputeWrappers(cx, js::AllCompartments(), js::AllCompartments());
  xpc::cowl::RefineCompartmentCSP(compartment);
}

already_AddRefed<Label>
COWL::GetTrustLabel(const GlobalObject& global, JSContext* cx,
                    ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  if (!IsEnabled(global)) {
    JSErrorResult(cx, aRv, "COWL is not enabled.");
    return nullptr;
  }
  GET_LABEL(TrustLabel);
}

void
COWL::SetTrustLabel(const GlobalObject& global, JSContext* cx, 
                    Label& aLabel, ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  Enable(global, cx);

  JSCompartment *compartment = js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);

  nsRefPtr<Label> privs = xpc::cowl::GetCompartmentPrivileges(compartment);

  nsRefPtr<Label> currentLabel = GetTrustLabel(global, cx, aRv);
  if (aRv.Failed()) return;
  if (!currentLabel) {
    JSErrorResult(cx, aRv, "Failed to get current trust label.");
    return;
  }

  if (!currentLabel->Subsumes(*privs, aLabel)) {
    JSErrorResult(cx, aRv, "Label is not below the current label.");
    return;
  }

  nsRefPtr<Label> currentClearance = GetTrustClearance(global, cx, aRv);
  if (aRv.Failed()) return;
  if (currentClearance && !aLabel.Subsumes(*currentClearance)) {
    JSErrorResult(cx, aRv, "Label is not above the current clearance.");
    return;
  }

  xpc::cowl::SetCompartmentTrustLabel(compartment, &aLabel);
  // Changing the trust/integrity label affects cross-compartment
  // communication. Adjust wrappers:
  js::RecomputeWrappers(cx, js::AllCompartments(), js::AllCompartments());
}

already_AddRefed<Label>
COWL::GetPrivacyClearance(const GlobalObject& global, JSContext* cx,
                          ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  if (!IsEnabled(global)) {
    JSErrorResult(cx, aRv, "COWL is not enabled.");
    return nullptr;
  }
  GET_LABEL(PrivacyClearance);
}

void
COWL::SetPrivacyClearance(const GlobalObject& global, JSContext* cx, 
                          Label* aLabel, ErrorResult& aRv)
{
  aRv.MightThrowJSException();

  if (!aLabel) {
    JSErrorResult(cx, aRv, "Expected label for current clearance.");
    return;
  }

  Enable(global, cx);
  
  JSCompartment *compartment = js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);

  nsRefPtr<Label> privs = xpc::cowl::GetCompartmentPrivileges(compartment);

  nsRefPtr<Label> currentClearance = GetPrivacyClearance(global, cx, aRv);
  if (aRv.Failed()) return;
  if (currentClearance && !currentClearance->Subsumes(*privs, *aLabel)) {
    JSErrorResult(cx, aRv, "Clearance is not below the current clearance.");
    return;
  }

  nsRefPtr<Label> currentLabel = GetPrivacyLabel(global, cx, aRv);
  if (aRv.Failed()) return;
  if (!currentLabel) {
    JSErrorResult(cx, aRv, "Failed to get current trust label.");
    return;
  }

  if (!aLabel->Subsumes(*currentLabel)) {
    JSErrorResult(cx, aRv, "Clearance is not above the current label.");
    return;
  }

  xpc::cowl::SetCompartmentPrivacyClearance(compartment, aLabel);
}

already_AddRefed<Label>
COWL::GetTrustClearance(const GlobalObject& global, JSContext* cx,
                        ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  if (!IsEnabled(global)) {
    JSErrorResult(cx, aRv, "COWL is not enabled.");
    return nullptr;
  }
  GET_LABEL(TrustClearance);
}

void
COWL::SetTrustClearance(const GlobalObject& global, JSContext* cx, 
                        Label* aLabel, ErrorResult& aRv)
{
  aRv.MightThrowJSException();

  if (!aLabel) {
    JSErrorResult(cx, aRv, "Expected label for current clearance.");
    return;
  }

  Enable(global, cx);

  JSCompartment *compartment = js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);

  nsRefPtr<Label> privs = xpc::cowl::GetCompartmentPrivileges(compartment);

  nsRefPtr<Label> currentClearance = GetTrustClearance(global, cx, aRv);
  if (aRv.Failed()) return;
  if (currentClearance && !aLabel->Subsumes(*privs, *currentClearance)) {
    JSErrorResult(cx, aRv, "Clearance is not above the current clearance.");
    return;
  }

  nsRefPtr<Label> currentLabel = GetTrustLabel(global, cx, aRv);
  if (aRv.Failed()) return;
  if (!currentLabel) {
    JSErrorResult(cx, aRv, "Failed to get current trust label.");
    return;
  }

  if (!currentLabel->Subsumes(*aLabel)) {
    JSErrorResult(cx, aRv, "Clearance is not below the current label.");
    return;
  }

  xpc::cowl::SetCompartmentTrustClearance(compartment, aLabel);
}

already_AddRefed<Privilege>
COWL::GetPrivileges(const GlobalObject& global, JSContext* cx, ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  if (!IsEnabled(global)) {
    JSErrorResult(cx, aRv, "COWL is not enabled.");
    return nullptr;
  }

  JSCompartment *compartment =
    js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);

  // copy compartment privileges
  nsRefPtr<Label> privL =
    xpc::cowl::GetCompartmentPrivileges(compartment);

  nsRefPtr<Privilege> privs;

  if (!privL) 
    return nullptr;

  privs = new Privilege(*privL);

  return privs.forget();
}

void 
COWL::SetPrivileges(const GlobalObject& global, JSContext* cx,
                    Privilege* priv, ErrorResult& aRv)
{
  Enable(global, cx);
  JSCompartment *compartment =
    js::GetObjectCompartment(global.Get());
  MOZ_ASSERT(compartment);
  if (priv) {
    nsRefPtr<Label> newPrivs = priv->GetAsLabel(aRv);
    if (aRv.Failed()) return;
    COWL_CONFIG(compartment).SetPrivileges(newPrivs);
  } else {
    // dropping privileges:
    COWL_CONFIG(compartment).SetPrivileges(nullptr);
  }
}

// Helper for setting the ErrorResult to a string.  This function
// should only be called after MightThrowJSException() is called.
void
COWL::JSErrorResult(JSContext *cx, ErrorResult& aRv, const char *msg)
{
  JSString *err = JS_NewStringCopyZ(cx,msg); 
  if (err) {
    JS::RootedValue errv(cx, STRING_TO_JSVAL(err));
    aRv.ThrowJSException(cx,errv);
  } else {
    aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
  }
}

#undef COWL_CONFIG
} // namespace dom
} // namespace mozilla
