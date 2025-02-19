// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_METRICS_PRIVATE_METRICS_PRIVATE_API_H_
#define EXTENSIONS_BROWSER_API_METRICS_PRIVATE_METRICS_PRIVATE_API_H_

#include <stddef.h>

#include <string>

#include "base/metrics/histogram.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class MetricsPrivateGetIsCrashReportingEnabledFunction
    : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.getIsCrashReportingEnabled",
                             METRICSPRIVATE_GETISCRASHRECORDINGENABLED)

 protected:
  ~MetricsPrivateGetIsCrashReportingEnabledFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateGetFieldTrialFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.getFieldTrial",
                             METRICSPRIVATE_GETFIELDTRIAL)

 protected:
  ~MetricsPrivateGetFieldTrialFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateGetVariationParamsFunction
    : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.getVariationParams",
                             METRICSPRIVATE_GETVARIATIONPARAMS)

 protected:
  ~MetricsPrivateGetVariationParamsFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordUserActionFunction
    : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordUserAction",
                             METRICSPRIVATE_RECORDUSERACTION)

 protected:
  ~MetricsPrivateRecordUserActionFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsHistogramHelperFunction : public UIThreadExtensionFunction {
 protected:
  ~MetricsHistogramHelperFunction() override {}
  void RecordValue(const std::string& name,
                   base::HistogramType type,
                   int min,
                   int max,
                   size_t buckets,
                   int sample);
};

class MetricsPrivateRecordValueFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordValue",
                             METRICSPRIVATE_RECORDVALUE)

 protected:
  ~MetricsPrivateRecordValueFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordSparseHashableFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordSparseHashable",
                             METRICSPRIVATE_RECORDSPARSEHASHABLE)

 protected:
  ~MetricsPrivateRecordSparseHashableFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordSparseValueFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordSparseValue",
                             METRICSPRIVATE_RECORDSPARSEVALUE)

 protected:
  ~MetricsPrivateRecordSparseValueFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordPercentageFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordPercentage",
                             METRICSPRIVATE_RECORDPERCENTAGE)

 protected:
  ~MetricsPrivateRecordPercentageFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordCountFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordCount",
                             METRICSPRIVATE_RECORDCOUNT)

 protected:
  ~MetricsPrivateRecordCountFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordSmallCountFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordSmallCount",
                             METRICSPRIVATE_RECORDSMALLCOUNT)

 protected:
  ~MetricsPrivateRecordSmallCountFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordMediumCountFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordMediumCount",
                             METRICSPRIVATE_RECORDMEDIUMCOUNT)

 protected:
  ~MetricsPrivateRecordMediumCountFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordTimeFunction : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordTime",
                             METRICSPRIVATE_RECORDTIME)

 protected:
  ~MetricsPrivateRecordTimeFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordMediumTimeFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordMediumTime",
                             METRICSPRIVATE_RECORDMEDIUMTIME)

 protected:
  ~MetricsPrivateRecordMediumTimeFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MetricsPrivateRecordLongTimeFunction
    : public MetricsHistogramHelperFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("metricsPrivate.recordLongTime",
                             METRICSPRIVATE_RECORDLONGTIME)

 protected:
  ~MetricsPrivateRecordLongTimeFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_METRICS_PRIVATE_METRICS_PRIVATE_API_H_
