#!/usr/bin/php -q
<?php

//--------------------------------------------------------------------------------------
//   If run with no command line args, print out usage statement and bail
//--------------------------------------------------------------------------------------

  $argc = count($argv);

  if($argc==1)
  {
    print "\n\nUsage:\n";
    print "   legacy-view_mask_as_image.php [options] input_mask_file num_x num_y output_image_file\n\n";
    print "Examples:\n";
    print "   ./legacy-view_mask_as_image.php Mask.CNTY.2231.2byte.gz 7001 3501 2byte-ok-cherokee.png\n";
    print "   ./legacy-view_mask_as_image.php -b=1 Mask.CNTY.2230.1byte.gz 7001 3501 1byte-ok-cherokee.png\n\n";

    print "   A way to get a simple, quick view of a raster map.  User specifies an input raster\n";
    print "   map file and a name for the output image.  The output will be a png with the same\n";
    print "   dimensions as the raster map.  It will cover the full lat/long region of the mask.\n";
    print "   Flag zero regions will be black and all non-zero flags will rotate thru a 12 color\n";
    print "   palette.  This utility works on older 'legacy' mask files which had no header.\n";
    print "   Those came in both a 1byte and 2byte version - the default is 2byte and a '-b=1'\n";
    print "   must be added to use 1byte masks.\n\n";
    print "   Other map utilities will be needed to get a map with more options for size, region\n";
    print "   coverage, Mercator projection, and color choices.\n\n";

    exit(0);
  }

//--------------------------------------------------------------------------------------
//   Read command line arguments.  No use of $argv after this point.  Test for existence
//   of input file.
//--------------------------------------------------------------------------------------

  if($argc<5) { print "Error:  Required command line arguments missing\n\n";  exit(0); }

  $mask = $argv[$argc-4];
  $num_x = $argv[$argc-3];
  $num_y = $argv[$argc-2];
  $img  = $argv[$argc-1];

  if(!file_exists($mask)) { print "Error: $mask file not found\n\n";  exit(0); }

//--------------------------------------------------------------------------------------
//   Check for option to set for 1 or 2 byte input files.
//--------------------------------------------------------------------------------------

  $mode = "2byte";

  for($i=1;$i<=$argc-5;++$i)
  {
    if($argv[$i]=="-b=1") { $mode = "1byte";   break; }
  }

//--------------------------------------------------------------------------------------
//   Create an all black image the same dimensions as the raster map
//--------------------------------------------------------------------------------------

  $image = imagecreate($num_x,$num_y);

  $black = imagecolorallocate($image,0,0,0);
  imagefilledrectangle($image,0,0,$num_x,$num_y,$black);

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

  $colors = Array();
  $colors[0] = imagecolorallocate($image,255,255,255);
  $colors[1] = imagecolorallocate($image,0,0,255);
  $colors[2] = imagecolorallocate($image,0,230,230);
  $colors[3] = imagecolorallocate($image,0,255,0);
  $colors[4] = imagecolorallocate($image,180,255,0);
  $colors[5] = imagecolorallocate($image,255,255,0);
  $colors[6] = imagecolorallocate($image,255,180,0);
  $colors[7] = imagecolorallocate($image,255,0,0);
  $colors[8] = imagecolorallocate($image,255,0,255);
  $colors[9] = imagecolorallocate($image,200,0,120);
  $colors[10] = imagecolorallocate($image,180,90,0);
  $colors[11] = imagecolorallocate($image,180,180,180);

//--------------------------------------------------------------------------------------
//   Open input mask, step through all the points and set the image pixel color 
//   according to the flag value.
//--------------------------------------------------------------------------------------

  $inf = gzopen($mask,"rb");

  for($y=0;$y<$num_y;++$y)
  {
    for($x=0;$x<$num_x;++$x)
    {
      if($mode=="1byte")
      {
        $binary_str = gzread($inf,1);
        $value = unpack("c1",$binary_str);
      }

      if($mode=="2byte")
      {
        $binary_str = gzread($inf,2);
        $value = unpack("s1",$binary_str);
      }

      $number = $value['1'];
      if($number==0) { continue; }
      $color = $colors[($number-1)%12];
      imagesetpixel($image,$x,$y,$color);
    }
  }

//--------------------------------------------------------------------------------------
//   Close input file, write out the final image and clean up memory
//--------------------------------------------------------------------------------------

  gzclose($inf);
  imagepng($image,$img);
  imagedestroy($image);

?>
