/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/Role.h"
#include "mozilla/dom/RoleBinding.h"
#include "nsContentUtils.h"
#include "nsNetUtil.h"
#include "nsIURI.h"
#include "nsCOMPtr.h"
#include "nsScriptSecurityManager.h"
#include "nsServiceManagerUtils.h"

namespace mozilla {
namespace dom {


//NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_1(Role, mPrincipals)
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(Role)
NS_IMPL_CYCLE_COLLECTING_ADDREF(Role)
NS_IMPL_CYCLE_COLLECTING_RELEASE(Role)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(Role)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END



Role::Role()
{
}

Role::Role(const nsAString& principal, ErrorResult& aRv)
{
  _Or(principal,aRv);
}

Role::Role(nsIPrincipal* principal)
{
  _Or(principal);
}

Role::~Role()
{
}

JSObject*
Role::WrapObject(JSContext* aCx)
{
  return RoleBinding::Wrap(aCx, this);
}

Role*
Role::GetParentObject() const
{
  return nullptr; //TODO: return something sensible here
}

already_AddRefed<Role>
Role::Constructor(const GlobalObject& global, const nsAString& principal, 
                  ErrorResult& aRv)
{
  nsRefPtr<Role> role = new Role(principal, aRv);
  if (aRv.Failed())
    return nullptr;
  return role.forget();
}

already_AddRefed<Role>
Role::Constructor(const GlobalObject& global, const Sequence<nsString >& principals, 
                  ErrorResult& aRv)
{
  nsRefPtr<Role> role = new Role();
  for (unsigned i = 0; i < principals.Length(); ++i) {
    role->_Or(principals[i],aRv);
    if (aRv.Failed())
      return nullptr;
  }
  return role.forget();
}

bool
Role::Equals(mozilla::dom::Role& other)
{
  // Break out early if the other and this are the same.
  if (&other == this)
    return true;

  PrincipalArray *otherPrincipals = other.GetDirectPrincipals();

  // The other role is of a different size, can't be equal.
  if (otherPrincipals->Length() != mPrincipals.Length())
    return false;

  nsIPrincipalComparator cmp;
  for (unsigned i=0; i< mPrincipals.Length(); ++i) {
    /* This role contains a principal that the other role does not, 
     * hence it cannot be equal to it. */
    if(!cmp.Equals(mPrincipals[i], (*otherPrincipals)[i]))
      return false;
  }

  return true;
}

bool
Role::Subsumes(mozilla::dom::Role& other)
{
  // Break out early if the other points to this
  if (&other == this)
    return true;

  PrincipalArray *otherPrincipals = other.GetDirectPrincipals();

  // The other role is smaller, this role cannot imply (subsume) it.
  if (otherPrincipals->Length() < mPrincipals.Length())
    return false;

  nsIPrincipalComparator cmp;
  for (unsigned i=0; i< mPrincipals.Length(); ++i) {
    /* This role contains a principal that the other role does not, 
     * hence it cannot imply (subsume) it. */
    if (!otherPrincipals->Contains(mPrincipals[i],cmp))
      return false;
  }

  return true;
}


already_AddRefed<Role>
Role::Or(const nsAString& principal, ErrorResult& aRv)
{
  _Or(principal, aRv);
  if (aRv.Failed())
    return nullptr;

  nsRefPtr<Role> _this = this;
  return _this.forget();
}

already_AddRefed<Role>
Role::Or(nsIPrincipal* principal, ErrorResult& aRv)
{
  _Or(principal);
  nsRefPtr<Role> _this = this;
  return _this.forget();
}

already_AddRefed<Role>
Role::Or(Role& other, ErrorResult& aRv)
{
  _Or(other);
  nsRefPtr<Role> _this = this;
  return _this.forget();
}


void 
Role::Stringify(nsString& retval)
{
  retval = NS_LITERAL_STRING("Role(");

  for (unsigned i=0; i < mPrincipals.Length(); ++i) {
    char *origin = NULL;
    nsresult rv = mPrincipals[i]->GetOrigin(&origin);
    if (NS_FAILED(rv) || !origin) {
      retval.Append(NS_LITERAL_STRING("x-bogus:<unknown-principal>"));
    } else {
      AppendASCIItoUTF16(origin, retval);
      NS_Free(origin);
    }

     if (i != (mPrincipals.Length() -1))
       retval.Append(NS_LITERAL_STRING(").or("));

  }
  retval.Append(NS_LITERAL_STRING(")"));
}

already_AddRefed<Role>
Role::Clone(ErrorResult &aRv) const
{
  nsRefPtr<Role> role = new Role();

  if(!role) {
    aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }

  PrincipalArray *newPrincipals = role->GetDirectPrincipals();
  for (unsigned i = 0; i < mPrincipals.Length(); ++i) {
    newPrincipals->InsertElementAt(i, mPrincipals[i]);
  }
  return role.forget();
}


//
// Internals
//

void
Role::_Or(const nsAString& principal, ErrorResult& aRv)
{
  nsCOMPtr<nsIScriptSecurityManager> secMan =
    nsContentUtils::GetSecurityManager();

  if (!secMan) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }

  nsresult rv;

  // Create URI
  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri), principal);
  if (NS_FAILED(rv)) {
    rv = NS_NewURI(getter_AddRefs(uri), 
                   NS_LITERAL_STRING("moz-role:") + principal);
    if (NS_FAILED(rv)) {
      aRv.Throw(rv);
      return;
    }
  }

  // Create Principal
  nsCOMPtr<nsIPrincipal> nPrincipal;
  rv = secMan->GetNoAppCodebasePrincipal(uri, getter_AddRefs(nPrincipal));
  if (NS_FAILED(rv)) {
    aRv.Throw(rv);
    return;
  }

  _Or(nPrincipal);
}

void
Role::_Or(nsIPrincipal* principal)
{
  // Add principal if it's not there
  nsIPrincipalComparator cmp;
  if (!mPrincipals.Contains(principal, cmp))
    mPrincipals.InsertElementSorted(principal, cmp);
}


void
Role::_Or(Role& other)
{
  PrincipalArray *otherPrincipals = other.GetDirectPrincipals();
  for (unsigned i=0; i< otherPrincipals->Length(); ++i) {
    _Or(otherPrincipals->ElementAt(i));
  }
}

// Comparator helpers

int
nsIPrincipalComparator::Compare(const nsCOMPtr<nsIPrincipal> &p1,
                                const nsCOMPtr<nsIPrincipal> &p2) const
{
  bool res;
  char *origin1, *origin2;
  nsresult rv;

  rv = p1->GetOrigin(&origin1);
  NS_ASSERTION(NS_SUCCEEDED(rv), "nsIPrincipal::GetOrigin failed");
  if (NS_FAILED(rv)) return false;

  rv = p2->GetOrigin(&origin2);
  NS_ASSERTION(NS_SUCCEEDED(rv), "nsIPrincipal::GetOrigin failed");
  if (NS_FAILED(rv)) return false;

  res = strcmp(origin1, origin2);

  nsMemory::Free(origin1);
  nsMemory::Free(origin2);

  return res;
}

} // namespace dom
} // namespace mozilla
