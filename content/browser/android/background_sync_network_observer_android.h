// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_ANDROID_BACKGROUND_SYNC_NETWORK_OBSERVER_ANDROID_H_
#define CONTENT_BROWSER_ANDROID_BACKGROUND_SYNC_NETWORK_OBSERVER_ANDROID_H_

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/browser/background_sync/background_sync_network_observer.h"
#include "content/public/browser/browser_thread.h"

namespace content {

// BackgroundSyncNetworkObserverAndroid is a specialized
// BackgroundSyncNetworkObserver which is backed by a NetworkChangeNotifier
// that listens for network events even when the browser is paused, unlike the
// standard NetworkChangeNotifier. This ensures that sync events can be fired
// even when the browser is backgrounded, and other network observers are
// disabled.
class BackgroundSyncNetworkObserverAndroid
    : public BackgroundSyncNetworkObserver {
 public:
  // Creates a BackgroundSyncNetworkObserver. |network_changed_callback| is
  // called via PostMessage when the network connection changes.
  BackgroundSyncNetworkObserverAndroid(
      const base::Closure& network_changed_callback);

  ~BackgroundSyncNetworkObserverAndroid() override;

  // This class lives on the UI thread and mediates all access to the Java
  // BackgroundSyncNetworkObserver, which it creates and owns. It is in turn
  // owned by the BackgroundSyncNetworkObserverAndroid.
  class Observer : public base::RefCountedThreadSafe<
                       BackgroundSyncNetworkObserverAndroid::Observer,
                       content::BrowserThread::DeleteOnUIThread> {
   public:
    static scoped_refptr<BackgroundSyncNetworkObserverAndroid::Observer> Create(
        base::Callback<void(net::NetworkChangeNotifier::ConnectionType)>
            callback);

    // Called from BackgroundSyncNetworkObserver.java over JNI whenever the
    // connection type changes. This updates the current connection type seen by
    // this class and calls the |network_changed_callback| provided to the
    // constructor, on the IO thread, with the new connection type.
    void NotifyConnectionTypeChanged(
        JNIEnv* env,
        const base::android::JavaParamRef<jobject>& jcaller,
        jint new_connection_type);

   private:
    friend struct BrowserThread::DeleteOnThread<BrowserThread::UI>;
    friend class base::DeleteHelper<
        BackgroundSyncNetworkObserverAndroid::Observer>;

    Observer(base::Callback<void(net::NetworkChangeNotifier::ConnectionType)>
                 callback);
    void Init();
    ~Observer();

    // This callback is to be run on the IO thread whenever the connection type
    // changes.
    base::Callback<void(net::NetworkChangeNotifier::ConnectionType)> callback_;
    base::android::ScopedJavaGlobalRef<jobject> j_observer_;

    DISALLOW_COPY_AND_ASSIGN(Observer);
  };

 private:
  // Accessed on UI Thread
  scoped_refptr<Observer> observer_;

  base::WeakPtrFactory<BackgroundSyncNetworkObserverAndroid> weak_ptr_factory_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_ANDROID_BACKGROUND_SYNC_NETWORK_OBSERVER_ANDROID_H_
