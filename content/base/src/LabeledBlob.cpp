/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "LabeledBlob.h"
#include "mozilla/dom/LabeledBlobBinding.h"
#include "nsLabeledBlobService.h"
#include "nsContentUtils.h"
#include "mozilla/dom/COWL.h"
#include "StructuredCloneTags.h"
#include "xpcprivate.h"

// Workaround for lack of idl
template<>
struct nsILabeledBlobService::COMTypeInfo<nsLabeledBlobService, void> {
    static const nsIID kIID NS_HIDDEN;
};
const nsIID nsILabeledBlobService::COMTypeInfo<nsLabeledBlobService, void>::kIID = LABELEDBLOBSERVICE_CID;

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(LabeledBlob)
NS_IMPL_CYCLE_COLLECTING_ADDREF(LabeledBlob)
NS_IMPL_CYCLE_COLLECTING_RELEASE(LabeledBlob)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(LabeledBlob)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

LabeledBlob::LabeledBlob(const nsAString& blob, Label& privacy, Label& trust)
  : mPrivacy(&privacy)
  , mTrust(&trust)
  , mBlob(blob)
{
}

already_AddRefed<LabeledBlob> 
LabeledBlob::Constructor(const GlobalObject& global, 
                         JSContext* cx,
                         const nsAString& blob,
                         ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  COWL::Enable(global, cx);

  nsRefPtr<Label> privacy = COWL::GetPrivacyLabel(global, cx, aRv);
  if (aRv.Failed())
    return nullptr;
  if (!privacy) {
    COWL::JSErrorResult(cx, aRv, "Failed to get current trust label.");
    return nullptr;
  }

  nsRefPtr<Label> trust = COWL::GetTrustLabel(global, cx, aRv);
  if (aRv.Failed()) 
    return nullptr;
  if (!trust) {
    COWL::JSErrorResult(cx, aRv, "Failed to get current trust label.");
    return nullptr;
  }
  return Constructor(global, cx, blob, *privacy, *trust, aRv);
}

already_AddRefed<LabeledBlob> 
LabeledBlob::Constructor(const GlobalObject& global,
                         JSContext* cx,
                         const nsAString& blob,
                         Label& privacy, ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  COWL::Enable(global, cx);

  nsRefPtr<Label> trust = COWL::GetTrustLabel(global, cx, aRv);
  if (aRv.Failed()) 
    return nullptr;
  if (!trust) {
    COWL::JSErrorResult(cx, aRv, "Failed to get current trust label.");
    return nullptr;
  }
  return Constructor(global, cx, blob, privacy, *trust, aRv);
}

already_AddRefed<LabeledBlob> 
LabeledBlob::Constructor(const GlobalObject& global,
                         JSContext* cx,
                         const nsAString& blob,
                         const nsAString& privacy,
                         ErrorResult& aRv)
{
  nsRefPtr<Label> lprivacy = Label::Constructor(global, privacy, aRv);
  if (aRv.Failed()) return nullptr;
  return Constructor(global, cx, blob, *lprivacy, aRv);
}

already_AddRefed<LabeledBlob> 
LabeledBlob::Constructor(const GlobalObject& global,
                         JSContext* cx,
                         const nsAString& blob,
                         const nsAString& privacy,
                         const nsAString& trust, ErrorResult& aRv)
{
  nsRefPtr<Label> lprivacy = Label::Constructor(global, privacy, aRv);
  if (aRv.Failed()) return nullptr;
  nsRefPtr<Label> ltrust = Label::Constructor(global, trust, aRv);
  if (aRv.Failed()) return nullptr;
  return Constructor(global, cx, blob, *lprivacy, *ltrust, aRv);
}

already_AddRefed<LabeledBlob> 
LabeledBlob::Constructor(const GlobalObject& global,
                         JSContext* cx,
                         const nsAString& blob,
                         Label& privacy,
                         Label& trust, ErrorResult& aRv)
{
  aRv.MightThrowJSException();
  JSCompartment *compartment = js::GetContextCompartment(cx);
  MOZ_ASSERT(compartment);

  if (MOZ_UNLIKELY(!xpc::cowl::IsCompartmentConfined(compartment)))
    xpc::cowl::EnableCompartmentConfinement(compartment);

  nsRefPtr<Label> privs = xpc::cowl::GetCompartmentPrivileges(compartment);

  // current compartment label must flow to label of blob
  if (!xpc::cowl::GuardWrite(compartment, privacy, trust, privs)) {
    COWL::JSErrorResult(cx, aRv, 
        "Label of blob is not above current label or below current clearance.");
    return nullptr;
  }

  // Create blob

  nsRefPtr<Label> privacyCopy = privacy.Clone(aRv);
  if (aRv.Failed()) {
    return nullptr;
  }

  nsRefPtr<Label> trustCopy = trust.Clone(aRv);
  if (aRv.Failed()) {
    return nullptr;
  }

  nsRefPtr<LabeledBlob> labeledBlob = new LabeledBlob(blob, privacy, trust);

  if (aRv.Failed()) {
    return nullptr;
  }

  return labeledBlob.forget();
}

already_AddRefed<Label> 
LabeledBlob::Privacy() const
{
  nsRefPtr<Label> privacy = mPrivacy;
  return privacy.forget();
}

already_AddRefed<Label> 
LabeledBlob::Trust() const
{
  nsRefPtr<Label> trust = mTrust;
  return trust.forget();
}

void 
LabeledBlob::GetBlob(JSContext* cx, nsString& aRetVal, ErrorResult& aRv) const
{
  aRv.MightThrowJSException();
  JSCompartment *compartment = js::GetContextCompartment(cx);
  MOZ_ASSERT(compartment);

  if (MOZ_UNLIKELY(!xpc::cowl::IsCompartmentConfined(compartment)))
    xpc::cowl::EnableCompartmentConfinement(compartment);

  nsRefPtr<Label> privs = xpc::cowl::GetCompartmentPrivileges(compartment);

  // current compartment label must flow to label of target
  // raise it if need be
  // TODO: raise label if the following check does not hold
  if (!xpc::cowl::GuardRead(compartment, *mPrivacy,*mTrust, privs, cx)) {
    COWL::JSErrorResult(cx, aRv, "Cannot inspect blob.");
    return;
  }
  aRetVal = mBlob;

}

bool
LabeledBlob::WriteStructuredClone(JSContext* cx, 
                                  JSStructuredCloneWriter* writer) 
{
  nsresult rv;
  nsCOMPtr<nsILabeledBlobService> ilbs(do_GetService(LABELEDBLOBSERVICE_CID, &rv));
  if (NS_FAILED(rv)) {
    return false;
  }
  nsLabeledBlobService* lbs = static_cast<nsLabeledBlobService*>(ilbs.get());
  if (!lbs) {
    return false;
  }
  if (JS_WriteUint32Pair(writer, SCTAG_DOM_LABELEDBLOB, 
                                 lbs->mLabeledBlobList.Length())) {
    lbs->mLabeledBlobList.AppendElement(this);
    return true;
  }
  return false;
}

JSObject*
LabeledBlob::ReadStructuredClone(JSContext* cx,
                                 JSStructuredCloneReader* reader, uint32_t idx)
{
  nsresult rv;
  nsCOMPtr<nsILabeledBlobService> ilbs(do_GetService(LABELEDBLOBSERVICE_CID, &rv));
  if (NS_FAILED(rv)) {
    return nullptr;
  }
  nsLabeledBlobService* lbs = static_cast<nsLabeledBlobService*>(ilbs.get());
  if (!lbs) {
    return nullptr;
  }
  if(idx >= lbs->mLabeledBlobList.Length()) {
    return nullptr;
  }
  nsRefPtr<LabeledBlob> labeledBlob = lbs->mLabeledBlobList[idx];
  lbs->mLabeledBlobList.RemoveElementAt(idx);

  ErrorResult aRv;
  //nsRefPtr<File> blob       = labeledBlob->Blob();
  nsRefPtr<Label> privacy   = labeledBlob->Privacy();
  nsRefPtr<Label> trust     = labeledBlob->Trust();

  privacy = privacy->Clone(aRv);
  if (aRv.Failed()) return nullptr;
  trust = trust->Clone(aRv);
  if (aRv.Failed()) return nullptr;

  nsRefPtr<LabeledBlob> b  = 
    new LabeledBlob(labeledBlob->GetBlob(), *(privacy.get()), *(trust.get()));

  return b->WrapObject(cx);
}

} // namespace dom
} // namespace mozilla
