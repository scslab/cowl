#ifndef nsLabeledBlobService_h__
#define nsLabeledBlobService_h__

#pragma once

#include "nsXPCOM.h"
#include "nsTArray.h"
#include "nsILabeledBlobService.h"
#include "mozilla/dom/LabeledBlob.h"

class nsLabeledBlobService : public nsILabeledBlobService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSILABELEDBLOBSERVICE

  nsLabeledBlobService();

  nsresult Init();


protected: friend class mozilla::dom::LabeledBlob;
  nsTArray<nsRefPtr<mozilla::dom::LabeledBlob> > mLabeledBlobList;
  virtual ~nsLabeledBlobService();
};

#endif
