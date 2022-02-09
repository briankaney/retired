<!DOCTYPE html>
<?php
//calls//|mrms-web.dmz.nssl|/var/www/html/common/css/leaflet-0.7.3.attribution_z-index_20.css|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/css/qvs.css|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/qvs/long_qpe_accum/long_qpe_accum.css|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/php/library_versioning_info.php|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/js/leaflet-0.7.3.js|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/js/sci/sci_base.js|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/js/sci/sci_mapping.js|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/js/sci/sci_windows.js|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/js/widgets/zoom_pan.js|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/js/qvs/qvs_standard_map_overlays.js|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/js/qvs/qvs_standard_base_maps.js|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/qvs/long_qpe_accum/calendar_select_unstacked.js|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/qvs/long_qpe_accum/long_qpe_accum.js|tracked||
?>

<html lang="en">
  <head>
    <link rel="stylesheet" href="/common/css/leaflet-0.7.3.attribution_z-index_20.css" type="text/css">
    <link rel="stylesheet" href="/common/css/qvs.css" type="text/css">
    <link rel="stylesheet" href="long_qpe_accum.css?version=20200331" type="text/css">
    <title>Custom Long Term QPE</title>

<?php
  include '/var/www/html/common/php/library_versioning_info.php';

  $cur_year  = gmdate('Y');
  $cur_month = gmdate('m');
  $cur_day   = gmdate('d');
  $cur_hour  = gmdate('H');
  $tot_sec = gmmktime($cur_hour-2,0,0,$cur_month,$cur_day+1,$cur_year);
  $cur_hour  = gmdate('H',$tot_sec);
  $cur_hour  = 6*floor($cur_hour/6);

  $tot_sec = gmmktime($cur_hour,0,0,$cur_month,$cur_day+1,$cur_year);
  $end_epoch_year   = gmdate('Y',$tot_sec);
  $end_epoch_month  = gmdate('m',$tot_sec);
  $end_epoch_day    = gmdate('d',$tot_sec);

  $tot_sec = gmmktime($cur_hour,0,0,$cur_month,$cur_day,$cur_year);
  $end_year   = gmdate('Y',$tot_sec);
  $end_month  = gmdate('m',$tot_sec);
  $end_day    = gmdate('d',$tot_sec);
  $end_hour   = gmdate('H',$tot_sec);

  $tot_sec = gmmktime($end_hour,0,0,$end_month,$end_day-12,$end_year);
  $start_year   = gmdate('Y',$tot_sec);
  $start_month  = gmdate('m',$tot_sec);
  $start_day    = gmdate('d',$tot_sec);
  $start_hour   = gmdate('H',$tot_sec);

  $qpe_type = "Q3Evap";
  $palette_type = "QPE_24Color_StepRainbow_0-060.0in_mmx10";
  $opacity = 1;
  $zoom = 4;
  $clon = -95;
  $clat = 39;
  $base = 0;
  $overlays = 1;
  $readout = "off";

  if(isset($_GET['end_epoch_year']))   $end_epoch_year = escapeshellcmd($_GET['end_epoch_year']);
  if(isset($_GET['end_epoch_month']))  $end_epoch_month = escapeshellcmd($_GET['end_epoch_month']);
  if(isset($_GET['end_epoch_day']))    $end_epoch_day = escapeshellcmd($_GET['end_epoch_day']);

  if(isset($_GET['qpe_type'])) $qpe_type = escapeshellcmd($_GET['file_type']);
  if(isset($_GET['palette_type'])) $palette_type = escapeshellcmd($_GET['palette_type']);
  if(isset($_GET['opacity'])) $opacity = escapeshellcmd($_GET['opacity']);
  if(isset($_GET['zoom'])) $zoom = escapeshellcmd($_GET['zoom']);
  if(isset($_GET['clon'])) $clon = escapeshellcmd($_GET['clon']);
  if(isset($_GET['clat'])) $clat = escapeshellcmd($_GET['clat']);
  if(isset($_GET['base'])) $base = escapeshellcmd($_GET['base']);
  if(isset($_GET['overlays'])) $overlays = escapeshellcmd($_GET['overlays']);
  if(isset($_GET['readout'])) $readout = escapeshellcmd($_GET['readout']);

  print "<script type=\"text/javascript\">\n";

    print "var qpe_type = \"$qpe_type\";\n";
    print "var palette_type = \"$palette_type\";\n";
    print "var opacity = $opacity;\n";

    print "var zoom = $zoom;\n";
    print "var clon = $clon;\n";
    print "var clat = $clat;\n";
    print "var base = $base;\n";
    print "var overlays = $overlays;\n";
    print "var readout = \"$readout\";\n";

    print "var end_epoch_year   = $end_epoch_year;\n";
    print "var end_epoch_month  = $end_epoch_month;\n";
    print "var end_epoch_day    = $end_epoch_day;\n";
    print "var end_year   = $end_year;\n";
    print "var end_month  = $end_month;\n";
    print "var end_day    = $end_day;\n";
    print "var end_hour   = $end_hour;\n";
    print "var start_year   = $start_year;\n";
    print "var start_month  = $start_month;\n";
    print "var start_day    = $start_day;\n";
    print "var start_hour   = $start_hour;\n";

  print "</script>\n";
?>
    <script src="/common/js/leaflet-0.7.3.js"></script>

    <script src="/common/js/sci/sci_base.js?version=20200331" type="text/javascript"></script>
    <script src="/common/js/sci/sci_mapping.js" type="text/javascript"></script>
    <script src="/common/js/sci/sci_windows.js" type="text/javascript"></script>

    <script src="/common/js/widgets/zoom_pan.js" type="text/javascript"></script>
    <script src="calendar_select_unstacked.js" type="text/javascript"></script>
<?php
    $file = "<script src=\"/common/js/qvs/qvs_standard_base_maps.js?version=".$qvs_standard_base_maps_version."\" type=\"text/javascript\"></script>";
    print "$file\n";
    $file = "<script src=\"/common/js/qvs/qvs_standard_map_overlays.js?version=".$qvs_standard_map_overlays_version."\" type=\"text/javascript\"></script>";
    print "$file\n";
?>

    <script src="long_qpe_accum.js?version=20200331" type="text/javascript"></script>
  </head>

  <body>
    <div id="banner">
      <div id="banner_drop"></div>
      <div id="banner_strip"></div>
      <div id="banner_storm"></div>
    </div>
    <div id="main_title" class="banner_title">Custom QPE Accumulations</div>
    <div id="sub_title" class="banner_sub_title">Time Intervals: Any range since Feb 1, 2019 in 6 hour increments</div>

    <div id="left_panel">
      <div id="start_accum_label" class="section_label">Start Accumulation:</div>
      <div id="end_accum_label" class="section_label">End Accumulation:</div>
      <canvas id="time_check" width="250" height="28"></canvas>

      <div id="file_select_label" class="section_label">Product Type:</div>

      <div id="prod_def_one"><b>Q3 DP Evap</b>:  A dual-pol radar synthetic QPE based on specific attenuation, 
            specific differential phase and vertical profile corrected reflectivity.  An evaporation correction based 
            on radar data height and environmental relative humidity is applied.</div>
      <div id="prod_def_two"><b>Q3 MS</b>: A multi-sensor QPE that merges dual-pol radar QPE, quality controlled gauge 
            observations and precipitation climatology.</div>

      <div id="palette_select_label" class="section_label">Color Palette Type:</div>

      <div id="readout_label" class="section_label">Product Readout</div>
      <div id="readout_select"><input type="checkbox" id="readout_check" name="READ"></div>
      <div id="readout_output">
        <div id="readout_user_label" class="small_label">Data Value</div>
        <div id="readout_output_user" class="label_div"></div>
        <div id="prod_read_info">Hover mouse over map for value readout.  Uncheck box to resume zoom/pan functionality.</div>
      </div>
    </div>

    <div id="map_panel">
      <div id="base_map" class="main_map"></div>
      <canvas id="product_layer" class="main_map" width="920" height="630" style="z-index:21"></canvas>
      <div id="overlay_layer" class="main_map" style="z-index:22"></div>

      <canvas id="zoom" class="main_map" width="920" height="630" style="z-index:23"></canvas>
      <canvas id="zoom_bar" width="21" height="196"></canvas>
      <div id="zoom_readout" class="label_div"></div>
      <div id="title" class="label_div"></div>
      <div id="readout_layer" class="main_map" style="z-index:24;"></div>
    </div>

    <div id="legend_panel"></div>

    <div id="footer_div">
      <div id="opacity_select_label" class="section_label">Product Opacity</div>
      <div id="wheel_enable_label" class="small_label">Enable Mouse Wheel</div>
    </div>

    <div id="overlays_window" class="floating_window_div">
    </div>
    <div id="base_window" class="floating_window_div">
      <canvas id="base_select" width="300" height="67"></canvas>
    </div>
  </body>
</html>
