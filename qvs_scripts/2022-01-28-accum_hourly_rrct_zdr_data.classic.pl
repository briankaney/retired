#!/usr/bin/perl

use Time::Local;

#------------------------------------------------------------------------
#   If no command line args, then print out a usage statement and exit.
#------------------------------------------------------------------------

$argc = @ARGV;

if($argc==0)  {
  print "\n\nUsage:\n";
  print "   accum_hourly_rrct_zdr_data.pl real\n";
  exit 0;
  }

#$mode=@ARGV[0];  # Need something to skip usage statement, but the value is not used.

#--------------hard coded paths here---------------------------------------

$radar_pairs = "/web_data/qvs/ref_data/rrct/RRCT_Pair_Indecies.txt";
$out_radcal_data_dir = "/web_data/qvs_data/rrct/";
$in_radcal_data_dir  = "/nfsdata/realtime/fetched/rrct/comparisons_Zdr/";

#-----------get current time stamp-----------------------------------------

($sec,$min,$current_hour,$current_day,$current_mon,$current_year_off)=gmtime();

#------------generate time stamp for 60 min ago----------------

$total_sec=timegm($sec,$min,$current_hour,$current_day,$current_mon,$current_year_off);
($sec,$min,$hour,$day,$mon,$year_off)=gmtime($total_sec-3600);

$min = 5 * int($min/5);
$month=$mon+1;
$year=$year_off+1900;

if(length($month)<2)  {  $month="0".$month;  }
if(length($day)<2)    {  $day="0".$day;      }
if(length($hour)<2)   {  $hour="0".$hour;    }
if(length($min)<2)    {  $min="0".$min;      }

$timestamp= $year.$month.$day.".".$hour.$min."00";

#--------------------------------------------------------------------------

open(PAIRS,"<$radar_pairs") || die print "Can't open file - $radar_pairs\n";
@pair_data=<PAIRS>;
close(PAIRS);

$num_pairs=@pair_data;

$num_new=0;
for($i=0;$i<$num_pairs;++$i)  {
  ($pair_name) = split(/ /,@pair_data[$i]);

  $new_file = $in_radcal_data_dir.$pair_name.".".$timestamp.".dat";
  $monthly_file = $out_radcal_data_dir.$year."/".$month."/zdr/".$pair_name."_".$year.$month."_ZDR_Comparison";
  if(-e $new_file)  {
    system "cat $monthly_file $new_file > $monthly_file.temp";
    system "mv -f $monthly_file.temp $monthly_file";
    ++$num_new;
    }
  }

print "$num_new data lines added for $month/$day/$year $hour:$min\n";


