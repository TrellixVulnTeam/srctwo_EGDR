if (this.document === undefined) {
  importScripts("/common/get-host-info.sub.js")
  importScripts("/resources/testharness.js");
}

var dataURL = "data:text/plain;base64,cmVzcG9uc2UncyBib2R5";
var body = "response's body";
var contentType = "text/plain";

function redirectDataURL(desc, redirectUrl, mode) {
    var url = redirectUrl +  "?cors&location=" + encodeURIComponent(dataURL);

    var requestInit = {"mode": mode};

    promise_test(function(test) {
        return promise_rejects(test, new TypeError(), fetch(url, requestInit));
    }, desc);
}

var redirUrl = get_host_info().HTTP_ORIGIN + "/fetch/api/resources/redirect.py";
var corsRedirUrl = get_host_info().HTTP_REMOTE_ORIGIN + "/fetch/api/resources/redirect.py";

redirectDataURL("Testing data URL loading after same-origin redirection (cors mode)", redirUrl, "cors");
redirectDataURL("Testing data URL loading after same-origin redirection (no-cors mode)", redirUrl, "no-cors");
redirectDataURL("Testing data URL loading after same-origin redirection (same-origin mode)", redirUrl, "same-origin");

redirectDataURL("Testing data URL loading after cross-origin redirection (cors mode)", corsRedirUrl, "cors");
redirectDataURL("Testing data URL loading after cross-origin redirection (no-cors mode)", corsRedirUrl, "no-cors");

done();
