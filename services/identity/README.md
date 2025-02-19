# Identity Service User Manual

The Identity Service maintains and provides access to the user's signed-in
identities with Google.

This service is currently in development. The design doc is here:

https://docs.google.com/document/d/1EPLEJTZewjiShBemNP5Zyk3b_9sgdbrZlXn7j1fubW0

The tracking bug is https://crbug.com/654990.

Below, we provide a user manual to performing various tasks via the Identity
Service, comparing to how you would have performed these tasks via the relevant
interfaces from //google_apis/gaia and //components/signin.

## Connecting to the Identity Manager

The primary interface exposed by the Identity Service is the Identity Manager.
Here is an [example CL](https://chromium-review.googlesource.com/c/539637/)
illustrating connection to the Identity Manager from a client
(specifically, IdentitySigninFlow).

## Obtaining the Information of the Primary Account

There is no way to obtain the information of the primary account synchronously,
i.e., there is no literal equivalent to
SigninManagerBase::GetAuthenticatedAccountInfo(). Instead, you should call the
asynchronous method IdentityManager::GetPrimaryAccountInfo(). To date, all use
cases of SigninManagerBase::GetAuthenticatedAccountInfo() that we have seen have
been part of a larger asynchronous flow (e.g., obtaining an access token in
response to an invocation of the chrome.identity extension API). In these use
cases, you can simply fold the asynchronous obtaining of the authenticated
account info into the larger asynchronous flow.

There is no one way to do this as the "larger asynchronous flow" in question
will be different from case to case, but here are a
[couple](https://chromium-review.googlesource.com/c/536754/)
[example](https://chromium-review.googlesource.com/c/558096/) CLs.

If the above guidance does not suffice for your use case, please contact
blundell@chromium.org.

## Obtaining an Access Token

Where you would have called OAuth2TokenService::StartRequest(), you should
instead call IdentityManager::GetAccessToken(). This interface is a 1:1
correspondence that should transparently work in all use cases, although there
are currently still some input parameters that need to be added to its API to
match esoteric variants of the OAuth2TokenService API. Here is an [example
CL](https://chromium-review.googlesource.com/c/514047/) that illustrates
conversion of a client.

If you were using or looking to use MintOAuth2TokenFlow, please contact
blundell@chromium.org. There is currently no corresponding interface in the
Identity Service, and we are looking to understand whether/how it is necessary
to work this class into the Identity Service interface.

## Being Notified on Signin of the Primary Account

If you were previously listening to SigninManagerBase::GoogleSigninSucceeded()
or OAuth2TokenService::OnRefreshTokenIsAvailable() to determine when the primary
account is available, you should call
IdentityManager::GetPrimaryAccountWhenAvailable(). This method will fire when
the authenticated account is signed in and has a refresh token available. Here
is an [example CL](https://chromium-review.googlesource.com/c/539637/)
illustrating this pattern.

## Determining if an Account Has a Refresh Token Available

There is no way to determine this information synchronously, i.e., there is no
literal equivalent to OAuth2TokenService::IsRefreshTokenAvailable().  To date,
all use cases of this method that we have seen have been part of a larger
asynchronous flow (e.g., determining whether the user needs to sign in before we
can obtain an access token in response to an invocation of the chrome.identity
extension API). In these use cases, you can simply call
IdentityManager::GetPrimaryAccountInfo() and check the AccountState that is
returned from that call, folding the asynchronous checking into the larger
asynchronous flow. Here is an [example
CL](https://chromium-review.googlesource.com/c/538672/) illustrating this
pattern.

If the above guidance does not suffice for your use case, please contact
blundell@chromium.org.

## Being Notified When An Account Has a Refresh Token Available

All of the use cases of OAuth2TokenService::OnRefreshTokenAvailable() that we
have seen to date have been for the purpose of knowing when the authenticated
account is available (i.e., signed in with a refresh token available). For this
purpose, you should call IdentityManager::GetPrimaryAccountWhenAvailable(). Here
is an [example CL](https://chromium-review.googlesource.com/c/539637/) illustrating the conversion.

If the above guidance does not suffice for your use case, please contact
blundell@chromium.org.

## Observing Signin-Related Events

There are plans to build a unified Observer interface that will supersede the
various current Observer interfaces (AccountTracker, OAuth2TokenService,
SigninManager, AccountTrackerService). However, this functionality has not yet
been built. Contact blundell@chromium.org with your use case, which can help
drive the bringup of this interface.

## Obtaining the Information of All Accounts

If you are currently calling AccountTracker::GetAccounts(),
AccountTrackerService::GetAccounts(), or OAuth2TokenService::GetAccounts(), the
corresponding interface in the Identity Service is
IdentityManager::GetAccounts(). Note the semantics of this method carefully (as
described in its documentation). In particular, this method returns only
accounts that have a refresh token available, which is not necessarily the
same behavior as AccountTracker::GetAccounts() or
AccountTrackerService::GetAccounts() (but *is* the same behavior as
OAuth2TokenService::GetAccounts()). If your use case is difficult to
implement with the semantics of this method, contact blundell@chromium.org.

## Other Needs

If you have any need that is not covered by the above guidance, contact
blundell@chromium.org.
