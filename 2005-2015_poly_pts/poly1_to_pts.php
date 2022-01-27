#!/usr/bin/php -q
<?php

//--------------------------------------------------------------------------------------
//   If run with no command line args, print out usage statement and bail
//--------------------------------------------------------------------------------------

  $argc = count($argv);

  if($argc==1)
  {
    print "\n\nUsage:\n";
    print "  poly1_to_pts.php input_file\n\n";
    print "Examples:\n";
    print "  ./poly1_to_pts.php sample.poly1\n\n";
    print "  Converts an old version of a *.poly file almost to pts.  Input file should be of the form:\n\n";
    print "  *\n";
    print "  -122.4187 37.8527\n";
    print "  -122.4344 37.8524\n";
    print "  ...\n\n";
    print "  Input longitude and latitude fields can be separated by a variable number of spaces but\n";
    print "  check to see they are in the correct order.\n\n";
    print "  Output will only contain the six required header fields.  Note: the number of points per\n";
    print "  block and the bounding box fields will both only be placeholders and not evaluated.  Use\n";
    print "  'fix_block_number_points.php' first and then 'fix_block_bounding_boxex.php' to get a good\n";
    print "  pts file.  Output is captured via redirect.\n\n";

    exit(0);
  }

//--------------------------------------------------------------------------------------

  $infile = $argv[1];

  $inf = fopen($infile,'r');

  $num_blocks=0;

  while(($buffer = fgets($inf)) !== false)
  {
    $buffer = trim($buffer);
    if($buffer == "*") { ++$num_blocks; }
  }  

  rewind($inf);
  print "$num_blocks\n";

  $i=0;
  while(($buffer = fgets($inf)) !== false)
  {
    $buffer = trim($buffer);
    if($buffer=="*")
    {
      print "$i|-1|-180|90|180|-90|\n";
      ++$i;
    }
    else
    {
      $fields = preg_split('/ +/',$buffer);
      print "$fields[0] $fields[1]\n";
    }
  }  

  fclose($inf);

?>
