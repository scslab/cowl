/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface Principal;

[Constructor,
 Constructor(DOMString principal),
 Constructor(Role role),
 Constructor(sequence<Role> roles)]
interface Label {

  /**
   * Returns whether the other label is equivalent to this label.
   * Labels are considered equal if they have the same roles.
   */
  boolean equals(Label other);

  /**
   * Returns whether the other label is equal to or weaker than this
   * label. 
   *
   * This label subsumes the other label if the set of roles
   * it contains is a superset of other's roles. That is this
   * label must have all the roles of the other label.
   *
   * Thus a label always subsumes itself.
   */
  boolean subsumes(Label other);

  /**
   * Add role to this label and return the label.
   */
  [Throws] Label _and(DOMString principal);
  [Throws] Label and(Role role);
  [Throws] Label _and(Label other);

  /**
   * Add the principals in the role to all the label roles.
   */
  [Throws] Label _or(DOMString principal);
  [Throws] Label _or(Role role);
  [Throws] Label _or(Label other);

  /**
   * Remove components that the other label subsumes.
   */
  void reduce(Label other);


  /**
   * Is this label empty?
   */
  [Pure] readonly attribute boolean isEmpty;

  /**
   * Make a new copy of this label.
   */
  [Throws] Label clone();

  stringifier;
};
