/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */


interface Principal;
interface URI;

[Constructor(DOMString principal),
 Constructor(sequence<DOMString> principals)]
interface Role {

  /**
   * Returns whether the other role is equivalent to this role.
   * Roles are considered equal if they have the same principals.
   */
  boolean equals(Role other);

  /**
   * Returns whether the other role is equal to or weaker than this
   * role. 
   *
   * This role subsumes the other role if the set of principals it
   * contains is a (non-empty) subset of other's principals. That is,
   * the other role must have all the principals of this role. An
   * empty role only subsumes and is subsumed by another empty
   * role.
   *
   * Thus a role always subsumes itself.
   */
  boolean subsumes(Role other);

  /**
   * Add principal to this role and return the role.
   */
  [Throws] Role _or(DOMString principal);
  [Throws] Role _or(Principal principal);
  [Throws] Role _or(Role other);

  /**
   * Make a new copy of this role.
   */
  [Throws]
  Role clone();

  stringifier;
};
