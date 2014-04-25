/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

[Constructor(DOMString blob),
 Constructor(DOMString blob, Label privacy),
 Constructor(DOMString blob, DOMString privacy),
 Constructor(DOMString blob, Label privacy, Label trust),
 Constructor(DOMString blob, DOMString privacy, DOMString trust)
]
interface LabeledBlob {

  // Blob privacy and trust labels
  [Pure] readonly attribute Label privacy;
  [Pure] readonly attribute Label trust;

  // Underlying labeled value
  [GetterThrows] readonly attribute DOMString blob;

};
