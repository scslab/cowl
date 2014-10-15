/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface COWL {

  // Enable confinement for compartment, if not enabled.
  static void enable();
  static boolean isEnabled();


  // Get the compartment principal, stringified
  static DOMString getPrincipal();

  // Current label
  [SetterThrows, GetterThrows] static attribute Label privacyLabel;
  [SetterThrows, GetterThrows] static attribute Label trustLabel;

  // Clearance
  [SetterThrows, GetterThrows] static attribute Label? privacyClearance;
  [SetterThrows, GetterThrows] static attribute Label? trustClearance;

  // Privileges
  [SetterThrows, GetterThrows] static attribute Privilege? privileges;
};
