// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.offline_items_collection;

import org.chromium.base.annotations.SuppressFBWarnings;

/**
 * This class is the Java counterpart to the C++ OfflineItem
 * (components/offline_items_collection/core/offline_item.h) class.
 *
 * For all member variable descriptions see the C++ class.
 * TODO(dtrainor): Investigate making all class members for this and the C++ counterpart const.
 */
@SuppressFBWarnings("URF_UNREAD_PUBLIC_OR_PROTECTED_FIELD")
public class OfflineItem {
    /**
     * This class is the Java counterpart to the C++ OfflineItemProgress
     * (components/offline_items_collection/core/offline_item.h) class.
     */
    public static class Progress {
        public final long value;
        public final Long max;
        @OfflineItemProgressUnit
        public final int unit;

        public Progress(long value, Long max, int unit) {
            this.value = value;
            this.max = max;
            this.unit = unit;
        }

        /** Helper method to create an indeterminate progress. */
        public static Progress createIndeterminateProgress() {
            return new Progress(0, null, OfflineItemProgressUnit.PERCENTAGE);
        }

        /** Whether the progress is indeterminate. */
        public boolean isIndeterminate() {
            return max == null;
        }

        /** Returns the percentage value. Should not be called on an indeterminate progress. */
        public int getPercentage() {
            assert max != null;
            return max == 0 ? 100 : (int) (value * 100 / max);
        }

        @Override
        public boolean equals(Object obj) {
            if (obj instanceof Progress) {
                Progress other = (Progress) obj;
                return value == other.value && unit == other.unit
                        && (max == other.max || (max != null && max.equals(other.max)));
            }
            return false;
        }

        @Override
        public int hashCode() {
            int result = (int) value;
            result = 31 * result + (max == null ? 0 : max.hashCode());
            result = 31 * result + unit;
            return result;
        }
    }

    public ContentId id;

    // Display metadata.
    public String title;
    public String description;
    @OfflineItemFilter
    public int filter;
    public boolean isTransient;

    // Content Metadata.
    public long totalSizeBytes;
    public boolean externallyRemoved;
    public long creationTimeMs;
    public long lastAccessedTimeMs;
    public boolean isOpenable;

    // Request Metadata.
    public String pageUrl;
    public String originalUrl;
    public boolean isOffTheRecord;

    // In Progress Metadata.
    @OfflineItemState
    public int state;
    public boolean isResumable;
    public boolean allowMetered;
    public long receivedBytes;
    public Progress progress;
    public long timeRemainingMs;

    public OfflineItem() {
        id = new ContentId();
        filter = OfflineItemFilter.FILTER_OTHER;
        state = OfflineItemState.COMPLETE;
    }
}
