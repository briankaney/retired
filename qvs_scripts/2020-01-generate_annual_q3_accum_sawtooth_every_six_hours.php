#!/usr/bin/php -q
<?php

//--------------------------------------------------------------------------------------
//   If run with no command line args, print out usage statement and bail
//--------------------------------------------------------------------------------------

  $argc = count($argv);
  if($argc==1)
  {
    print "\n\nUsage:\n";
    print "  generate_annual_q3_accum_sawtooth_every_six_hours.php real\n";
    print "  generate_annual_q3_accum_sawtooth_every_six_hours.php archive year month day hour\n\n";
    print "Script to build an annual accumulation 'sawtooth'.  This term refers to a set of accumulation files where\n";
    print "each new file in a series adds six hours of precip and then the whole set periodically resets\n";
    print "to zero.  All members of a given sawtooth set will be found in the same directory.\n\n";
    print "The first file will have a timestamp of 06Z on Jan 1 of a given year.  It will contain just the precip since\n";
    print "00Z on Jan 1 that year.  Each subsequent file will contain six additional hours of precip total.  Until\n";
    print "the last file which has a timestamp of 00Z on the 1st day of the next year.  Then the process starts over\n";
    print "in that year's directory.\n\n";

    exit(0);
  }

//--------------------------------------------------------------------------------------
//   Read command line args
//--------------------------------------------------------------------------------------

  $mode = $argv[1];

  if($mode=="real")
  {
    $year  = gmdate('Y');
    $month = gmdate('m');
    $day   = gmdate('d');
    $hour  = gmdate('H');
    $tot_sec = gmmktime($hour-2,0,0,$month,$day,$year);
    $hour  = gmdate('H',$tot_sec);
    $hour = 6*floor($hour/6);

    $year  = gmdate('Y',$tot_sec);
    $month = gmdate('m',$tot_sec);
    $day   = gmdate('d',$tot_sec);
    $hour  = gmdate('H',$tot_sec);
  }

  if($mode=="archive")
  {
    $year  = $argv[2];
    $month = $argv[3];
    $day   = $argv[4];
    $hour  = $argv[5];
  }

  $m_str = sprintf("%02d",$month);
  $d_str = sprintf("%02d",$day);
  $h_str = sprintf("%02d",$hour);

  $exec = "/home/metop/scripts/qpe/add_two_geo_matched_files";

//--------------------------------------------------------------------------------------
//   Special case #1.  The run at the very start of the new year will fill in last file 
//   in last year's directory.  Only triggers once a year (untested as of 9/12/19).
//--------------------------------------------------------------------------------------

  if($month==1 && $day==1 && $hour==0)
  {
    $last_year = $year-1;

    $old_accum = "/localdata/archive/long_term_qpe/".$last_year."/Total/q3ms/Q3MS_Accum.".$last_year."0101.000000_".$last_year."1201.000000.gz";
    $new_file  = "/localdata/archive/long_term_qpe/".$last_year."/12/q3ms/Q3MS_Accum.".$last_year."1201.000000_".$year."0101.000000.gz"; 
    $new_accum = "/localdata/archive/long_term_qpe/".$last_year."/Total/q3ms/Q3MS_Accum.".$last_year."0101.000000_".$year."0101.000000.gz";
    $command = "$exec $old_accum $new_file $new_accum";
    system($command);

    $old_accum = "/localdata/archive/long_term_qpe/".$last_year."/Total/q3evap/Q3Evap_Accum.".$last_year."0101.000000_".$last_year."1201.000000.gz";
    $new_file  = "/localdata/archive/long_term_qpe/".$last_year."/12/q3evap/Q3Evap_Accum.".$last_year."1201.000000_".$year."0101.000000.gz"; 
    $new_accum = "/localdata/archive/long_term_qpe/".$last_year."/Total/q3evap/Q3Evap_Accum.".$last_year."0101.000000_".$year."0101.000000.gz";
    $command = "$exec $old_accum $new_file $new_accum";
    system($command);
    return;
  }

//--------------------------------------------------------------------------------------
//   Special case #2.  The run that is 6 hrs into a new year will start to populate the 
//   new directory for that year.  Just a 'copy' is needed, since accum starts from scratch.
//   Only triggers once a year (untested as of 9/12/19).
//--------------------------------------------------------------------------------------

  if($month==1 && $day==1 && $hour==6)
  {
    $source = "/localdata/archive/long_term_qpe/".$year."/01/q3ms/Q3MS_Accum.".$year."0101.000000_".$year."0101.060000.gz"; 
    $target = "/localdata/archive/long_term_qpe/".$year."/Total/q3ms/Q3MS_Accum.".$year."0101.000000_".$year."0101.060000.gz"; 
    system("cp $source $target");

    $source = "/localdata/archive/long_term_qpe/".$year."/01/q3evap/Q3Evap_Accum.".$year."0101.000000_".$year."0101.060000.gz"; 
    $target = "/localdata/archive/long_term_qpe/".$year."/Total/q3evap/Q3Evap_Accum.".$year."0101.000000_".$year."0101.060000.gz"; 
    system("cp $source $target");
    return;
  }

//--------------------------------------------------------------------------------------
//   If the code is still running at this ponit, then special cases do not apply.
//
//   The new accum files being appended are in monthly directories and if the run 
//   is the very first of the month, then the new accum file to append is in the 
//   prior month's directory.
//--------------------------------------------------------------------------------------

  if($day==1 && $hour==0) { $start_m_str = sprintf("%02d",$month-1); }
  else                    { $start_m_str = $m_str; }

  $old_accum = "/localdata/archive/long_term_qpe/".$year."/Total/q3ms/Q3MS_Accum.".$year."0101.000000_".$year.$start_m_str."01.000000.gz";
  $new_file  = "/localdata/archive/long_term_qpe/".$year."/".$start_m_str."/q3ms/Q3MS_Accum.".$year.$start_m_str."01.000000_".$year.$m_str.$d_str.".".$h_str."0000.gz"; 
  $new_accum = "/localdata/archive/long_term_qpe/".$year."/Total/q3ms/Q3MS_Accum.".$year."0101.000000_".$year.$m_str.$d_str.".".$h_str."0000.gz";

  $command = "$exec $old_accum $new_file $new_accum";
  system($command);

  $old_accum = "/localdata/archive/long_term_qpe/".$year."/Total/q3evap/Q3Evap_Accum.".$year."0101.000000_".$year.$start_m_str."01.000000.gz";
  $new_file  = "/localdata/archive/long_term_qpe/".$year."/".$start_m_str."/q3evap/Q3Evap_Accum.".$year.$start_m_str."01.000000_".$year.$m_str.$d_str.".".$h_str."0000.gz"; 
  $new_accum = "/localdata/archive/long_term_qpe/".$year."/Total/q3evap/Q3Evap_Accum.".$year."0101.000000_".$year.$m_str.$d_str.".".$h_str."0000.gz";

  $command = "$exec $old_accum $new_file $new_accum";
  system($command);

?>
