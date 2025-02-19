// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.test.util.browser.contextmenu;

import android.app.Activity;
import android.app.Instrumentation;
import android.text.TextUtils;
import android.view.ContextMenu;
import android.view.MenuItem;

import org.junit.Assert;

import org.chromium.base.test.util.CallbackHelper;
import org.chromium.chrome.browser.tab.EmptyTabObserver;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;
import org.chromium.content.browser.test.util.DOMUtils;

import java.util.concurrent.TimeoutException;

/**
 * A utility class to help open and interact with context menus triggered by a WebContents.
 */
public class ContextMenuUtils {
    /**
     * Callback helper that also provides access to the last display ContextMenu.
     */
    private static class OnContextMenuShownHelper extends CallbackHelper {
        private ContextMenu mContextMenu;

        public void notifyCalled(ContextMenu menu) {
            mContextMenu = menu;
            notifyCalled();
        }

        public ContextMenu getContextMenu() {
            assert getCallCount() > 0;
            return mContextMenu;
        }
    }

    /**
     * Opens a context menu.
     * @param tab                   The tab to open a context menu for.
     * @param openerDOMNodeId       The DOM node to long press to open the context menu for.
     * @return                      The {@link ContextMenu} that was opened.
     * @throws InterruptedException
     * @throws TimeoutException
     */
    public static ContextMenu openContextMenu(Tab tab, String openerDOMNodeId)
            throws InterruptedException, TimeoutException {
        String jsCode = "document.getElementById('" + openerDOMNodeId + "')";
        return openContextMenuByJs(tab, jsCode);
    }

    /**
     * Opens a context menu.
     * @param tab                   The tab to open a context menu for.
     * @param jsCode                The javascript to get the DOM node to long press to
     *                              open the context menu for.
     * @return                      The {@link ContextMenu} that was opened.
     * @throws InterruptedException
     * @throws TimeoutException
     */
    public static ContextMenu openContextMenuByJs(Tab tab, String jsCode)
            throws InterruptedException, TimeoutException {
        final OnContextMenuShownHelper helper = new OnContextMenuShownHelper();
        tab.addObserver(new EmptyTabObserver() {
            @Override
            public void onContextMenuShown(Tab tab, ContextMenu menu) {
                helper.notifyCalled(menu);
                tab.removeObserver(this);
            }
        });
        int callCount = helper.getCallCount();
        DOMUtils.longPressNodeByJs(tab.getContentViewCore(), jsCode);

        helper.waitForCallback(callCount);
        return helper.getContextMenu();
    }

    /**
     * Opens and selects an item from a context menu.
     * @param tab                   The tab to open a context menu for.
     * @param openerDOMNodeId       The DOM node to long press to open the context menu for.
     * @param itemId                The context menu item ID to select.
     * @param activity              The activity to assert for gaining focus after click or null.
     * @throws InterruptedException
     * @throws TimeoutException
     */
    public static void selectContextMenuItem(Instrumentation instrumentation, Activity activity,
            Tab tab, String openerDOMNodeId, final int itemId)
            throws InterruptedException, TimeoutException {
        String jsCode = "document.getElementById('" + openerDOMNodeId + "')";
        selectContextMenuItemByJs(instrumentation, activity, tab, jsCode, itemId);
    }

    /**
     * Long presses to open and selects an item from a context menu.
     * @param tab                   The tab to open a context menu for.
     * @param jsCode                The javascript to get the DOM node to long press
     *                              to open the context menu for.
     * @param itemId                The context menu item ID to select.
     * @param activity              The activity to assert for gaining focus after click or null.
     * @throws InterruptedException
     * @throws TimeoutException
     */
    public static void selectContextMenuItemByJs(Instrumentation instrumentation, Activity activity,
            Tab tab, String jsCode, final int itemId)
            throws InterruptedException, TimeoutException {
        ContextMenu menu = openContextMenuByJs(tab, jsCode);
        Assert.assertNotNull("Failed to open context menu", menu);

        selectOpenContextMenuItem(instrumentation, activity, menu, itemId);
    }

    /**
     * Opens and selects an item from a context menu.
     * @param tab                   The tab to open a context menu for.
     * @param openerDOMNodeId       The DOM node to long press to open the context menu for.
     * @param itemTitle             The title of the context menu item to select.
     * @throws InterruptedException
     * @throws TimeoutException
     */
    public static void selectContextMenuItemByTitle(Instrumentation instrumentation,
            Activity activity, Tab tab, String openerDOMNodeId, String itemTitle)
            throws InterruptedException, TimeoutException {
        ContextMenu menu = openContextMenu(tab, openerDOMNodeId);
        Assert.assertNotNull("Failed to open context menu", menu);

        Integer itemId = null;
        for (int i = 0; i < menu.size(); i++) {
            MenuItem item = menu.getItem(i);
            if (TextUtils.equals(item.getTitle(), itemTitle)) {
                itemId = item.getItemId();
                break;
            }
        }
        Assert.assertNotNull("Couldn't find context menu item for '" + itemTitle + "'", itemId);

        selectOpenContextMenuItem(instrumentation, activity, menu, itemId);
    }

    private static void selectOpenContextMenuItem(Instrumentation instrumentation,
            final Activity activity, final ContextMenu menu, final int itemId) {
        MenuItem item = menu.findItem(itemId);
        Assert.assertNotNull("Could not find '" + itemId + "' in menu", item);
        Assert.assertTrue("'" + itemId + "' is not visible", item.isVisible());
        Assert.assertTrue("'" + itemId + "' is not enabled", item.isEnabled());

        instrumentation.runOnMainSync(new Runnable() {
            @Override
            public void run() {
                boolean activated = menu.performIdentifierAction(itemId, 0);
                Assert.assertTrue("Failed to activate '" + itemId + "' in menu", activated);
            }
        });

        if (activity != null) {
            CriteriaHelper.pollInstrumentationThread(
                    new Criteria("Activity did not regain focus.") {
                        @Override
                        public boolean isSatisfied() {
                            return activity.hasWindowFocus();
                        }
                    });
        }
    }
}
