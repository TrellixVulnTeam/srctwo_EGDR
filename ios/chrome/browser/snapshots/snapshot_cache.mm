// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/snapshots/snapshot_cache.h"

#import <UIKit/UIKit.h>

#include "base/base_paths.h"
#include "base/critical_closure.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/mac/bind_objc_block.h"
#include "base/mac/scoped_nsobject.h"
#include "base/path_service.h"
#include "base/sequence_checker.h"
#include "base/sequenced_task_runner.h"
#include "base/strings/sys_string_conversions.h"
#include "base/task_runner_util.h"
#include "base/task_scheduler/post_task.h"
#include "base/threading/thread_restrictions.h"
#include "ios/chrome/browser/experimental_flags.h"
#import "ios/chrome/browser/snapshots/lru_cache.h"
#import "ios/chrome/browser/snapshots/snapshot_cache_internal.h"
#include "ios/chrome/browser/ui/ui_util.h"
#import "ios/chrome/browser/ui/uikit_ui_util.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@interface SnapshotCache ()
// Remove all UIImages from |lruCache_|.
- (void)handleEnterBackground;
// Remove all but adjacent UIImages from |lruCache_|.
- (void)handleLowMemory;
// Restore adjacent UIImages to |lruCache_|.
- (void)handleBecomeActive;
// Clear most recent caller information.
- (void)clearGreySessionInfo;
// Load uncached snapshot image and convert image to grey.
- (void)loadGreyImageAsync:(NSString*)sessionID;
// Save grey image to |greyImageDictionary_| and call into most recent
// |mostRecentGreyBlock_| if |mostRecentGreySessionId_| matches |sessionID|.
- (void)saveGreyImage:(UIImage*)greyImage forKey:(NSString*)sessionID;
@end

namespace {
enum ImageType {
  IMAGE_TYPE_COLOR,
  IMAGE_TYPE_GREYSCALE,
};

enum ImageScale {
  IMAGE_SCALE_1X,
  IMAGE_SCALE_2X,
  IMAGE_SCALE_3X,
};

const ImageType kImageTypes[] = {
    IMAGE_TYPE_COLOR, IMAGE_TYPE_GREYSCALE,
};

const NSUInteger kGreyInitialCapacity = 8;
const CGFloat kJPEGImageQuality = 1.0;  // Highest quality. No compression.

// Maximum size in number of elements that the LRU cache can hold before
// starting to evict elements.
const NSUInteger kLRUCacheMaxCapacity = 6;

// Returns the path of the directory containing the snapshots.
bool GetSnapshotsCacheDirectory(base::FilePath* snapshots_cache_directory) {
  base::FilePath cache_directory;
  if (!base::PathService::Get(base::DIR_CACHE, &cache_directory))
    return false;

  *snapshots_cache_directory =
      cache_directory.Append(FILE_PATH_LITERAL("Chromium"))
          .Append(FILE_PATH_LITERAL("Snapshots"));
  return true;
}

// Returns the path of the image for |session_id|, in |cache_directory|,
// of type |image_type| and scale |image_scale|.
base::FilePath ImagePath(NSString* session_id,
                         ImageType image_type,
                         ImageScale image_scale,
                         const base::FilePath& cache_directory) {
  NSString* filename = session_id;
  switch (image_type) {
    case IMAGE_TYPE_COLOR:
      // no-op
      break;
    case IMAGE_TYPE_GREYSCALE:
      filename = [filename stringByAppendingString:@"Grey"];
      break;
  }
  switch (image_scale) {
    case IMAGE_SCALE_1X:
      // no-op
      break;
    case IMAGE_SCALE_2X:
      filename = [filename stringByAppendingString:@"@2x"];
      break;
    case IMAGE_SCALE_3X:
      filename = [filename stringByAppendingString:@"@3x"];
      break;
  }
  filename = [filename stringByAppendingPathExtension:@"jpg"];
  return cache_directory.Append(base::SysNSStringToUTF8(filename));
}

ImageScale ImageScaleForDevice() {
  // On handset, the color snapshot is used for the stack view, so the scale of
  // the snapshot images should match the scale of the device.
  // On tablet, the color snapshot is only used to generate the grey snapshot,
  // which does not have to be high quality, so use scale of 1.0 on all tablets.
  if (IsIPadIdiom())
    return IMAGE_SCALE_1X;

  // Cap snapshot resolution to 2x to reduce the amount of memory used.
  return [UIScreen mainScreen].scale == 1.0 ? IMAGE_SCALE_1X : IMAGE_SCALE_2X;
}

CGFloat ScaleFromImageScale(ImageScale image_scale) {
  switch (image_scale) {
    case IMAGE_SCALE_1X:
      return 1.0;
    case IMAGE_SCALE_2X:
      return 2.0;
    case IMAGE_SCALE_3X:
      return 3.0;
  }
}

UIImage* ReadImageForSessionFromDisk(NSString* session_id,
                                     ImageType image_type,
                                     ImageScale image_scale,
                                     const base::FilePath& cache_directory) {
  base::ThreadRestrictions::AssertIOAllowed();
  // TODO(crbug.com/295891): consider changing back to -imageWithContentsOfFile
  // instead of -imageWithData if both rdar://15747161 and the bug incorrectly
  // reporting the image as damaged https://stackoverflow.com/q/5081297/5353
  // are fixed.
  base::FilePath file_path =
      ImagePath(session_id, image_type, image_scale, cache_directory);
  NSString* path = base::SysUTF8ToNSString(file_path.AsUTF8Unsafe());
  return [UIImage imageWithData:[NSData dataWithContentsOfFile:path]
                          scale:(image_type == IMAGE_TYPE_GREYSCALE
                                     ? 1.0
                                     : ScaleFromImageScale(image_scale))];
}

void WriteImageToDisk(UIImage* image, const base::FilePath& file_path) {
  base::ThreadRestrictions::AssertIOAllowed();
  if (!image)
    return;

  base::FilePath directory = file_path.DirName();
  if (!base::DirectoryExists(directory)) {
    bool success = base::CreateDirectory(directory);
    if (!success) {
      DLOG(ERROR) << "Error creating thumbnail directory "
                  << directory.AsUTF8Unsafe();
      return;
    }
  }

  NSString* path = base::SysUTF8ToNSString(file_path.AsUTF8Unsafe());
  [UIImageJPEGRepresentation(image, kJPEGImageQuality) writeToFile:path
                                                        atomically:YES];

  // Encrypt the snapshot file (mostly for Incognito, but can't hurt to
  // always do it).
  NSDictionary* attribute_dict =
      [NSDictionary dictionaryWithObject:NSFileProtectionComplete
                                  forKey:NSFileProtectionKey];
  NSError* error = nil;
  BOOL success = [[NSFileManager defaultManager] setAttributes:attribute_dict
                                                  ofItemAtPath:path
                                                         error:&error];
  if (!success) {
    DLOG(ERROR) << "Error encrypting thumbnail file "
                << base::SysNSStringToUTF8([error description]);
  }
}

void ConvertAndSaveGreyImage(NSString* session_id,
                             ImageScale image_scale,
                             UIImage* color_image,
                             const base::FilePath& cache_directory) {
  base::ThreadRestrictions::AssertIOAllowed();
  if (!color_image) {
    color_image = ReadImageForSessionFromDisk(session_id, IMAGE_TYPE_COLOR,
                                              image_scale, cache_directory);
    if (!color_image)
      return;
  }
  UIImage* grey_image = GreyImage(color_image);
  WriteImageToDisk(grey_image, ImagePath(session_id, IMAGE_TYPE_GREYSCALE,
                                         image_scale, cache_directory));
}

}  // anonymous namespace

@implementation SnapshotCache {
  // Cache to hold color snapshots in memory. n.b. Color snapshots are not
  // kept in memory on tablets.
  LRUCache* lruCache_;

  // Temporary dictionary to hold grey snapshots for tablet side swipe. This
  // will be nil before -createGreyCache is called and after -removeGreyCache
  // is called.
  NSMutableDictionary<NSString*, UIImage*>* greyImageDictionary_;

  // Session ID of most recent pending grey snapshot request.
  NSString* mostRecentGreySessionId_;
  // Block used by pending request for a grey snapshot.
  void (^mostRecentGreyBlock_)(UIImage*);

  // Session ID and corresponding UIImage for the snapshot that will likely
  // be requested to be saved to disk when the application is backgrounded.
  NSString* backgroundingImageSessionId_;
  UIImage* backgroundingColorImage_;

  // Scale for snapshot images. May be smaller than the screen scale in order
  // to save memory on some devices.
  ImageScale snapshotsScale_;

  // Directory where the thumbnails are saved.
  base::FilePath cacheDirectory_;

  // Task runner used to run tasks in the background. Will be invalidated when
  // -shutdown is invoked. Code should support this value to be null (generally
  // by not posting the task).
  scoped_refptr<base::SequencedTaskRunner> taskRunner_;

  // Check that public API is called from the correct sequence.
  SEQUENCE_CHECKER(sequenceChecker_);
}

@synthesize pinnedIDs = pinnedIDs_;

- (instancetype)init {
  base::FilePath cacheDirectory;
  if (!GetSnapshotsCacheDirectory(&cacheDirectory))
    return nil;

  return [self initWithCacheDirectory:cacheDirectory
                       snapshotsScale:ImageScaleForDevice()];
}

- (instancetype)initWithCacheDirectory:(const base::FilePath&)cacheDirectory
                        snapshotsScale:(ImageScale)snapshotsScale {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  if ((self = [super init])) {
    lruCache_ = [[LRUCache alloc] initWithCacheSize:kLRUCacheMaxCapacity];
    cacheDirectory_ = cacheDirectory;
    snapshotsScale_ = snapshotsScale;

    taskRunner_ = base::CreateSequencedTaskRunnerWithTraits(
        {base::MayBlock(), base::TaskPriority::USER_VISIBLE});

    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(handleLowMemory)
               name:UIApplicationDidReceiveMemoryWarningNotification
             object:nil];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(handleEnterBackground)
               name:UIApplicationDidEnterBackgroundNotification
             object:nil];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(handleBecomeActive)
               name:UIApplicationDidBecomeActiveNotification
             object:nil];
  }
  return self;
}

- (void)dealloc {
  DCHECK(!taskRunner_) << "-shutdown must be called before -dealloc";

  [[NSNotificationCenter defaultCenter]
      removeObserver:self
                name:UIApplicationDidReceiveMemoryWarningNotification
              object:nil];
  [[NSNotificationCenter defaultCenter]
      removeObserver:self
                name:UIApplicationDidEnterBackgroundNotification
              object:nil];
  [[NSNotificationCenter defaultCenter]
      removeObserver:self
                name:UIApplicationDidBecomeActiveNotification
              object:nil];
}

- (CGFloat)snapshotScaleForDevice {
  return ScaleFromImageScale(snapshotsScale_);
}

- (void)retrieveImageForSessionID:(NSString*)sessionID
                         callback:(void (^)(UIImage*))callback {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  DCHECK(sessionID);

  UIImage* image = [lruCache_ objectForKey:sessionID];
  if (image) {
    if (callback)
      callback(image);
    return;
  }

  if (!taskRunner_) {
    callback(nil);
    return;
  }

  // Copy ivars used by the block so that it does not reference |self|.
  const base::FilePath cacheDirectory = cacheDirectory_;
  const ImageScale snapshotsScale = snapshotsScale_;

  __weak SnapshotCache* weakSelf = self;
  base::PostTaskAndReplyWithResult(
      taskRunner_.get(), FROM_HERE,
      base::BindBlockArc(^base::scoped_nsobject<UIImage>() {
        // Retrieve the image on a high priority thread.
        return base::scoped_nsobject<UIImage>(ReadImageForSessionFromDisk(
            sessionID, IMAGE_TYPE_COLOR, snapshotsScale, cacheDirectory));
      }),
      base::BindBlockArc(^(base::scoped_nsobject<UIImage> image) {
        __strong SnapshotCache* strongSelf = weakSelf;
        if (image && strongSelf)
          [strongSelf->lruCache_ setObject:image forKey:sessionID];
        if (callback)
          callback(image);
      }));
}

- (void)setImage:(UIImage*)image withSessionID:(NSString*)sessionID {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  if (!image || !sessionID || !taskRunner_)
    return;

  [lruCache_ setObject:image forKey:sessionID];

  // Copy ivars used by the block so that it does not reference |self|.
  const base::FilePath cacheDirectory = cacheDirectory_;
  const ImageScale snapshotsScale = snapshotsScale_;

  // Save the image to disk.
  taskRunner_->PostTask(
      FROM_HERE, base::BindBlockArc(^{
        WriteImageToDisk(image, ImagePath(sessionID, IMAGE_TYPE_COLOR,
                                          snapshotsScale, cacheDirectory));
      }));
}

- (void)removeImageWithSessionID:(NSString*)sessionID {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  [lruCache_ removeObjectForKey:sessionID];

  if (!taskRunner_)
    return;

  // Copy ivars used by the block so that it does not reference |self|.
  const base::FilePath cacheDirectory = cacheDirectory_;
  const ImageScale snapshotsScale = snapshotsScale_;

  taskRunner_->PostTask(
      FROM_HERE, base::BindBlockArc(^{
        for (size_t index = 0; index < arraysize(kImageTypes); ++index) {
          base::DeleteFile(ImagePath(sessionID, kImageTypes[index],
                                     snapshotsScale, cacheDirectory),
                           false /* recursive */);
        }
      }));
}

- (base::FilePath)imagePathForSessionID:(NSString*)sessionID {
  return ImagePath(sessionID, IMAGE_TYPE_COLOR, snapshotsScale_,
                   cacheDirectory_);
}

- (base::FilePath)greyImagePathForSessionID:(NSString*)sessionID {
  return ImagePath(sessionID, IMAGE_TYPE_GREYSCALE, snapshotsScale_,
                   cacheDirectory_);
}

- (void)purgeCacheOlderThan:(const base::Time&)date
                    keeping:(NSSet*)liveSessionIds {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);

  if (!taskRunner_)
    return;

  // Copying the date, as the block must copy the value, not the reference.
  const base::Time dateCopy = date;

  // Copy ivars used by the block so that it does not reference |self|.
  const base::FilePath cacheDirectory = cacheDirectory_;
  const ImageScale snapshotsScale = snapshotsScale_;

  taskRunner_->PostTask(
      FROM_HERE, base::BindBlockArc(^{
        if (!base::DirectoryExists(cacheDirectory))
          return;

        std::set<base::FilePath> filesToKeep;
        for (NSString* sessionID : liveSessionIds) {
          for (size_t index = 0; index < arraysize(kImageTypes); ++index) {
            filesToKeep.insert(ImagePath(sessionID, kImageTypes[index],
                                         snapshotsScale, cacheDirectory));
          }
        }
        base::FileEnumerator enumerator(cacheDirectory, false,
                                        base::FileEnumerator::FILES);
        base::FilePath current_file = enumerator.Next();
        for (; !current_file.empty(); current_file = enumerator.Next()) {
          if (current_file.Extension() != ".jpg")
            continue;
          if (filesToKeep.find(current_file) != filesToKeep.end())
            continue;
          base::FileEnumerator::FileInfo fileInfo = enumerator.GetInfo();
          if (fileInfo.GetLastModifiedTime() > dateCopy)
            continue;
          base::DeleteFile(current_file, false);
        }
      }));
}

- (void)willBeSavedGreyWhenBackgrounding:(NSString*)sessionID {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  if (!sessionID)
    return;
  backgroundingImageSessionId_ = [sessionID copy];
  backgroundingColorImage_ = [lruCache_ objectForKey:sessionID];
}

- (void)handleLowMemory {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  NSMutableDictionary<NSString*, UIImage*>* dictionary =
      [NSMutableDictionary dictionaryWithCapacity:2];
  for (NSString* sessionID in pinnedIDs_) {
    UIImage* image = [lruCache_ objectForKey:sessionID];
    if (image)
      [dictionary setObject:image forKey:sessionID];
  }
  [lruCache_ removeAllObjects];
  for (NSString* sessionID in pinnedIDs_)
    [lruCache_ setObject:[dictionary objectForKey:sessionID] forKey:sessionID];
}

- (void)handleEnterBackground {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  [lruCache_ removeAllObjects];
}

- (void)handleBecomeActive {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  for (NSString* sessionID in pinnedIDs_)
    [self retrieveImageForSessionID:sessionID callback:nil];
}

- (void)saveGreyImage:(UIImage*)greyImage forKey:(NSString*)sessionID {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  if (greyImage)
    [greyImageDictionary_ setObject:greyImage forKey:sessionID];
  if ([sessionID isEqualToString:mostRecentGreySessionId_]) {
    mostRecentGreyBlock_(greyImage);
    [self clearGreySessionInfo];
  }
}

- (void)loadGreyImageAsync:(NSString*)sessionID {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  // Don't call -retrieveImageForSessionID here because it caches the colored
  // image, which we don't need for the grey image cache. But if the image is
  // already in the cache, use it.
  UIImage* image = [lruCache_ objectForKey:sessionID];

  if (!taskRunner_)
    return;

  // Copy ivars used by the block so that it does not reference |self|.
  const base::FilePath cacheDirectory = cacheDirectory_;
  const ImageScale snapshotsScale = snapshotsScale_;

  __weak SnapshotCache* weakSelf = self;
  base::PostTaskAndReplyWithResult(
      taskRunner_.get(), FROM_HERE,
      base::BindBlockArc(^base::scoped_nsobject<UIImage>() {
        base::scoped_nsobject<UIImage> result(image);
        // If the image is not in the cache, load it from disk.
        if (!result) {
          result.reset(ReadImageForSessionFromDisk(
              sessionID, IMAGE_TYPE_COLOR, snapshotsScale, cacheDirectory));
        }
        if (result)
          result.reset(GreyImage(result));
        return result;
      }),
      base::BindBlockArc(^(base::scoped_nsobject<UIImage> greyImage) {
        [weakSelf saveGreyImage:greyImage forKey:sessionID];
      }));
}

- (void)createGreyCache:(NSArray*)sessionIDs {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  greyImageDictionary_ =
      [NSMutableDictionary dictionaryWithCapacity:kGreyInitialCapacity];
  for (NSString* sessionID in sessionIDs)
    [self loadGreyImageAsync:sessionID];
}

- (void)removeGreyCache {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  greyImageDictionary_ = nil;
  [self clearGreySessionInfo];
}

- (void)clearGreySessionInfo {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  mostRecentGreySessionId_ = nil;
  mostRecentGreyBlock_ = nil;
}

- (void)greyImageForSessionID:(NSString*)sessionID
                     callback:(void (^)(UIImage*))callback {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  DCHECK(greyImageDictionary_);
  UIImage* image = [greyImageDictionary_ objectForKey:sessionID];
  if (image) {
    callback(image);
    [self clearGreySessionInfo];
  } else {
    mostRecentGreySessionId_ = [sessionID copy];
    mostRecentGreyBlock_ = [callback copy];
  }
}

- (void)retrieveGreyImageForSessionID:(NSString*)sessionID
                             callback:(void (^)(UIImage*))callback {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  if (greyImageDictionary_) {
    UIImage* image = [greyImageDictionary_ objectForKey:sessionID];
    if (image) {
      callback(image);
      return;
    }
  }

  if (!taskRunner_) {
    callback(nil);
    return;
  }

  // Copy ivars used by the block so that it does not reference |self|.
  const base::FilePath cacheDirectory = cacheDirectory_;
  const ImageScale snapshotsScale = snapshotsScale_;

  __weak SnapshotCache* weakSelf = self;
  base::PostTaskAndReplyWithResult(
      taskRunner_.get(), FROM_HERE,
      base::BindBlockArc(^base::scoped_nsobject<UIImage>() {
        // Retrieve the image on a high priority thread.
        return base::scoped_nsobject<UIImage>(ReadImageForSessionFromDisk(
            sessionID, IMAGE_TYPE_GREYSCALE, snapshotsScale, cacheDirectory));
      }),
      base::BindBlockArc(^(base::scoped_nsobject<UIImage> image) {
        if (image) {
          if (callback)
            callback(image);
          return;
        }
        [weakSelf retrieveImageForSessionID:sessionID
                                   callback:^(UIImage* local_image) {
                                     if (callback && local_image)
                                       callback(GreyImage(local_image));
                                   }];
      }));
}

- (void)saveGreyInBackgroundForSessionID:(NSString*)sessionID {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequenceChecker_);
  if (!sessionID)
    return;

  // The color image may still be in memory.  Verify the sessionID matches.
  if (backgroundingColorImage_) {
    if (![backgroundingImageSessionId_ isEqualToString:sessionID]) {
      backgroundingImageSessionId_ = nil;
      backgroundingColorImage_ = nil;
    }
  }

  if (!taskRunner_)
    return;

  // Copy ivars used by the block so that it does not reference |self|.
  UIImage* backgroundingColorImage = backgroundingColorImage_;
  const base::FilePath cacheDirectory = cacheDirectory_;
  const ImageScale snapshotsScale = snapshotsScale_;

  taskRunner_->PostTask(
      FROM_HERE, base::BindBlockArc(^{
        ConvertAndSaveGreyImage(sessionID, snapshotsScale,
                                backgroundingColorImage, cacheDirectory);
      }));
}

- (void)shutdown {
  taskRunner_ = nullptr;
}

@end

@implementation SnapshotCache (TestingAdditions)

- (BOOL)hasImageInMemory:(NSString*)sessionID {
  return [lruCache_ objectForKey:sessionID] != nil;
}

- (BOOL)hasGreyImageInMemory:(NSString*)sessionID {
  return [greyImageDictionary_ objectForKey:sessionID] != nil;
}

- (NSUInteger)lruCacheMaxSize {
  return [lruCache_ maxCacheSize];
}

@end
