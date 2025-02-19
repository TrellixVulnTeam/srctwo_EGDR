// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/accelerometer/accelerometer_reader.h"

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_util.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/singleton.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/sys_info.h"
#include "base/task_runner.h"
#include "base/task_runner_util.h"
#include "base/threading/platform_thread.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/threading/thread_task_runner_handle.h"

namespace chromeos {

namespace {

// Paths to access necessary data from the accelerometer device.
const base::FilePath::CharType kAccelerometerDevicePath[] =
    FILE_PATH_LITERAL("/dev/cros-ec-accel");
const base::FilePath::CharType kAccelerometerIioBasePath[] =
    FILE_PATH_LITERAL("/sys/bus/iio/devices/");

// Trigger created by accelerometer-init.sh to query the sensors.
const char kTriggerPrefix[] = "trigger";
const char kTriggerName[] = "sysfstrig0\n";

// Sysfs entry to trigger readings.
const base::FilePath::CharType kTriggerNow[] = "trigger_now";

// This is the per source scale file in use on kernels older than 3.18. We
// should remove this when all devices having accelerometers are on kernel 3.18
// or later or have been patched to use new format: http://crbug.com/510831
const base::FilePath::CharType kLegacyScaleNameFormatString[] =
    "in_accel_%s_scale";

// File within kAccelerometerDevicePath/device* which denotes a single scale to
// be used across all axes.
const base::FilePath::CharType kAccelerometerScaleFileName[] = "scale";

// File within kAccelerometerDevicePath/device* which denotes the
// AccelerometerSource for the accelerometer.
const base::FilePath::CharType kAccelerometerLocationFileName[] = "location";

// The filename giving the path to read the scan index of each accelerometer
// axis.
const char kLegacyAccelerometerScanIndexPathFormatString[] =
    "scan_elements/in_accel_%s_%s_index";

// The filename giving the path to read the scan index of each accelerometer
// when they are separate device paths.
const char kAccelerometerScanIndexPathFormatString[] =
    "scan_elements/in_accel_%s_index";

// The names of the accelerometers. Matches up with the enum AccelerometerSource
// in chromeos/accelerometer/accelerometer_types.h.
const char kAccelerometerNames[ACCELEROMETER_SOURCE_COUNT][5] = {"lid", "base"};

// The axes on each accelerometer. The order was changed on kernel 3.18+.
const char kAccelerometerAxes[][2] = {"x", "y", "z"};
const char kLegacyAccelerometerAxes[][2] = {"y", "x", "z"};

// The length required to read uint values from configuration files.
const size_t kMaxAsciiUintLength = 21;

// The size of individual values.
const size_t kDataSize = 2;

// The mean acceleration due to gravity on Earth in m/s^2.
const float kMeanGravity = 9.80665f;

// The number of axes for which there are acceleration readings.
const int kNumberOfAxes = 3;

// The size of data in one reading of the accelerometers.
const int kSizeOfReading = kDataSize * kNumberOfAxes;

// Reads |path| to the unsigned int pointed to by |value|. Returns true on
// success or false on failure.
bool ReadFileToInt(const base::FilePath& path, int* value) {
  std::string s;
  DCHECK(value);
  if (!base::ReadFileToStringWithMaxSize(path, &s, kMaxAsciiUintLength)) {
    return false;
  }
  base::TrimWhitespaceASCII(s, base::TRIM_ALL, &s);
  if (!base::StringToInt(s, value)) {
    LOG(ERROR) << "Failed to parse int \"" << s << "\" from " << path.value();
    return false;
  }
  return true;
}

// Reads |path| to the double pointed to by |value|. Returns true on success or
// false on failure.
bool ReadFileToDouble(const base::FilePath& path, double* value) {
  std::string s;
  DCHECK(value);
  if (!base::ReadFileToString(path, &s)) {
    return false;
  }
  base::TrimWhitespaceASCII(s, base::TRIM_ALL, &s);
  if (!base::StringToDouble(s, value)) {
    LOG(ERROR) << "Failed to parse double \"" << s << "\" from "
               << path.value();
    return false;
  }
  return true;
}

}  // namespace

const int AccelerometerReader::kDelayBetweenReadsMs = 100;

// Work that runs on a base::TaskRunner. It determines the accelerometer
// configuartion, and reads the data. Upon a successful read it will notify
// all observers.
class AccelerometerFileReader
    : public base::RefCountedThreadSafe<AccelerometerFileReader> {
 public:
  AccelerometerFileReader();

  // Detects the accelerometer configuration, if an accelerometer is available
  // triggers reads.
  void Initialize(
      scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner);

  // Attempts to read the accelerometer data. Upon a success, converts the raw
  // reading to an AccelerometerUpdate and notifies observers. Triggers another
  // read at the current sampling rate.
  void Read();

  // Add/Remove observers.
  void AddObserver(AccelerometerReader::Observer* observer);
  void RemoveObserver(AccelerometerReader::Observer* observer);

 private:
  friend class base::RefCountedThreadSafe<AccelerometerFileReader>;

  // Represents necessary information in order to read an accelerometer device.
  struct ReadingData {
    // The full path to the accelerometer device to read.
    base::FilePath path;

    // The accelerometer sources which can be read from |path|.
    std::vector<AccelerometerSource> sources;
  };

  // Configuration structure for accelerometer device.
  struct ConfigurationData {
    ConfigurationData();
    ~ConfigurationData();

    // Number of accelerometers on device.
    size_t count;

    // sysfs entry to trigger readings.
    base::FilePath trigger_now;

    // Which accelerometers are present on device.
    bool has[ACCELEROMETER_SOURCE_COUNT];

    // Scale of accelerometers (i.e. raw value * scale = m/s^2).
    float scale[ACCELEROMETER_SOURCE_COUNT][3];

    // Index of each accelerometer axis in data stream.
    int index[ACCELEROMETER_SOURCE_COUNT][3];

    // The information for each accelerometer device to be read. In kernel 3.18
    // there is one per ACCELEROMETER_SOURCE_COUNT, on 3.14 there is only one.
    std::vector<ReadingData> reading_data;
  };

  ~AccelerometerFileReader() {}

  // When accelerometers are presented as separate iio_devices this will perform
  // the initialize for one of the devices, at the given |iio_path| and the
  // symbolic link |name|. |location| is defined by AccelerometerSoure.
  bool InitializeAccelerometer(const base::FilePath& iio_path,
                               const base::FilePath& name,
                               const std::string& location);

  // TODO(jonross): Separate the initialization into separate files. Add a gyp
  // rule to have them built for the appropriate kernels. (crbug.com/525658)
  // When accelerometers are presented as a single iio_device this will perform
  // the initialization for both of them.
  bool InitializeLegacyAccelerometers(const base::FilePath& iio_path,
                                      const base::FilePath& name);

  // Attempts to read the accelerometer data. Upon a success, converts the raw
  // reading to an AccelerometerUpdate and notifies observers.
  void ReadFileAndNotify();

  // True if Initialize completed successfully, and there is an accelerometer
  // file to read.
  bool initialization_successful_;

  // The accelerometer configuration.
  ConfigurationData configuration_;

  // The observers to notify of accelerometer updates.
  scoped_refptr<base::ObserverListThreadSafe<AccelerometerReader::Observer>>
      observers_;

  // The task runner to use for blocking tasks.
  scoped_refptr<base::SequencedTaskRunner> task_runner_;

  // The last seen accelerometer data.
  scoped_refptr<AccelerometerUpdate> update_;

  DISALLOW_COPY_AND_ASSIGN(AccelerometerFileReader);
};

AccelerometerFileReader::AccelerometerFileReader()
    : initialization_successful_(false),
      observers_(
          new base::ObserverListThreadSafe<AccelerometerReader::Observer>()) {
}

void AccelerometerFileReader::Initialize(
    scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner) {
  DCHECK(base::SequencedTaskRunnerHandle::IsSet());
  task_runner_ = sequenced_task_runner;

  // Check for accelerometer symlink which will be created by the udev rules
  // file on detecting the device.
  if (base::IsDirectoryEmpty(base::FilePath(kAccelerometerDevicePath))) {
    if (base::SysInfo::IsRunningOnChromeOS()) {
      LOG(WARNING) << "Accelerometer device directory is empty at "
                   << kAccelerometerDevicePath;
    }
    return;
  }

  // Find trigger to use:
  base::FileEnumerator trigger_dir(base::FilePath(kAccelerometerIioBasePath),
                                   false, base::FileEnumerator::DIRECTORIES);
  std::string prefix = kTriggerPrefix;
  for (base::FilePath name = trigger_dir.Next(); !name.empty();
       name = trigger_dir.Next()) {
    if (name.BaseName().value().substr(0, prefix.size()) != prefix)
      continue;
    std::string trigger_name;
    if (!base::ReadFileToString(name.Append("name"), &trigger_name)) {
      if (base::SysInfo::IsRunningOnChromeOS()) {
        LOG(WARNING) << "Unable to read the trigger name at " << name.value();
      }
      continue;
    }
    if (trigger_name == kTriggerName) {
      base::FilePath trigger_now = name.Append(kTriggerNow);
      if (!base::PathExists(trigger_now)) {
        if (base::SysInfo::IsRunningOnChromeOS()) {
          LOG(ERROR) << "Accelerometer trigger does not exist at "
                     << trigger_now.value();
        }
        return;
      } else {
        configuration_.trigger_now = trigger_now;
        break;
      }
    }
  }
  if (configuration_.trigger_now.empty()) {
    if (base::SysInfo::IsRunningOnChromeOS()) {
      LOG(ERROR) << "Accelerometer trigger not found";
    }
    return;
  }

  base::FileEnumerator symlink_dir(base::FilePath(kAccelerometerDevicePath),
                                   false, base::FileEnumerator::FILES);
  bool legacy_cross_accel = false;
  for (base::FilePath name = symlink_dir.Next(); !name.empty();
       name = symlink_dir.Next()) {
    base::FilePath iio_device;
    if (!base::ReadSymbolicLink(name, &iio_device)) {
      LOG(ERROR) << "Failed to read symbolic link " << kAccelerometerDevicePath
                 << "/" << name.MaybeAsASCII() << "\n";
      return;
    }

    base::FilePath iio_path(base::FilePath(kAccelerometerIioBasePath)
                                .Append(iio_device.BaseName()));
    std::string location;
    legacy_cross_accel = !base::ReadFileToString(
        base::FilePath(iio_path).Append(kAccelerometerLocationFileName),
        &location);
    if (legacy_cross_accel) {
      if (!InitializeLegacyAccelerometers(iio_path, name))
        return;
    } else {
      base::TrimWhitespaceASCII(location, base::TRIM_ALL, &location);
      if (!InitializeAccelerometer(iio_path, name, location))
        return;
    }
  }

  // Verify indices are within bounds.
  for (int i = 0; i < ACCELEROMETER_SOURCE_COUNT; ++i) {
    if (!configuration_.has[i])
      continue;
    for (int j = 0; j < 3; ++j) {
      if (configuration_.index[i][j] < 0 ||
          configuration_.index[i][j] >=
              3 * static_cast<int>(configuration_.count)) {
        const char* axis = legacy_cross_accel ? kLegacyAccelerometerAxes[j]
                                              : kAccelerometerAxes[j];
        LOG(ERROR) << "Field index for " << kAccelerometerNames[i] << " "
                   << axis << " axis out of bounds.";
        return;
      }
    }
  }

  initialization_successful_ = true;
  Read();
}

void AccelerometerFileReader::Read() {
  DCHECK(base::SequencedTaskRunnerHandle::IsSet());
  ReadFileAndNotify();
  task_runner_->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&AccelerometerFileReader::Read, this),
      base::TimeDelta::FromMilliseconds(
          AccelerometerReader::kDelayBetweenReadsMs));
}

void AccelerometerFileReader::AddObserver(
    AccelerometerReader::Observer* observer) {
  observers_->AddObserver(observer);
  if (initialization_successful_) {
    task_runner_->PostNonNestableTask(
        FROM_HERE,
        base::Bind(&AccelerometerFileReader::ReadFileAndNotify, this));
  }
}

void AccelerometerFileReader::RemoveObserver(
    AccelerometerReader::Observer* observer) {
  observers_->RemoveObserver(observer);
}

bool AccelerometerFileReader::InitializeAccelerometer(
    const base::FilePath& iio_path,
    const base::FilePath& name,
    const std::string& location) {
  size_t config_index = 0;
  for (; config_index < arraysize(kAccelerometerNames); ++config_index) {
    if (location == kAccelerometerNames[config_index])
      break;
  }

  if (config_index >= arraysize(kAccelerometerNames)) {
    LOG(ERROR) << "Unrecognized location: " << location << " for device "
               << name.MaybeAsASCII() << "\n";
    return false;
  }

  double scale;
  if (!ReadFileToDouble(iio_path.Append(kAccelerometerScaleFileName), &scale))
    return false;

  const int kNumberAxes = arraysize(kAccelerometerAxes);
  for (size_t i = 0; i < kNumberAxes; ++i) {
    std::string accelerometer_index_path = base::StringPrintf(
        kAccelerometerScanIndexPathFormatString, kAccelerometerAxes[i]);
    if (!ReadFileToInt(iio_path.Append(accelerometer_index_path.c_str()),
                       &(configuration_.index[config_index][i]))) {
      LOG(ERROR) << "Index file " << accelerometer_index_path
                 << " could not be parsed\n";
      return false;
    }
    configuration_.scale[config_index][i] = scale;
  }
  configuration_.has[config_index] = true;
  configuration_.count++;

  ReadingData reading_data;
  reading_data.path =
      base::FilePath(kAccelerometerDevicePath).Append(name.BaseName());
  reading_data.sources.push_back(
      static_cast<AccelerometerSource>(config_index));

  configuration_.reading_data.push_back(reading_data);

  return true;
}

bool AccelerometerFileReader::InitializeLegacyAccelerometers(
    const base::FilePath& iio_path,
    const base::FilePath& name) {
  ReadingData reading_data;
  reading_data.path =
      base::FilePath(kAccelerometerDevicePath).Append(name.BaseName());
  // Read configuration of each accelerometer axis from each accelerometer from
  // /sys/bus/iio/devices/iio:deviceX/.
  for (size_t i = 0; i < arraysize(kAccelerometerNames); ++i) {
    configuration_.has[i] = false;
    // Read scale of accelerometer.
    std::string accelerometer_scale_path = base::StringPrintf(
        kLegacyScaleNameFormatString, kAccelerometerNames[i]);
    // Read the scale for all axes.
    int scale_divisor = 0;
    if (!ReadFileToInt(iio_path.Append(accelerometer_scale_path.c_str()),
                       &scale_divisor)) {
      continue;
    }
    if (scale_divisor == 0) {
      LOG(ERROR) << "Accelerometer " << accelerometer_scale_path
                 << "has scale of 0 and will not be used.";
      continue;
    }

    configuration_.has[i] = true;
    for (size_t j = 0; j < arraysize(kLegacyAccelerometerAxes); ++j) {
      configuration_.scale[i][j] = kMeanGravity / scale_divisor;
      std::string accelerometer_index_path = base::StringPrintf(
          kLegacyAccelerometerScanIndexPathFormatString,
          kLegacyAccelerometerAxes[j], kAccelerometerNames[i]);
      if (!ReadFileToInt(iio_path.Append(accelerometer_index_path.c_str()),
                         &(configuration_.index[i][j]))) {
        configuration_.has[i] = false;
        LOG(ERROR) << "Index file " << accelerometer_index_path
                   << " could not be parsed\n";
        return false;
      }
    }
    if (configuration_.has[i]) {
      configuration_.count++;
      reading_data.sources.push_back(static_cast<AccelerometerSource>(i));
    }
  }

  // Adjust the directions of accelerometers to match the AccelerometerUpdate
  // type specified in chromeos/accelerometer/accelerometer_types.h.
  configuration_.scale[ACCELEROMETER_SOURCE_SCREEN][1] *= -1.0f;
  configuration_.scale[ACCELEROMETER_SOURCE_SCREEN][2] *= -1.0f;

  configuration_.reading_data.push_back(reading_data);
  return true;
}

void AccelerometerFileReader::ReadFileAndNotify() {
  DCHECK(initialization_successful_);

  // Initiate the trigger to read accelerometers simultaneously
  int bytes_written = base::WriteFile(configuration_.trigger_now, "1\n", 2);
  if (bytes_written < 2) {
    PLOG(ERROR) << "Accelerometer trigger failure: " << bytes_written;
    return;
  }

  // Read resulting sample from /dev/cros-ec-accel.
  update_ = new AccelerometerUpdate();
  for (auto reading_data : configuration_.reading_data) {
    int reading_size = reading_data.sources.size() * kSizeOfReading;
    DCHECK_GT(reading_size, 0);
    char reading[reading_size];
    int bytes_read = base::ReadFile(reading_data.path, reading, reading_size);
    if (bytes_read < reading_size) {
      LOG(ERROR) << "Accelerometer Read " << bytes_read << " byte(s), expected "
                 << reading_size << " bytes from accelerometer "
                 << reading_data.path.MaybeAsASCII();
      return;
    }
    for (AccelerometerSource source : reading_data.sources) {
      DCHECK(configuration_.has[source]);
      int16_t* values = reinterpret_cast<int16_t*>(reading);
      update_->Set(source, values[configuration_.index[source][0]] *
                               configuration_.scale[source][0],
                   values[configuration_.index[source][1]] *
                       configuration_.scale[source][1],
                   values[configuration_.index[source][2]] *
                       configuration_.scale[source][2]);
    }
  }

  observers_->Notify(FROM_HERE,
                     &AccelerometerReader::Observer::OnAccelerometerUpdated,
                     update_);
}

AccelerometerFileReader::ConfigurationData::ConfigurationData() : count(0) {
  for (int i = 0; i < ACCELEROMETER_SOURCE_COUNT; ++i) {
    has[i] = false;
    for (int j = 0; j < 3; ++j) {
      scale[i][j] = 0;
      index[i][j] = -1;
    }
  }
}

AccelerometerFileReader::ConfigurationData::~ConfigurationData() {
}

// static
AccelerometerReader* AccelerometerReader::GetInstance() {
  return base::Singleton<AccelerometerReader>::get();
}

void AccelerometerReader::Initialize(
    scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner) {
  DCHECK(sequenced_task_runner.get());
  // Asynchronously detect and initialize the accelerometer to avoid delaying
  // startup.
  sequenced_task_runner->PostNonNestableTask(
      FROM_HERE,
      base::Bind(&AccelerometerFileReader::Initialize,
                 accelerometer_file_reader_.get(), sequenced_task_runner));
}

void AccelerometerReader::AddObserver(Observer* observer) {
  accelerometer_file_reader_->AddObserver(observer);
}

void AccelerometerReader::RemoveObserver(Observer* observer) {
  accelerometer_file_reader_->RemoveObserver(observer);
}

AccelerometerReader::AccelerometerReader()
    : accelerometer_file_reader_(new AccelerometerFileReader()) {
}

AccelerometerReader::~AccelerometerReader() {
}

}  // namespace chromeos
