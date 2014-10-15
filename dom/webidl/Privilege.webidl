/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface Principal;

[Constructor,
 NamedConstructor=FreshPrivilege]
interface Privilege {

  /**
   * Returns whether the other privilege equal the other.
   */
  boolean equals(Privilege other);

  /**
   * Returns whether the other privilege is equal to or weaker than
   * this privilege. 
   */
  boolean subsumes(Privilege other);

  /**
   * Join privileges.
   */
  Privilege and(Privilege other);

  /**
   * Is this privilege empty?
   */
  [Pure] readonly attribute boolean isEmpty;

  /**
   * Get a copy of the underlying label.
   */
  [Throws] readonly attribute Label asLabel;

  /**
   * Make a new copy of this privilege.
   */
  [Throws] Privilege clone();

  stringifier;
};
