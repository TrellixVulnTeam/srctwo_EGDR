// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ntp.cards;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;

/**
 * Placeholder item to let the snippets flow to the top of the scroll list even when it does not
 * contain enough of them. It is displayed as a dummy item with variable height that just occupies
 * the remaining space between the last item in the RecyclerView and the bottom of the screen.
 */
public class SpacingItem extends OptionalLeaf {
    private static class SpacingView extends View {
        public SpacingView(Context context) {
            super(context);
        }

        @Override
        protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
            setMeasuredDimension(
                    0, ((NewTabPageRecyclerView) getParent()).calculateBottomSpacing());
        }
    }

    /** Creates the View object for displaying the variable spacing. */
    public static View createView(ViewGroup parent) {
        return new SpacingView(parent.getContext());
    }

    public SpacingItem() {
        setVisibilityInternal(true);
    }

    @Override
    @ItemViewType
    protected int getItemViewType() {
        return ItemViewType.SPACING;
    }

    @Override
    protected void onBindViewHolder(NewTabPageViewHolder holder) {
        // Nothing to do.
    }

    @Override
    public void visitOptionalItem(NodeVisitor visitor) {
        visitor.visitSpacingItem();
    }

    /** Schedules a recalculation of the space occupied by the item. */
    public void refresh() {
        if (isVisible()) notifyItemChanged(0);
    }

    public void setVisible(boolean visible) {
        setVisibilityInternal(visible);
    }
}