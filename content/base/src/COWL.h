/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_COWL_h
#define mozilla_dom_COWL_h

#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/Label.h"
#include "mozilla/dom/Privilege.h"
#include "mozilla/dom/COWLBinding.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIDocument.h"

struct JSContext;

namespace mozilla {
namespace dom {

class COWL MOZ_FINAL : public nsISupports,
                       public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(COWL)

public:
  COWL();

protected:
  ~COWL();

public:
  COWL* GetParentObject() const; //FIXME

  virtual JSObject* WrapObject(JSContext* aCx) MOZ_OVERRIDE;

  static void Enable(const GlobalObject& global, JSContext* cx);

  static bool IsEnabled(const GlobalObject& global);

  static void GetPrincipal(const GlobalObject& global, 
                           JSContext* cx, nsString& aRetVal);

  static
  already_AddRefed<Label> GetPrivacyLabel(const GlobalObject& global,
                                          JSContext* cx,
                                          ErrorResult& aRv);

  static void SetPrivacyLabel(const GlobalObject& global,
                              JSContext* cx, Label& arg, ErrorResult& aRv);

  static
  already_AddRefed<Label> GetTrustLabel(const GlobalObject& global,
                                        JSContext* cx,
                                        ErrorResult& aRv);

  static void SetTrustLabel(const GlobalObject& global,
                            JSContext* cx, Label& arg, ErrorResult& aRv);

  static
  already_AddRefed<Label> GetPrivacyClearance(const GlobalObject& global,
                                              JSContext* cx,
                                              ErrorResult& aRv);

  static void SetPrivacyClearance(const GlobalObject& global,
                                  JSContext* cx, Label* arg, ErrorResult& aRv);

  static
  already_AddRefed<Label> GetTrustClearance(const GlobalObject& global,
                                            JSContext* cx,
                                            ErrorResult& aRv);

  static void SetTrustClearance(const GlobalObject& global,
                                JSContext* cx, Label* arg, ErrorResult& aRv);

  static
  already_AddRefed<Privilege> GetPrivileges(const GlobalObject& global,
                                            JSContext* cx, ErrorResult& aRv);

  static void SetPrivileges(const GlobalObject& global,
                            JSContext* cx, Privilege* arg, ErrorResult& aRv);
public:  // utils
  static void JSErrorResult(JSContext *cx, ErrorResult& aRv, const char *msg);
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_COWL_h
