#!/usr/bin/perl -wT
use strict;
use CGI;

my $cgi = new CGI;

print "Content-Type: text/html; charset=UTF-8\n";
print "Content-Security-Policy: ".$cgi->param('csp')."\n\n";

my ($text, $replacement) = ("FAIL", "PASS");
($text, $replacement) = ($replacement, $text) if $cgi->param('should_run') eq 'no';

my $nonce = "";
if (defined $cgi->param('nonce')) {
  $nonce = "nonce='".$cgi->param('nonce')."'";
}


print "<!DOCTYPE html>\n";
print "<html>\n";
print "<body>\n";
print "<div id=\"result\" text=\"$replacement\">\n";
print "$text\n";
print "</div>\n";
print "<script $nonce src=\"".$cgi->param('q')."\"></script>\n";
print "</body>\n";
print "</html>\n";
