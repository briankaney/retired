#!/usr/bin/php -q
<?php

//--------------------------------------------------------------------------------------
//   If run with no command line args, print out usage statement and bail
//--------------------------------------------------------------------------------------

  $argc = count($argv);

  if($argc==1)
  {
    print "\n\nUsage:\n";
    print "  poly4_to_pts.php input_file\n\n";

    print "Examples:\n";
    print "  ./poly4_to_pts.php sample.poly4\n\n";

    print "  Converts an old version of a *.poly file to pts.  Input file should be of the form:\n\n";

    print "  2\n";
    print "  * 0 11\n";
    print "  -122.4324 37.8623\n";
    print "  -122.4463 -122.4185   37.8524 37.8722\n";
    print "  US County name Island\n";
    print "  Block Text Can Contain Spaces\n";
    print "  -122.4187 37.8527\n";
    print "  -122.4344 37.8524\n";
    print "  ...\n\n";

    print "  Output will only contain the six required header fields.  The old header contained all\n";
    print "  the info needed to fill these and will be trusted to be correct.  Check to see that\n";
    print "  longitude and latitude are in the correct order.  Also note that the bounding box field\n";
    print "  order is shuffled between the input and output.  Output is captured via redirect.\n\n";

    exit(0);
  }

//--------------------------------------------------------------------------------------

  $infile = $argv[1];

  $inf = fopen($infile,'r');

  $buffer = trim(fgets($inf));
  $col = preg_split('/ +/',$buffer);
  $num_blocks = $col[0];
  print "$num_blocks\n";

  for($i=0;$i<$num_blocks;++$i)
  {
    $buffer = trim(fgets($inf));
    $col = preg_split('/ +/',$buffer);
    $num_pts = $col[2];

    fgets($inf);  //--skip line 2 of header block (centroid)

    $buffer = trim(fgets($inf));
    $col = preg_split('/ +/',$buffer);
    $wlon = $col[0];
    $nlat = $col[3];
    $elon = $col[1];
    $slat = $col[2];

    print "$i|$num_pts|$wlon|$nlat|$elon|$slat|\n";

    fgets($inf);  //--skip line 4 and 5 of header (text metadata)
    fgets($inf);

    for($j=0;$j<$num_pts;++$j)
    {
      $buffer = trim(fgets($inf));
      $col = preg_split('/ +/',$buffer);
      print "$col[0] $col[1]\n";
    }
  }

  fclose($inf);

?>
