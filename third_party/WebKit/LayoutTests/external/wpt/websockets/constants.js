//This file requires server-side substitutions and must be included as constants.js?pipe=sub

var PORT = "{{ports[ws][0]}}";
//FIXME: Add support for wss
var PORT_SSL = "{{ports[ws][0]}}";

var SCHEME_DOMAIN_PORT;
if (location.search == '?wss') {
  SCHEME_DOMAIN_PORT = 'wss://{{host}}:' + PORT_SSL;
} else {
  SCHEME_DOMAIN_PORT = 'ws://{{host}}:' + PORT;
}
