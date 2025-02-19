// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.android.webview.chromium;

import android.annotation.TargetApi;
import android.os.Build;
import android.os.Handler;
import android.webkit.WebMessage;
import android.webkit.WebMessagePort;

import org.chromium.content.browser.AppWebMessagePort;
import org.chromium.content_public.browser.MessagePort;

/**
 * This class is used to convert a WebMessagePort to a MessagePort in chromium
 * world.
 */
@TargetApi(Build.VERSION_CODES.M)
public class WebMessagePortAdapter extends WebMessagePort {
    private MessagePort mPort;

    public WebMessagePortAdapter(MessagePort port) {
        mPort = port;
    }

    public void postMessage(WebMessage message) {
        mPort.postMessage(message.getData(), toMessagePorts(message.getPorts()));
    }

    public void close() {
        mPort.close();
    }

    public void setWebMessageCallback(WebMessageCallback callback) {
        setWebMessageCallback(callback, null);
    }

    public void setWebMessageCallback(final WebMessageCallback callback, final Handler handler) {
        mPort.setMessageCallback(new MessagePort.MessageCallback() {
            @Override
            public void onMessage(String message, MessagePort[] ports) {
                callback.onMessage(WebMessagePortAdapter.this,
                        new WebMessage(message, fromMessagePorts(ports)));
            }
        }, handler);
    }

    public MessagePort getPort() {
        return mPort;
    }

    public static WebMessagePort[] fromMessagePorts(MessagePort[] messagePorts) {
        if (messagePorts == null) return null;
        WebMessagePort[] ports = new WebMessagePort[messagePorts.length];
        for (int i = 0; i < messagePorts.length; i++) {
            ports[i] = new WebMessagePortAdapter(messagePorts[i]);
        }
        return ports;
    }

    public static MessagePort[] toMessagePorts(WebMessagePort[] webMessagePorts) {
        if (webMessagePorts == null) return null;
        MessagePort[] ports = new AppWebMessagePort[webMessagePorts.length];
        for (int i = 0; i < webMessagePorts.length; i++) {
            ports[i] = ((WebMessagePortAdapter) webMessagePorts[i]).getPort();
        }
        return ports;
    }
}
