#!/ur/bin/php -q
<?php

//--------------------------------------------------------------------------------------
//   If run with no command line args, print out usage statement and bail
//--------------------------------------------------------------------------------------

  $argc = count($argv);
  if($argc==1)
  {
    print "\n\nUsage:\n";
    print "  generate_monthly_q3_accum_sawtooth_every_hour.php_str qpe real\n";
    print "  generate_monthly_q3_accum_sawtooth_every_hour.php_str qpe archive year month day hour\n\n";
    print "Script to build a monthly accumulation 'sawtooth'.  This term refers to a set of accumulation files where\n";
    print "a new hour of precip is added each hour and then the whole set periodically resets to zero.  All members of\n";
    print "of a given sawtooth set will be found in the same directory (/localdata/archive/long_term_qpe/YYYY/MM/qpe_str/)\n";
    print "The first file will have a timestamp of 01Z on the 1st of the month.  It will contain just the precip since\n";
    print "00Z on the 1st of the month.  Each subsequent file will contain one additional hour of precip total.  Until\n";
    print "the last file which has a timestamp of 00Z on the 1st day of the next month.  Then the process starts over\n";
    print "in that next month's directory.  The arg qpe_str can be 'q3ms' or 'q3evap'.\n\n";

    exit(0);
  }

//--------------------------------------------------------------------------------------
//   Read command line args
//--------------------------------------------------------------------------------------

  $qpe  = $argv[1];
  $mode = $argv[2];

  if($mode=="real")
  {
    $year  = gmdate('Y');
    $month = gmdate('m');
    $day   = gmdate('d');
    $hour  = gmdate('H');
    if($qpe=="q3ms")
    {
      $tot_sec = gmmktime($hour-1,0,0,$month,$day,$year);

      $year  = gmdate('Y',$tot_sec);
      $month = gmdate('m',$tot_sec);
      $day   = gmdate('d',$tot_sec);
      $hour  = gmdate('H',$tot_sec);
    }
  }

  if($mode=="archive")
  {
    $year  = $argv[3];
    $month = $argv[4];
    $day   = $argv[5];
    $hour  = $argv[6];
  }

//--------------------------------------------------------------------------------------

  $tot_sec = gmmktime($hour-1,0,0,$month,$day,$year);
  $prior_year  = gmdate('Y',$tot_sec);
  $prior_month = gmdate('m',$tot_sec);
  $prior_day   = gmdate('d',$tot_sec);
  $prior_hour  = gmdate('H',$tot_sec);

//--------------------------------------------------------------------------------------

  $prior_month_str = sprintf("%02d",$prior_month);
  $prior_day_str   = sprintf("%02d",$prior_day);
  $prior_hour_str  = sprintf("%02d",$prior_hour);

  $month_str = sprintf("%02d",$month);
  $day_str   = sprintf("%02d",$day);
  $hour_str  = sprintf("%02d",$hour);

//--------------------------------------------------------------------------------------

  $exec = "/home/metop/scripts/qpe/add_two_geo_matched_files";

  if($qpe=="q3evap") { $new_hour  = "/localdata/realtime/fetched/VMRMS/CONUS/q3evap/1h_acc/1HRAD.EVAP.".$year.$month_str.$day_str.".".$hour_str."0000.gz"; }
  if($qpe=="q3ms")   { $new_hour  = "/localdata/realtime/fetched/VMRMS/CONUS/q3ms/1h_acc/1HMS.".$year.$month_str.$day_str.".".$hour_str."0000.gz"; }

  if($qpe=="q3evap") { $file_str = "q3evap/Q3Evap"; }
  if($qpe=="q3ms")   { $file_str = "q3ms/Q3MS"; }

  if($prior_month==$month)
  {
    $old_accum = "/localdata/archive/long_term_qpe/".$year."/".$month_str."/".$file_str."_Accum.".$year.$month_str.
                 "01.000000_".$year.$month_str.$prior_day_str.".".$prior_hour_str."0000.gz";
    $new_accum = "/localdata/archive/long_term_qpe/".$year."/".$month_str."/".$file_str."_Accum.".$year.$month_str.
                 "01.000000_".$year.$month_str.$day_str.".".$hour_str."0000.gz";
  }

//--------------------------------------------------------------------------------------
//  On first file of new month, accumulate as usual but write output to prior month dir.
//--------------------------------------------------------------------------------------

  else
  {
    $old_accum = "/localdata/archive/long_term_qpe/".$prior_year."/".$prior_month_str."/".$file_str."_Accum.".$prior_year.
                 $prior_month_str."01.000000_".$prior_year.$prior_month_str.$prior_day_str.".".$prior_hour_str."0000.gz";
    $new_accum = "/localdata/archive/long_term_qpe/".$prior_year."/".$prior_month_str."/".$file_str."_Accum.".$prior_year.
                 $prior_month_str."01.000000_".$year.$month_str.$day_str.".".$hour_str."0000.gz";
  }

//--------------------------------------------------------------------------------------
//  On 2nd run of new month, we need to start a new set.
//--------------------------------------------------------------------------------------

  if($day==1 && $hour==1) { system("cp $new_hour $new_accum"); }

  if(!file_exists($old_accum)) { print "Fatal error: no long term accum $old_accum\n\n";  exit(0); }
  if(!file_exists($new_hour))
  { 
    print "Warning: no new hourly file $new_hour\n\n";
    system("cp $old_accum $new_accum");
    exit(0);
  }

  $command = "$exec $old_accum $new_hour $new_accum";
  system($command);

?>
