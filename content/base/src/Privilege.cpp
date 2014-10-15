/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Privilege.h"
#include "mozilla/dom/PrivilegeBinding.h"
#include "nsContentUtils.h"
#include "nsCOMPtr.h"
#include "nsComponentManagerUtils.h"
#include "nsIPrincipal.h"

namespace mozilla {
namespace dom {


NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(Privilege)
NS_IMPL_CYCLE_COLLECTING_ADDREF(Privilege)
NS_IMPL_CYCLE_COLLECTING_RELEASE(Privilege)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(Privilege)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

Privilege::Privilege()
{
}

Privilege::Privilege(mozilla::dom::Label &label)
  : mLabel(label)
{
}

Privilege::~Privilege()
{
}

JSObject*
Privilege::WrapObject(JSContext* aCx)
{
  return PrivilegeBinding::Wrap(aCx, this);
}

Privilege*
Privilege::GetParentObject() const
{
  return nullptr; //TODO: return something sensible here
}

already_AddRefed<Privilege> 
Privilege::Constructor(const GlobalObject& global, 
                       ErrorResult& aRv)
{
  nsRefPtr<Privilege> priv = new Privilege();
  if (aRv.Failed()) return nullptr;

  return priv.forget();
}

already_AddRefed<Privilege> 
Privilege::FreshPrivilege(const GlobalObject& global, ErrorResult& aRv)
{
  nsresult rv;
  // create a fresh principal
  nsCOMPtr<nsIPrincipal> prin = 
    do_CreateInstance("@mozilla.org/nullprincipal;1", &rv);

  if (NS_FAILED(rv)) {
    aRv.Throw(rv);
    return nullptr;
  }
   
  nsRefPtr<Role> role = new Role(prin);
  if (aRv.Failed()) return nullptr;

  nsRefPtr<Label> label = new Label(*role, aRv);
  if (aRv.Failed()) return nullptr;

  nsRefPtr<Privilege> priv = new Privilege(*label);
  if (!priv) return nullptr;

  return priv.forget();
}

bool 
Privilege::Equals(mozilla::dom::Privilege& other)
{
  return mLabel.Equals(*(other.DirectLabel()));
}

bool 
Privilege::Subsumes(mozilla::dom::Privilege& other)
{
  return mLabel.Subsumes(*(other.DirectLabel()));
}

already_AddRefed<Privilege>
Privilege::And(mozilla::dom::Privilege& other)
{ 
  _And(other);
  nsRefPtr<Privilege> _this = this;
  return _this.forget();
}

void
Privilege::_And(mozilla::dom::Privilege& other)
{ 
  // And clones the label
  ErrorResult aRv;
  mLabel._And(*(other.DirectLabel()), aRv);
}

bool 
Privilege::IsEmpty() const
{
  return mLabel.IsEmpty();
}

already_AddRefed<Label>
Privilege::GetAsLabel(ErrorResult& aRv)
{
  return mLabel.Clone(aRv);
}

already_AddRefed<Privilege>
Privilege::Clone(ErrorResult& aRv)
{
  nsRefPtr<Privilege> priv = new Privilege(mLabel);
  return priv.forget();
}

void 
Privilege::Stringify(nsString& retval)
{
  retval = NS_LITERAL_STRING("Privilege(");
  nsAutoString str;
  mLabel.Stringify(str);
  retval.Append(str);
  retval.Append(NS_LITERAL_STRING(")"));
}

} // namespace dom
} // namespace mozilla
