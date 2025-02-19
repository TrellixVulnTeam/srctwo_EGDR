// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.payments;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.os.StrictMode;
import android.os.StrictMode.ThreadPolicy;

import org.chromium.base.ContextUtils;

import java.util.List;

import javax.annotation.Nullable;

/** Abstraction of Android's package manager to enable testing. */
public class PackageManagerDelegate {
    /**
     * Retrieves package information of an installed application.
     *
     * @param packageName The package name of an installed application.
     * @return The package information of the installed application.
     */
    @SuppressLint("PackageManagerGetSignatures")
    public PackageInfo getPackageInfoWithSignatures(String packageName) {
        try {
            return ContextUtils.getApplicationContext().getPackageManager().getPackageInfo(
                    packageName, PackageManager.GET_SIGNATURES);
        } catch (NameNotFoundException e) {
            return null;
        }
    }

    /**
     * Retrieves the single activity that matches the given intent, or null if none found.
     * @param intent The intent to query.
     * @return The matching activity.
     */
    public ResolveInfo resolveActivity(Intent intent) {
        ThreadPolicy oldPolicy = StrictMode.allowThreadDiskReads();
        try {
            return ContextUtils.getApplicationContext().getPackageManager().resolveActivity(
                    intent, 0);
        } finally {
            StrictMode.setThreadPolicy(oldPolicy);
        }
    }

    /**
     * Retrieves the list of activities that can respond to the given intent.
     * @param intent The intent to query.
     * @return The list of activities that can respond to the intent.
     */
    public List<ResolveInfo> getActivitiesThatCanRespondToIntent(Intent intent) {
        ThreadPolicy oldPolicy = StrictMode.allowThreadDiskReads();
        try {
            return ContextUtils.getApplicationContext().getPackageManager().queryIntentActivities(
                    intent, 0);
        } finally {
            StrictMode.setThreadPolicy(oldPolicy);
        }
    }

    /**
     * Retrieves the list of activities that can respond to the given intent. And returns the
     * activites' meta data in ResolveInfo.
     *
     * @param intent The intent to query.
     * @return The list of activities that can respond to the intent.
     */
    public List<ResolveInfo> getActivitiesThatCanRespondToIntentWithMetaData(Intent intent) {
        ThreadPolicy oldPolicy = StrictMode.allowThreadDiskReads();
        try {
            return ContextUtils.getApplicationContext().getPackageManager().queryIntentActivities(
                    intent, PackageManager.GET_META_DATA);
        } finally {
            StrictMode.setThreadPolicy(oldPolicy);
        }
    }

    /**
     * Retrieves the list of services that can respond to the given intent.
     * @param intent The intent to query.
     * @return The list of services that can respond to the intent.
     */
    public List<ResolveInfo> getServicesThatCanRespondToIntent(Intent intent) {
        ThreadPolicy oldPolicy = StrictMode.allowThreadDiskReads();
        try {
            return ContextUtils.getApplicationContext().getPackageManager().queryIntentServices(
                    intent, 0);
        } finally {
            StrictMode.setThreadPolicy(oldPolicy);
        }
    }

    /**
     * Retrieves the label of the app.
     * @param resolveInfo The identifying information for an app.
     * @return The label for this app.
     */
    public CharSequence getAppLabel(ResolveInfo resolveInfo) {
        return resolveInfo.loadLabel(ContextUtils.getApplicationContext().getPackageManager());
    }

    /**
     * Retrieves the icon of the app.
     * @param resolveInfo The identifying information for an app.
     * @return The icon for this app.
     */
    public Drawable getAppIcon(ResolveInfo resolveInfo) {
        return resolveInfo.loadIcon(ContextUtils.getApplicationContext().getPackageManager());
    }

    /**
     * Gets the string array resource of the given application.
     *
     * @param applicationInfo The application info.
     * @param resourceId      The identifier of the string array resource.
     * @return The string array resource, or null if not found.
     */
    @Nullable
    public String[] getStringArrayResourceForApplication(
            ApplicationInfo applicationInfo, int resourceId) {
        Resources resources;
        try {
            resources = ContextUtils.getApplicationContext()
                                .getPackageManager()
                                .getResourcesForApplication(applicationInfo);
        } catch (NameNotFoundException e) {
            return null;
        }
        return resources == null ? null : resources.getStringArray(resourceId);
    }
}
