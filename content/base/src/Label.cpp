/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/Label.h"
#include "mozilla/dom/LabelBinding.h"
#include "nsContentUtils.h"
#include "mozilla/dom/BindingUtils.h"

namespace mozilla {
namespace dom {


NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(Label)
//NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_1(Label, mRoles)
NS_IMPL_CYCLE_COLLECTING_ADDREF(Label)
NS_IMPL_CYCLE_COLLECTING_RELEASE(Label)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(Label)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

Label::Label()
{
}

Label::Label(mozilla::dom::Role &role, ErrorResult &aRv)
{
  _And(role,aRv);
}

Label::~Label()
{
}

JSObject*
Label::WrapObject(JSContext* aCx)
{
  return LabelBinding::Wrap(aCx, this);
}

Label*
Label::GetParentObject() const
{
  return nullptr; //TODO: return something sensible here
}

already_AddRefed<Label>
Label::Constructor(const GlobalObject& global, ErrorResult& aRv)
{
  nsRefPtr<Label> label = new Label();
  if (!label) {
    aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  return label.forget();
}

already_AddRefed<Label>
Label::Constructor(const GlobalObject& global, const nsAString& principal,
                   ErrorResult& aRv)
{
  nsRefPtr<Role> role = new Role(principal, aRv);
  if (aRv.Failed()) return nullptr;
  return Constructor(global,*role,aRv);
}

already_AddRefed<Label>
Label::Constructor(const GlobalObject& global, mozilla::dom::Role& role, 
                   ErrorResult& aRv)
{
  nsRefPtr<Label> label = new Label(role, aRv);
  if (aRv.Failed())
    return nullptr;
  return label.forget();
}

already_AddRefed<Label>
Label::Constructor(const GlobalObject& global, 
                   const Sequence<OwningNonNull<mozilla::dom::Role> >& roles, 
                   ErrorResult& aRv)
{
  nsRefPtr<Label> label = new Label();
  for (unsigned i = 0; i < roles.Length(); i++) {
    OwningNonNull<Role> role = roles[i];
    label->_And(role, aRv);
    if (aRv.Failed())
      return nullptr;
  }
  return label.forget();
}

bool
Label::Equals(mozilla::dom::Label& other)
{
  // Break out early if the other and this are the same.
  if (&other == this)
    return true;

  RoleArray *otherRoles = other.GetDirectRoles();

  // The other label is of a different size.
  if (otherRoles->Length() != mRoles.Length())
    return false;

  RoleComparator cmp;
  for (unsigned i = 0; i<mRoles.Length(); ++i) {
    /* This label contains a role that the other label does not, hence
     * they cannot be equal. */
    if (!otherRoles->Contains(mRoles[i],cmp))
      return false;
  }

  return true;
}

bool
Label::Subsumes(const mozilla::dom::Label& other)
{
  // Break out early if the other and this are the same.
  if (&other == this)
    return true;

  RoleArray *otherRoles =
    const_cast<mozilla::dom::Label&>(other).GetDirectRoles();

  /* There are more roles in the other formula, this label cannot
   * imply (subsume) it. */
  if (otherRoles->Length() >  mRoles.Length())
    return false;

  RoleSubsumeComparator cmp;
  for (unsigned i = 0; i<otherRoles->Length(); ++i) {
    /* The other label has a role (the ith role) that no role in this
     * label subsumes. */
    if (!mRoles.Contains(otherRoles->ElementAt(i),cmp))
      return false;
  }

  return true;
}

already_AddRefed<Label>
Label::And(const nsAString& principal, ErrorResult& aRv)
{
  nsRefPtr<Role> role = new Role(principal, aRv);
  if (aRv.Failed())
    return nullptr;

  _And(*role, aRv);
  if (aRv.Failed())
    return nullptr;

  nsRefPtr<Label> _this = this;
  return _this.forget();
}

already_AddRefed<Label>
Label::And(mozilla::dom::Role& role, ErrorResult& aRv)
{
  _And(role, aRv);
  if (aRv.Failed())
    return nullptr;

  nsRefPtr<Label> _this = this;
  return _this.forget();
}

already_AddRefed<Label>
Label::And(mozilla::dom::Label& other, ErrorResult& aRv)
{
  _And(other, aRv);
  if (aRv.Failed())
    return nullptr;

  nsRefPtr<Label> _this = this;
  return _this.forget();
}

already_AddRefed<Label>
Label::Or(const nsAString& principal, ErrorResult& aRv)
{
  nsRefPtr<Role> role = new Role(principal, aRv);
  if (aRv.Failed())
    return nullptr;

  _Or(*role, aRv);
  if (aRv.Failed())
    return nullptr;

  nsRefPtr<Label> _this = this;
  return _this.forget();
}


already_AddRefed<Label>
Label::Or(mozilla::dom::Role& role, ErrorResult& aRv)
{
  _Or(role, aRv);
  if (aRv.Failed())
    return nullptr;

  nsRefPtr<Label> _this = this;
  return _this.forget();
}

already_AddRefed<Label>
Label::Or(mozilla::dom::Label& other, ErrorResult& aRv)
{
  _Or(other, aRv);
  if (aRv.Failed())
    return nullptr;

  nsRefPtr<Label> _this = this;
  return _this.forget();
}

bool 
Label::IsEmpty() const
{
  return !mRoles.Length();
}

already_AddRefed<Label> 
Label::Clone(ErrorResult &aRv) const
{
  nsRefPtr<Label> label = new Label();

  if(!label) {
    aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }

  RoleArray *newRoles = label->GetDirectRoles();
  for (unsigned i = 0; i < mRoles.Length(); i++) {
    nsRefPtr<Role> role = mRoles[i]->Clone(aRv);
    if (aRv.Failed())
      return nullptr;
    newRoles->InsertElementAt(i, role);
  }

  return label.forget();
}


void
Label::Stringify(nsString& retval)
{
  retval = NS_LITERAL_STRING("Label(");

  for (unsigned i = 0; i < mRoles.Length(); i++) {
    nsAutoString role;
    mRoles[i]->Stringify(role);
    retval.Append(role);
    if (i != (mRoles.Length() -1))
      retval.Append(NS_LITERAL_STRING(").and("));
  }

  retval.Append(NS_LITERAL_STRING(")"));
}

bool
Label::Subsumes(nsIPrincipal* priv, const mozilla::dom::Label& other)
{
  if (!priv)
    return Subsumes(other);

  nsRefPtr<Role> privRole = new Role(priv);
  return Subsumes(*privRole,other);
}

bool
Label::Subsumes(const mozilla::dom::Role &role,
                const mozilla::dom::Label& other)
{
  // Break out early if the other and this are the same.
  // Or the other is an empty label.
  if (&other == this || other.IsEmpty())
    return true;

  if (role.IsEmpty())
    return Subsumes(other);

  ErrorResult aRv;
  nsRefPtr<Label> privs =
    new Label(const_cast<mozilla::dom::Role &>(role), aRv);

  if (aRv.Failed())
    return Subsumes(other);

  return Subsumes(*privs, other);
}

bool
Label::Subsumes(const mozilla::dom::Label &privs, 
                const mozilla::dom::Label& other)
{
  // Break out early if the other and this are the same.
  // Or the other is an empty label.
  if (&other == this || other.IsEmpty())
    return true;

  ErrorResult aRv;
  nsRefPtr<Label> _this = Clone(aRv);
  if (aRv.Failed())
    return false;
  _this->_And(const_cast<mozilla::dom::Label&>(privs), aRv);
  if (aRv.Failed())
    return false;
  return _this->Subsumes(other);
}

void
Label::_And(nsIPrincipal *p, ErrorResult& aRv)
{
  nsRefPtr<Role> role = new Role(p);
  _And(*role, aRv);
}

void
Label::_And(mozilla::dom::Role& role, ErrorResult& aRv)
{
  InternalAnd(role, &aRv, true);
}

void
Label::_And(mozilla::dom::Label& label, ErrorResult& aRv)
{
  RoleArray *otherRoles = label.GetDirectRoles();
  for (unsigned i = 0; i < otherRoles->Length(); ++i) {
    _And(*(otherRoles->ElementAt(i)), aRv);
    if (aRv.Failed()) return;
  }
}

void 
Label::_Or(mozilla::dom::Role& role, ErrorResult& aRv)
{

  // This label is empty, disjunction should not change it.
  if (IsEmpty())
    return;

  /* Create a new label to which we'll add new roles containing the
   * above role. This eliminates the need to first do the disjunction
   * and then reduce the label to conjunctive normal form. */

  Label tmpLabel;

  for (unsigned i = 0; i < mRoles.Length(); ++i) {
    nsRefPtr<Role> nRole = mRoles.ElementAt(i);
    nRole->_Or(role);

    tmpLabel.InternalAnd(*nRole);
  }
  // copy assignment
  mRoles = *(tmpLabel.GetDirectRoles());
}

void
Label::_Or(mozilla::dom::Label& label, ErrorResult& aRv)
{
  RoleArray *otherRoles = label.GetDirectRoles();
  for (unsigned i = 0; i < otherRoles->Length(); ++i) {
    _Or(*(otherRoles->ElementAt(i)), aRv);
    if (aRv.Failed()) return;
  }
}

void
Label::Reduce(mozilla::dom::Label &label)
{
  if (label.IsEmpty()) return;

  RoleArray *roles = const_cast<mozilla::dom::Label&>(label).GetDirectRoles();
  RoleSubsumeInvComparator cmp;
  for (unsigned i = 0; i < roles->Length(); ++i) {
    while (mRoles.RemoveElement(roles->ElementAt(i), cmp)) ;
  }
}

already_AddRefed<nsIPrincipal>
Label::GetPrincipalIfSingleton() const
{
  PrincipalArray* ps = GetPrincipalsIfSingleton();

  if (!ps || ps->Length() != 1)
    return nullptr;

  nsCOMPtr<nsIPrincipal> p = ps->ElementAt(0);
  return p.forget();
}

PrincipalArray*
Label::GetPrincipalsIfSingleton() const
{
  if (mRoles.Length() != 1)
    return nullptr;
  return mRoles.ElementAt(0)->GetDirectPrincipals();
}

//
// Internals
//

void 
Label::InternalAnd(mozilla::dom::Role& role, ErrorResult* aRv, bool clone)
{
  /* If there is no role in this label that subsumes |role|, append it
   * and remove any roles it subsumes.  An empty role is ignored.  */
  if (!mRoles.Contains(&role, RoleSubsumeComparator())) {
    RoleSubsumeInvComparator cmp;

    // Remove any elements that this role subsumes
    while(mRoles.RemoveElement(&role, cmp)) ;

    if (clone && aRv) {
      nsRefPtr<mozilla::dom::Role> roleCopy = role.Clone(*aRv);
      if(!roleCopy)
        return;

      mRoles.AppendElement(roleCopy);
    } else {
      mRoles.AppendElement(&role);
    }
  }
}


} // namespace dom
} // namespace mozilla
