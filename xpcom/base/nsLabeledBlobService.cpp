#include "nsLabeledBlobService.h"

NS_IMPL_CLASSINFO(nsLabeledBlobService, nullptr, 0, LABELEDBLOBSERVICE_CID)
NS_IMPL_ISUPPORTS_CI(nsLabeledBlobService, nsILabeledBlobService)

nsLabeledBlobService::nsLabeledBlobService() { }

nsLabeledBlobService::~nsLabeledBlobService() { }

nsresult nsLabeledBlobService::Init() { return NS_OK; }
