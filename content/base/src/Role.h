/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIDocument.h"
#include "nsTArray.h"
#include "nsContentUtils.h"

struct JSContext;

namespace mozilla {
namespace dom {

typedef nsTArray< nsCOMPtr<nsIPrincipal> > PrincipalArray;

class Role MOZ_FINAL : public nsISupports
                     , public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(Role)

protected:
  ~Role();

public:
  Role();
  Role(const nsAString& principal, ErrorResult& aRv);
  Role(nsIPrincipal* principal);


  Role* GetParentObject() const; //FIXME

  virtual JSObject* WrapObject(JSContext* aCx) MOZ_OVERRIDE;

  static already_AddRefed<Role> Constructor(const GlobalObject& global, 
                                            const nsAString& principal, 
                                            ErrorResult& aRv);
  static already_AddRefed<Role> Constructor(const GlobalObject& global, 
                                            const Sequence<nsString >& principals, 
                                            ErrorResult& aRv);

  bool Equals(mozilla::dom::Role& other);

  bool Subsumes(mozilla::dom::Role& other);

  already_AddRefed<Role> Or(const nsAString& principal, ErrorResult& aRv);
  already_AddRefed<Role> Or(nsIPrincipal* principal, ErrorResult& aRv);
  already_AddRefed<Role> Or(mozilla::dom::Role& other, ErrorResult& aRv);

  already_AddRefed<Role> Clone(ErrorResult &aRv) const;

  void Stringify(nsString& retval);

public: // C++ only:
  void _Or(const nsAString& principal, ErrorResult& aRv);
  void _Or(nsIPrincipal* principal);
  void _Or(mozilla::dom::Role& other);

  bool IsEmpty() const
  {
    return !mPrincipals.Length();
  }

private:

public: // XXX TODO make private, unsafe
  PrincipalArray* GetDirectPrincipals()
  {
    return &mPrincipals;
  }

//  friend class Label;

private:
  PrincipalArray mPrincipals;

};

// Comparator helpers

class nsIPrincipalComparator {

public:
  int Compare(const nsCOMPtr<nsIPrincipal> &p1,
              const nsCOMPtr<nsIPrincipal> &p2) const;

  bool Equals(const nsCOMPtr<nsIPrincipal> &p1,
              const nsCOMPtr<nsIPrincipal> &p2) const
  {
    return Compare(p1,p2) == 0;
  }

  bool LessThan(const nsCOMPtr<nsIPrincipal> &p1,
                const nsCOMPtr<nsIPrincipal> &p2) const
  {
    return Compare(p1,p2) < 0;
  }

};


} // namespace dom
} // namespace mozilla
