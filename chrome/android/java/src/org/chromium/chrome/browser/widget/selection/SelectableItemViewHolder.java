// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.widget.selection;

import android.support.v7.widget.RecyclerView.ViewHolder;
import android.view.View;

/**
 * An ViewHolder for a {@link SelectableItemView}.
 * @param <E> The type of the item associated with the {@link SelectableItemView}.
 */
public class SelectableItemViewHolder<E> extends ViewHolder {
    private SelectableItemView<E> mItemView;

    /**
     * @param itemView The {@link SelectableItemView} to be held by this ViewHolder.
     * @param delegate The {@link SelectionDelegate} for the itemView.
     */
    @SuppressWarnings("unchecked")
    public SelectableItemViewHolder(View itemView, SelectionDelegate<E> delegate) {
        super(itemView);
        mItemView = (SelectableItemView<E>) itemView;
        mItemView.setSelectionDelegate(delegate);
    }

    /**
     * @param item The item to display in the {@link SelectableItemView} held by this
     *             {@link ViewHolder}.
     */
    public void displayItem(E item) {
        mItemView.setItem(item);
    }

    /**
     * @return The {@link SelectableItemView} held by this ViewHolder.
     */
    public SelectableItemView<E> getItemView() {
        return mItemView;
    }
}
