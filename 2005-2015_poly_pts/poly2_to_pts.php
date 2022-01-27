#!/usr/bin/php -q
<?php

//--------------------------------------------------------------------------------------
//   If run with no command line args, print out usage statement and bail
//--------------------------------------------------------------------------------------

  $argc = count($argv);

  if($argc==1)
  {
    print "\n\nUsage:\n";
    print "  poly2_to_pts.php input_file\n\n";
    print "Examples:\n";
    print "  ./poly2_to_pts.php sample.poly2\n\n";
    print "  Converts an old version of a *.poly file to pts.  Input file should be of the form:\n\n";
    print "  2\n";
    print "  * 11\n";
    print "  -122.4187 37.8527\n";
    print "  -122.4344 37.8524\n";
    print "  ...\n\n";
    print "  Input fields can be separated by a variable number of spaces.  Check to make sure long/lat\n";
    print "  are in the correct order.  Output will only contain the six required header fields.  Note: the\n";
    print "  bounding box fields will only be placeholders.  Use 'fix_block_bounding_boxes.php' to fill\n";
    print "  these in and get a good pts file.  Output is captured via redirect.\n\n";

    exit(0);
  }

//--------------------------------------------------------------------------------------

  $infile = $argv[1];

  $inf = fopen($infile,'r');

  $num_blocks = trim(fgets($inf));
  print "$num_blocks\n";

  for($i=0;$i<$num_blocks;++$i)
  {
    $buffer = trim(fgets($inf));

    $col = preg_split('/ +/',$buffer);
    $num_pts = $col[1];
    print "$i|$num_pts|-180|90|180|-90|\n";

    for($j=0;$j<$num_pts;++$j)
    {
      $buffer = trim(fgets($inf));
      $col = preg_split('/ +/',$buffer);
      print "$col[0] $col[1]\n";
    }
  }

  fclose($inf);

?>
