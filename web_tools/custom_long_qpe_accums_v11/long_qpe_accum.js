//############################################################################
//       index.js
//       by Brian T Kaney, Feb 2019
//############################################################################
//calls//|mrms-web.dmz.nssl|/var/www/html/common/images/general/Background-OpacitySlider-172x45.svg|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/common/images/product_legends/[svg]|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/qvs/long_qpe_accum/fetch_long_qpe_accum_image_from_annual_sawtooth.php|tracked||
//calls//|mrms-web.dmz.nssl|/var/www/html/qvs/long_qpe_accum/fetch_long_qpe_accum_value_from_annual_sawtooth.php|tracked||

document.addEventListener("DOMContentLoaded",function() { InitAll() },false);

function InitAll() {
  StartCalendar.setDiv('left_panel',18,40,'start_calendar','#EEEEEE');
//  StartCalendar.setCanvas('left_panel',8,36,'start_calendar',210,140,'#EEEEEE');
  StartCalendar.initialize();
  EndCalendar.setDiv('left_panel',250,40,'end_calendar','#EEEEEE');
//  EndCalendar.setCanvas('left_panel',244,36,'end_calendar',210,140,'#EEEEEE');
  EndCalendar.initialize();
  RealityCheckTime();

  QPESelect.setDiv('left_panel',150,382);
  QPESelect.initialize();

  PaletteSelect.setDiv('left_panel',192,570);
  PaletteSelect.initialize();

  BaseSelect.initialize();
  BaseMap.setLeafMap(zoom,clat,clon);
  BaseMap.initialize();

  OverlaySelect.initialize();
  OverlayLayer.setMap(zoom,clat,clon);
  OverlayLayer.setDimensions(920,630);
  OverlayLayer.updateOverlayLayer();

  createImageDiv('footer_div','slider_back',184,19,172,45,'url(/common/images/general/Background-OpacitySlider-172x45.svg)');
  OpacitySlider.allowed_step = 10;
  OpacitySlider.setCanvas('footer_div',214,42);
  OpacitySlider.initialize(opacity*100);
  OpacitySlider.updateRestOfPage = function() {
    opacity= this.current_pixel/100;
    document.getElementById('product_layer').style.opacity = opacity.toFixed(1);
    };

  ProdLayer.prod = qpe_type;
  ProdLayer.palette = palette_type;
  ProdLayer.setMap(zoom,clon,clat);
  ProdLayer.initialize();
  document.getElementById('legend_panel').style.backgroundImage = "url(/common/images/product_legends/" + 
                          file_legends[PaletteSelect.getIndexOfSelectedOption()] + ")";
  DrawTitle();

  ZoomPan.zi_map.setMap(zoom,clon,clat);
  ZoomPan.setZoomPanLimits(3,15,-180,180,-85.05,85.05);
  ZoomPan.initialize();

  ResetRegion.setDiv('footer_div',452,30);
  ResetRegion.initialize();
  WheelEnable.setDiv('footer_div',706,30);
  WheelEnable.initialize();

  ReadoutLayer.prod = qpe_type;
  ReadoutLayer.initialize(readout);
  if(readout=="on")  document.getElementById('readout_check').checked = true;
  document.getElementById('readout_check').addEventListener('change',readout_select_handler,false);

/*
  TimeWindow.setTimes(start_epoch,start_time,end_time,end_epoch);
  TimeWindow.initialize();
  TimeWindow.updateRestOfPage = function() {
    start_time.equateToTime(this.start_time);
    end_time.equateToTime(this.end_time);
    DrawTitle();
    ProdLayer.updateProductLayer();
    };
*/

  divWin.initialize();

  return;
  }

//var end_epoch  = new time(end_epoch_year,end_epoch_month,end_epoch_day,0,0,0);
//var start_epoch = end_epoch.spawnTime(-30,'days');
//var start_time = new time(start_year,start_month,start_day,start_hour,start_minute,0);
//var end_time = new time(end_year,end_month,end_day,end_hour,end_minute,0);
//var TimeWindow = new dayWindowSlider('time_window');





//var minute_resolution = 360;
var minute_resolution = 360;

var end_time = new time(end_year,end_month,end_day,end_hour,0,0);
var start_time = new time(start_year,start_month,start_day,start_hour,0,0);

var end_archive = new time(0,3600);
var start_archive = new time(2019,2,1,0,0,0);

var StartCalendar = new calendarSelect();
StartCalendar.setTimes(start_archive,start_time,end_archive,minute_resolution);

StartCalendar.updateRestOfPage = function() {
  start_time.equateToTime(this.selected_time);

  RealityCheckTime();
  DrawTitle();
  ProdLayer.updateProductLayer();
  };

var EndCalendar = new calendarSelect();
EndCalendar.setTimes(start_archive,end_time,end_archive,minute_resolution);

EndCalendar.updateRestOfPage = function() {
  end_time.equateToTime(this.selected_time);

  RealityCheckTime();
  DrawTitle();
  ProdLayer.updateProductLayer();
  };

function RealityCheckTime() {
  canvasClear('time_check');
  var ctx = document.getElementById('time_check').getContext('2d');
  ctx.font = "10pt Arial";

  var num_hours = subtractTimes(end_time,start_time,'hours');

  if(num_hours<=0) {
    ctx.fillStyle = "#000000";
    ctx.fillText("Not Valid:",6,19);
    ctx.fillStyle = "#FF0000";
    ctx.fillText("End Time <= Start Time",72,19);
    return;
    }

  ctx.fillStyle = "#000000";
  ctx.fillText("Current Interval:",6,19);

  var time_window_str;
  if(num_hours>0 && num_hours<24)       time_window_str = num_hours + " Hours";
  if(num_hours>=24 && num_hours%24==0)  time_window_str = num_hours/24 + " Days";
  if(num_hours>=24 && num_hours%24!=0)  time_window_str = Math.floor(num_hours/24) + " Days, " + num_hours%24 + " Hours";
  ctx.fillStyle = "#0000FF";
  ctx.fillText(time_window_str,108,19);
  }
















//----------------------------------------------------------------------------

var OpacitySlider = new singleSlider('opacity_select','horiz',100);

//----------------------------------------------------------------------------

var QPESelect = new ddMenu('qpe_select_dd',qpe_type);
QPESelect.options = ["Q3DP","Q3Evap","Q3GC","Q3MS","Q3CBPCK","HRRRF01","STAGE4"];
QPESelect.display_strs = ["Q3 Dual Pol","Q3 DP Evap Corrected","Q3 Gauge Corrected (pass2)","Q3 Multi-Sensor (pass2)","Q3 CBPCK","HRRR Forecast 1hr","Stage IV"];

QPESelect.updateRestOfPage = function() {
  qpe_type = this.selected_option;

  ReadoutLayer.prod = qpe_type;
  ProdLayer.prod = qpe_type;
  ProdLayer.updateProductLayer();
  document.getElementById('legend_panel').style.backgroundImage = "url(/common/images/product_legends/" + 
                          file_legends[PaletteSelect.getIndexOfSelectedOption()] + ")";
  DrawTitle();
  }

var file_legends = [
"",
"QPE_24Color_StepRainbow_0-003.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-005.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-008.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-012.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-016.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-024.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-032.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-040.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-050.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-060.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-075.0in_80x600.svg",
"QPE_24Color_StepRainbow_0-090.0in_80x600.svg",
"QPE_WPC_7Color_0-020.0in_80x600.svg",
"",
"QPE_256Color_GreyScale_0-003.0in_80x600.svg",
"QPE_256Color_GreyScale_0-005.0in_80x600.svg",
"QPE_256Color_GreyScale_0-008.0in_80x600.svg",
"QPE_256Color_GreyScale_0-012.0in_80x600.svg",
"QPE_256Color_GreyScale_0-016.0in_80x600.svg",
"QPE_256Color_GreyScale_0-024.0in_80x600.svg",
"QPE_256Color_GreyScale_0-032.0in_80x600.svg",
"QPE_256Color_GreyScale_0-040.0in_80x600.svg",
"QPE_256Color_GreyScale_0-050.0in_80x600.svg",
"QPE_256Color_GreyScale_0-060.0in_80x600.svg",
"QPE_256Color_GreyScale_0-075.0in_80x600.svg",
"QPE_256Color_GreyScale_0-090.0in_80x600.svg"
];

//----------------------------------------------------------------------------

var PaletteSelect = new ddMenu('palette_select_dd',palette_type);
PaletteSelect.options = [
"div",
"QPE_24Color_StepRainbow_0-003.0in_mmx10",
"QPE_24Color_StepRainbow_0-005.0in_mmx10",
"QPE_24Color_StepRainbow_0-008.0in_mmx10",
"QPE_24Color_StepRainbow_0-012.0in_mmx10",
"QPE_24Color_StepRainbow_0-016.0in_mmx10",
"QPE_24Color_StepRainbow_0-024.0in_mmx10",
"QPE_24Color_StepRainbow_0-032.0in_mmx10",
"QPE_24Color_StepRainbow_0-040.0in_mmx10",
"QPE_24Color_StepRainbow_0-050.0in_mmx10",
"QPE_24Color_StepRainbow_0-060.0in_mmx10",
"QPE_24Color_StepRainbow_0-075.0in_mmx10",
"QPE_24Color_StepRainbow_0-090.0in_mmx10",
"QPE_WPC_7Color_0-020.0in_mmx10",
"div",
"QPE_256Color_GreyScale_0-003.0in_mmx10",
"QPE_256Color_GreyScale_0-005.0in_mmx10",
"QPE_256Color_GreyScale_0-008.0in_mmx10",
"QPE_256Color_GreyScale_0-012.0in_mmx10",
"QPE_256Color_GreyScale_0-016.0in_mmx10",
"QPE_256Color_GreyScale_0-024.0in_mmx10",
"QPE_256Color_GreyScale_0-032.0in_mmx10",
"QPE_256Color_GreyScale_0-040.0in_mmx10",
"QPE_256Color_GreyScale_0-050.0in_mmx10",
"QPE_256Color_GreyScale_0-060.0in_mmx10",
"QPE_256Color_GreyScale_0-075.0in_mmx10",
"QPE_256Color_GreyScale_0-090.0in_mmx10"
];

PaletteSelect.display_strs = [
"Color Steps",
"0 - 3.0 inch",
"0 - 5.0 inch",
"0 - 8.0 inch",
"0 - 12 inch",
"0 - 16 inch",
"0 - 24 inch",
"0 - 32 inch",
"0 - 40 inch",
"0 - 50 inch",
"0 - 60 inch",
"0 - 75 inch",
"0 - 90 inch",
"WPC/NCEP",
"Continuous Grey Scale",
"0 - 3.0 inch",
"0 - 5.0 inch",
"0 - 8.0 inch",
"0 - 12 inch",
"0 - 16 inch",
"0 - 24 inch",
"0 - 32 inch",
"0 - 40 inch",
"0 - 50 inch",
"0 - 60 inch",
"0 - 75 inch",
"0 - 90 inch"
];

PaletteSelect.updateRestOfPage = function() {
  palette_type = this.selected_option;

  ProdLayer.palette = palette_type;
  ProdLayer.updateProductLayer();
  document.getElementById('legend_panel').style.backgroundImage = "url(/common/images/product_legends/" + 
                          file_legends[PaletteSelect.getIndexOfSelectedOption()] + ")";
  DrawTitle();
  }

//----------------------------------------------------------------------------

var ProdLayer = new productLayer('product_layer');

//----------------------------------------------------------------------------

var ZoomPan = new zoomPanOneMap("zoom");

ZoomPan.updateRestOfPage = function() {
  if(BaseMap.leaf_map._animatingZoom === true)  return;

  this.bar.selected_index = findIndex(this.bar.options,this.next_zoom);
  this.bar.draw(-1);
  this.zi_map.setMap(this.next_zoom,this.next_clon,this.applyLatLimits(this.next_clat));

  zoom = this.zi_map.zoom_level;
  clat = this.zi_map.center_lat;
  clon = this.zi_map.center_lon;

  BaseMap.setLeafMap(clat,clon,zoom);
  BaseMap.leaf_map.setView([clat,clon],zoom);
  OverlayLayer.setMap(zoom,clat,clon);
  OverlayLayer.updateOverlayLayer();
  ProdLayer.setMap(zoom,clon,clat);
  ProdLayer.updateProductLayer();
  ReadoutLayer.ro_map.setMap(zoom,clon,clat);
  }

//----------------------------------------------------------------------------

var ResetRegion = new textDivButton('reset_region','Reset Region');
ResetRegion.updateRestOfPage = function() {
  zoom = 4;    clon = -98;    clat = 38;
  ZoomPan.bar.current_index = 15 - zoom;
  ZoomPan.bar.draw(-1);
  ZoomPan.zi_map.setMap(zoom,clon,clat);
  ZoomPan.drawReadoutText(-999,-999);
  BaseMap.setLeafMap(clat,clon,zoom);
  BaseMap.leaf_map.setView([clat,clon],zoom);
  OverlayLayer.setMap(zoom,clat,clon);
  OverlayLayer.updateOverlayLayer();
  ProdLayer.setMap(zoom,clon,clat);
  ProdLayer.updateProductLayer();
  ReadoutLayer.ro_map.setMap(zoom,clon,clat);
  }

var WheelEnable = new checkBox('wheel_enable','off');
  WheelEnable.updateRestOfPage = function() {
  if(this.state == 'on')   {  ZoomPan.wheel_enable = "on";   }
  if(this.state == 'off')  {  ZoomPan.wheel_enable = "off";  }
  };

//----------------------------------------------------------------------------

var BaseSelect = new baseSelect('base_select',base);
BaseSelect.updateRestOfPage = function() {
  base = this.base_index;
  BaseMap.base_index = base;
  BaseMap.drawBaseLayer();
  }

var BaseMap = new baseMap('base_map',base);

var OverlaySelect = new standardCONUSOverlays('overlays_window',overlays);
OverlaySelect.updateRestOfPage = function() {
  overlays = this.overlay_index;
  OverlayLayer.overlay_index = overlays;
  OverlayLayer.updateOverlayLayer();
  };

var OverlayLayer = new overlayLayer('overlay_layer',overlays);

//----------------------------------------------------------------------------

var divWin = new divWindowSet(2);
divWin.window_id = ["overlays","base"];
divWin.window_width = [300,300];
divWin.window_status = ["closed","closed"];

divWin.park_container_id = "footer_div";
divWin.park_win_label = ["Overlays","Base Map Layer"];
divWin.park_win_width = [120,120];
divWin.park_win_top = [16,44];
divWin.park_win_left = [762,762];
divWin.park_win_color = ["#DDE6DD","#DDE6DD"];

//----------------------------------------------------------------------------

function productLayer(canvas_id) {
  this.product_canvas_id = canvas_id;
  this.prod;
  this.palette;
  this.zoom;
  this.clon;
  this.clat;
  this.width;
  this.height;

  this.image;
  this.frame_call_url;
  }

productLayer.prototype.setMap = function(zoom,clon,clat) {
  this.zoom = zoom;
  this.clon = clon;
  this.clat = clat;
  };

productLayer.prototype.initialize = function() {
  this.width = document.getElementById(this.product_canvas_id).width;
  this.height = document.getElementById(this.product_canvas_id).height;

  this.updateProductLayer();
  };

productLayer.prototype.updateProductLayer = function() {
  var palette_str = "/home/metop/web/ref_data/palettes/vmrms/" + this.palette + ".pal";

  this.frame_call_url = "/qvs/long_qpe_accum/fetch_long_qpe_accum_image_from_annual_sawtooth.php?prod=" + this.prod + "&start_yr=" + 
        start_time.year + "&start_mo=" + start_time.month + "&start_dy=" + start_time.day + "&start_hr=" + start_time.hour + 
        "&end_yr=" + end_time.year + "&end_mo=" + end_time.month + "&end_dy=" + end_time.day + "&end_hr=" + end_time.hour + 
        "&zoom=" + this.zoom + "&clon=" + this.clon + "&clat=" + this.clat + 
        "&image_blank=/home/metop/web/binary_prod_image_drawing/ProductBlank-920x630.png&palette=" + palette_str + 
        "&width=" + this.width + "&height=" + this.height;

  canvasClear(this.product_canvas_id);

  var num_hours = subtractTimes(end_time,start_time,'hours');
  if(num_hours<=0) { return; }

  this.image = new Image();
  this.image.onload = function() {
    var ctx = document.getElementById(this.product_canvas_id).getContext('2d');
    ctx.drawImage(this.image,0,0);
    }.bind(this);
  this.image.src = this.frame_call_url;
  };

function DrawTitle() {
  if(ProdLayer.prod=="Q3DP")     var prod_str = "Q3 DP";
  if(ProdLayer.prod=="Q3Evap")   var prod_str = "Q3 DP Evap";
  if(ProdLayer.prod=="Q3MS")     var prod_str = "Q3 MS (pass2)";
  if(ProdLayer.prod=="Q3GC")     var prod_str = "Q3 GC (pass2)";
  if(ProdLayer.prod=="Q3CBPCK")  var prod_str = "Q3 CBPCK";
  if(ProdLayer.prod=="HRRRF01")  var prod_str = "HRRR Fore 1hr";
  if(ProdLayer.prod=="STAGE4")   var prod_str = "Stage IV";

  var title_str = prod_str + ": " + timeString(start_time,"MM/DD/YYYY HH:MM Z","UTC") +" to " + 
                  timeString(end_time,"MM/DD/YYYY HH:MM Z","UTC");
  document.getElementById('title').innerHTML = title_str;
  }

//----------------------------------------------------------------------------

var ReadoutLayer = new readoutLayer('readout_layer','readout_output');
ReadoutLayer.ro_map.setDimensions(920,630);
ReadoutLayer.ro_map.setMap(zoom,clon,clat);

function readout_select_handler(e) {
  if(document.getElementById('readout_check').checked == true) {
    document.getElementById('readout_layer').style.visibility = "visible";
    document.getElementById('readout_output').style.visibility = "visible";
    } 
  if(document.getElementById('readout_check').checked == false) {
    document.getElementById('readout_layer').style.visibility = "hidden";
    document.getElementById('readout_output').style.visibility = "hidden";
    }
  }

//----------------------------------------------------------------------------

function readoutLayer(layer_id,output_id) {
  this.readout_layer_id = layer_id;
  this.readout_output_id = output_id;
  this.readout_user_id = output_id + "_user";
  this.prod;

  this.timer_handle;
  this.timer_set = 0;
  this.cursor_lat;
  this.cursor_lon;

  this.ro_map = new mapBox();
  this.readout_value;
  this.clear_readout;
  }

readoutLayer.prototype.initialize = function(view_state) {
  if(view_state=="on") {
    document.getElementById(this.readout_layer_id).style.visibility = "visible";
    document.getElementById(this.readout_output_id).style.visibility = "visible";
    }
  else { 
    document.getElementById(this.readout_layer_id).style.visibility = "hidden";
    document.getElementById(this.readout_output_id).style.visibility = "hidden";
    }
  document.getElementById(this.readout_layer_id).addEventListener('mousemove',this.moveAction.bind(this),false);
  document.getElementById(this.readout_layer_id).addEventListener('mouseout',this.outAction.bind(this),false);
  };

readoutLayer.prototype.moveAction = function(e) {
  e.preventDefault();
  this.clear_readout = 0;

  var pt = getMouseScreenPoint(this.readout_layer_id,e);
  this.cursor_lon = lonFromRefLonPlusPix(this.ro_map.zoom_level,this.ro_map.center_lon,pt.x-this.ro_map.pix_width/2);
  this.cursor_lat = latFromRefLatPlusPix(this.ro_map.zoom_level,this.ro_map.center_lat,this.ro_map.pix_height/2-pt.y);

  if(this.timer_set==1)   clearTimeout(this.timer_handle);
  this.timer_handle = setTimeout(this.loadValueJSON.bind(this),250);
  this.timer_set=1;  
  };

readoutLayer.prototype.outAction = function(e) {
  this.clear_readout = 1;   //--tricky, json response from 'old' mousemoves can come in after a mouseout, this keeps the readout div clear
  };

readoutLayer.prototype.loadValueJSON = function() {
  var request = new XMLHttpRequest();
  request.onload = function () { this.parseReadoutReq(request.responseText); }.bind(this);

  var url = "/qvs/long_qpe_accum/fetch_long_qpe_accum_value_from_annual_sawtooth.php?prod=" + this.prod + "&start_yr=" + 
        start_time.year + "&start_mo=" + start_time.month + "&start_dy=" + start_time.day + "&start_hr=" + start_time.hour + 
        "&start_min=" + start_time.minute + "&end_yr=" + end_time.year + "&end_mo=" + end_time.month + "&end_dy=" + end_time.day + 
        "&end_hr=" + end_time.hour + "&end_min=" + end_time.minute + "&lon=" + this.cursor_lon + "&lat=" + this.cursor_lat;
  request.open("get",url,true);
  request.send();
  };

readoutLayer.prototype.parseReadoutReq = function(value_json_str) {
  this.readout_value = JSON.parse(value_json_str);
  this.drawReadoutValue();
  };

readoutLayer.prototype.drawReadoutValue = function() {
  if(this.clear_readout==0)  {
    if(this.readout_value.qvs_value<0)   document.getElementById(this.readout_user_id).innerHTML = "No Data";
    else {
      document.getElementById(this.readout_user_id).innerHTML = this.readout_value.qvs_value.toFixed(2) + " " + this.readout_value.qvs_units;
      }
    }
  else  {
    document.getElementById(this.readout_user_id).innerHTML = " ";
    }
  };

