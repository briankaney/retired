#!/usr/bin/perl

use Time::Local;

#------------------------------------------------------------------------
#   If no command line args, then print out a usage statement and exit.
#------------------------------------------------------------------------

$argc = @ARGV;

if($argc==0)  {
  print "\n\nUsage:\n";
  print "   append_data.pl real\n";
  exit 0;
  }

$script_dir = "/home/metop/scripts/production/rrct/";

#-----------get current time stamp-----------------------------------------

($sec,$min,$hour,$day,$mon,$year_off)=gmtime();

$total_sec=timegm($sec,$min,$hour,$day,$mon,$year_off);
($sec,$min,$hour,$day,$mon,$year_off)=gmtime($total_sec-3600);

$month=$mon+1;
$year=$year_off+1900;

#--------------------------------------------------------------------------

$command = $script_dir."append_monthly_rrct_refl ".$year." ".$month;
system "$command";

$command = $script_dir."append_monthly_rrct_zdr ".$year." ".$month;
system "$command";

