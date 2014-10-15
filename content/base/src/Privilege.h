/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/Label.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIDocument.h"

struct JSContext;

namespace mozilla {
namespace dom {


class Privilege MOZ_FINAL : public nsISupports
                          , public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(Privilege)

protected:
  ~Privilege();

public:
  Privilege();
  Privilege(mozilla::dom::Label &label);

  Privilege* GetParentObject() const;//FIXME

  virtual JSObject* WrapObject(JSContext* aCx) MOZ_OVERRIDE;

  static already_AddRefed<Privilege> Constructor(const GlobalObject& global, ErrorResult& aRv);
  static already_AddRefed<Privilege> FreshPrivilege(const GlobalObject& global, ErrorResult& aRv);

  bool Equals(mozilla::dom::Privilege& other);

  bool Subsumes(mozilla::dom::Privilege& other);

  already_AddRefed<Privilege> And(mozilla::dom::Privilege& other);

  bool IsEmpty() const;

  already_AddRefed<Label> GetAsLabel(ErrorResult& aRv);
  already_AddRefed<Privilege> Clone(ErrorResult& aRv);

  void Stringify(nsString& retval);


public: // C++ only
  void _And(mozilla::dom::Privilege& other);

private:
  Label* DirectLabel() { return &mLabel; }


private:
  Label mLabel;

};

} // namespace dom
} // namespace mozilla
