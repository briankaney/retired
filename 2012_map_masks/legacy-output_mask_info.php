#!/usr/bin/php -q
<?php

//--------------------------------------------------------------------------------------
//   If run with no command line args, print out usage statement and bail
//--------------------------------------------------------------------------------------

  $argc = count($argv);

  if($argc==1)
  {
    print "\n\nUsage:\n";
    print "   legacy-output_mask_info.php [options] input_file num_x num_y\n\n";
    print "Examples:\n";
    print "   ./legacy-output_mask_info.php Mask.CNTY.2231.2byte.gz 7001 3501\n";
    print "   ./legacy-output_mask_info.php -b=1 Mask.CNTY.2230.1byte.gz 7001 3501\n\n";

    print "   Given a mask file this utility outputs some information about the file.  This utility\n";
    print "   works on older 'legacy' mask files which had no header.  The dimensions are output\n";
    print "   but these are just the user supplied values.  The legacy files had no lat/long reference\n";
    print "   or resolution stored.  The utility counts the number of occurrences of each flag value\n";
    print "   that is used in the file (the number of possible flags is capped at 32,766) and outputs\n";
    print "   a list of these counts.\n\n";
    print "   The legacy files came in both a 1byte and 2byte version.  The default is 2byte but use\n";
    print "   a '-b=1' option for 1byte files.\n\n";

    exit(0);
  }

//--------------------------------------------------------------------------------------
//   Read command line arguments.  No use of $argv after this point.  Test for existence
//   of input file.
//--------------------------------------------------------------------------------------

  if($argc<4) { print "Error:  Required command line arguments missing\n\n";  exit(0); }

  $mask = $argv[$argc-3];
  $num_x = $argv[$argc-2];
  $num_y = $argv[$argc-1];

  if(!file_exists($mask)) { print "Error: $mask file not found\n\n";  exit(0); }

//--------------------------------------------------------------------------------------
//   Check for option to set for 1 or 2 byte input files.
//--------------------------------------------------------------------------------------

  $mode = "2byte";

  for($i=1;$i<=$argc-4;++$i)
  {
    if($argv[$i]=="-b=1") { $mode = "1byte";   break; }
  }

//--------------------------------------------------------------------------------------
//   Set up a fixed length array of bin_counts and initialize to zeros.
//--------------------------------------------------------------------------------------

  $bin_count = Array();
  for($i=0;$i<32767;++$i) { $bin_count[$i] = 0; }

//--------------------------------------------------------------------------------------
//   Read input file
//--------------------------------------------------------------------------------------

  $inf = gzopen($mask,"rb");

  $num_pts = $num_x*$num_y;
  
  if($mode=="1byte") {
    $binary_str = gzread($inf,$num_pts);
    $format = "c".$num_pts;
  }

  if($mode=="2byte") {
    $binary_str = gzread($inf,$num_pts*2);
    $format = "s".$num_pts;
  }

  $values = unpack($format,$binary_str);

  $flags = Array();
  for($i=1;$i<=$num_pts;++$i)
  {
    $str = strval($i);
    $flags[$i-1] = $values[$str];
  }

  gzclose($inf);

//--------------------------------------------------------------------------------------
//   Increment the bin counts.
//--------------------------------------------------------------------------------------

  for($i=0;$i<$num_x*$num_y;++$i)
  {
    if($flags[$i]<32767) { ++$bin_count[$flags[$i]]; }
  }

//--------------------------------------------------------------------------------------
//   Print out dimensions and bin counts.
//--------------------------------------------------------------------------------------

  print "\n";
  $str = "Dimensions: ".$num_x."x".$num_y;
  print "$str\n\n";

  $n=0;
  for($i=0;$i<32767;++$i)
  {
    if($bin_count[$i]>0)
    {
      ++$n;
      printf("%-3d  Flag: %-3d   Count: %-8d\n",$n,$i,$bin_count[$i]);
    }
  }
  print "\n";

?>
