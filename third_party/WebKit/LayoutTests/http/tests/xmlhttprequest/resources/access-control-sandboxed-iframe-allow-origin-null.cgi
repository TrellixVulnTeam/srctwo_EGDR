#!/usr/bin/perl -wT
use strict;

print "Content-Type: text/plain\n";
print "Access-Control-Allow-Credentials: true\n";
print "Access-Control-Allow-Origin: null\n";
print "Access-Control-Allow-External: true\n\n";

print "PASS: Sandboxed iframe XHR access allowed.\n";
