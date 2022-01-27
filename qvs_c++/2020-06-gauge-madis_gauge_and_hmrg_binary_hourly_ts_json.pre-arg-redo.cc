//######################################################################################
//       madis_gauge_and_hmrg_binary_hourly_ts_json.cc
//       by Brian T Kaney, 2020
//######################################################################################

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cmath>
#include <zlib.h>

#include "../c++_library/general/library_json_output.h"
#include "../c++_library/general/library_time.epoch1960.h"
#include "../c++_library/gauge_text/library_read_text_ts_gauge.h"
#include "../c++_library/hmrg_binary/library_basic_mapping.h"
#include "../c++_library/hmrg_binary/library_process_hmrg_binary.h"
#include "../c++_library/hmrg_binary/library_read_hmrg_rect_binary.h"
#include "../c++_library/hmrg_binary/library_read_hmrg_polar_binary.h"

#define MAX_PATH_FILENAME_LEN  128
#define DEBUG 0

using namespace std;

int main(int argc, char *argv[])
  {
  int end_year,end_month,end_day,end_hour,num_hrs;
  double nlat,wlon,slat,elon;

  char base_gauge_dir[MAX_PATH_FILENAME_LENGTH];
  char gauge_str[MAX_PATH_FILENAME_LENGTH];
  char base_path[MAX_PATH_FILENAME_LENGTH];
  char mid_path[MAX_PATH_FILENAME_LENGTH];
  char filename[MAX_PATH_FILENAME_LENGTH];

//--------------------------------------------------------------------------------------
//    If in debug mode, dump out all command line args.
//--------------------------------------------------------------------------------------

  if(DEBUG==1)
  {
    cout<<"Command Line:";
    for(int a=1;a<argc;++a) { cout<<" "<<argv[a]; }
    cout<<endl;
  }

//--------------------------------------------------------------------------------------
//    Usage message if run with no command line args
//--------------------------------------------------------------------------------------

  if(argc<2)
  {
    cout<<endl<<endl<<"Usage:"<<endl;
    cout<<"  madis_gauge_and_hmrg_binary_hourly_ts_json end_year end_month end_day end_hour num_hrs ";
    cout<<"wlon nlat elon slat base_gauge_dir gauge_str base_path mid_path";

    cout<<endl<<endl<<"Samples:"<<endl;
    cout<<"  ./madis_gauge_and_hmrg_binary_hourly_ts_json 2020 2 24 0 6 -95.25 36.25 -94.75 35.75 /localdata/archive/gauge_ts/";
    cout<<"conus/CONUS_ALLSETS_p2.48H /qvs-storage/VMRMS/ /CONUS/q3rad/1h_acc/1HRAD."<<endl;
    return -1;
  }

//--------------------------------------------------------------------------------------
//    Read in command line args.  No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  sscanf(argv[1],"%d",&end_year);
  sscanf(argv[2],"%d",&end_month);
  sscanf(argv[3],"%d",&end_day);
  sscanf(argv[4],"%d",&end_hour);
  sscanf(argv[5],"%d",&num_hrs);
  sscanf(argv[6],"%lf",&wlon);
  sscanf(argv[7],"%lf",&nlat);
  sscanf(argv[8],"%lf",&elon);
  sscanf(argv[9],"%lf",&slat);
  strcpy(base_gauge_dir,argv[10]);
  strcpy(gauge_str,argv[11]);
  strcpy(base_path,argv[12]);
  strcpy(mid_path,argv[13]);

  float conversion = 0.254;
  char *ptr;
  if( (ptr = strstr(mid_path,"PPS")) != NULL )  conversion = 0.01;

//--------------------------------------------------------------------------------------
//    The 'num_hrs' variable will not excede 24.  That way the whole time series period 
//    will fit within a single 48hr hourly ts gauge file.  No file crossing needed.  We
//    do need the time stamp for the file to read from.  If end_hour=0, then the file 
//    date is the same as the times series end time.  Any other end_hour and the 
//    file date must be the next calendar day.
//--------------------------------------------------------------------------------------

  int file_year,file_month,file_day;

  if(end_hour==0)
  {
    file_year  = end_year;
    file_month = end_month;
    file_day   = end_day;
  }
  else
  {
    AddDaysToTimeStamp(end_day,end_month,end_year,1,&file_day,&file_month,&file_year);
  }

//--------------------------------------------------------------------------------------
//    Read the whole 48 hr gauge ts file and then crop out the gauge object via lat/long test.
//--------------------------------------------------------------------------------------

  char gauge_file[MAX_PATH_FILENAME_LEN];
  sprintf(gauge_file,"%s%d/%02d/%s.%d%02d%02d.000000.ga.mtxt",base_gauge_dir,file_year,file_month,
          gauge_str,file_year,file_month,file_day);

  TSGauge *in_gauge;
  if( (in_gauge = LoadTSGaugeFrom48HrTextTS(gauge_file)) ==NULL) { cout<<"{\"status\":\"fail_no_gauge_file\",\"count\":0}";  return -1; }

  char qc_file[MAX_PATH_FILENAME_LEN];
  sprintf(qc_file,"%s%d/%02d/%s.%d%02d%02d.000000.qc.mtxt",base_gauge_dir,file_year,file_month,
          gauge_str,file_year,file_month,file_day);

  TSFlag *in_qc;
  if( (in_qc = LoadTSFlagFrom48HrTextTS(qc_file)) ==NULL) { cout<<"{\"status\":\"fail_no_qc_file\",\"count\":0}";  return -1; }

//--------------------------------------------------------------------------------------
//    Crop the gauge object via lat/long test.
//--------------------------------------------------------------------------------------

  TSGauge *crop_gauge;
  crop_gauge = CropTSGaugeViaLatLonBoundingBox(in_gauge,wlon,nlat,elon,slat);
  DestroyTSGauge(in_gauge);

  if(crop_gauge->num_list==0) { return -1; }

  TSFlag *crop_qc;
  crop_qc = CropTSFlagViaLatLonBoundingBox(in_qc,wlon,nlat,elon,slat);
  DestroyTSFlag(in_qc);

  if(crop_qc->num_list==0) { return -1; }

//--------------------------------------------------------------------------------------
//    Extract a time series subset of the whole 48 hr object
//--------------------------------------------------------------------------------------

  int first_index;

  if(end_hour==0) { first_index = 48 - num_hrs; }
  else { first_index = 48 - (24 - end_hour) - num_hrs; }

  TSGauge *out_gauge;
  out_gauge = TSSubsetOfTSGauge(crop_gauge,num_hrs,first_index);
  DestroyTSGauge(crop_gauge);

  TSFlag *out_qc;
  out_qc = TSSubsetOfTSFlag(crop_qc,num_hrs,first_index);
  DestroyTSFlag(crop_qc);

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------

  int *missing_cnt;
  missing_cnt = new int [out_gauge->num_list];

  for(int i=0;i<out_gauge->num_list;++i)
  {
    missing_cnt[i] = 0;
    for(int h=0;h<out_gauge->num_hours;++h)
    {
      if(out_gauge->precip_ts[i][h]<0) { ++missing_cnt[i]; }
    } 
  }

  int *hrs_bad;
  hrs_bad = new int [out_gauge->num_list];

  for(int i=0;i<out_gauge->num_list;++i)
  {
    hrs_bad[i] = 0;
    for(int h=0;h<out_gauge->num_hours;++h)
    {
      if(out_qc->qc_flag_ts[i][h]!=0) { ++hrs_bad[i]; }
    } 
  }

//----------------------------------------------------------------------------
//    Test for file geometry.  Set up arrays.
//----------------------------------------------------------------------------

  float **values;
  values = new float* [num_hrs];
  for(int i=0;i<num_hrs;++i)
  {
    values[i] = new float [out_gauge->num_list];
    for(int j=0;j<out_gauge->num_list;++j) { values[i][j] = -9999; }
  }

  int fyear,fmonth,fday,fhour;

  int type;
  for(int f=0;f<num_hrs;++f)
  {
    AddHoursToTimeStamp(end_hour,end_day,end_month,end_year,-1*(num_hrs-f-1),&fhour,&fday,&fmonth,&fyear);
    sprintf(filename,"%s%d/%02d%s%d%02d%02d.%02d0000.gz",base_path,fyear,fmonth,mid_path,fyear,fmonth,fday,fhour);

    type = AutoDetectHMRGFileType(filename);
    if(type>=0)  break;
  }

  for(int f=0;f<num_hrs;++f)
  {
    AddHoursToTimeStamp(end_hour,end_day,end_month,end_year,-1*(num_hrs-f-1),&fhour,&fday,&fmonth,&fyear);
    sprintf(filename,"%s%d/%02d%s%d%02d%02d.%02d0000.gz",base_path,fyear,fmonth,mid_path,fyear,fmonth,fday,fhour);

    if(type==RECT)
    {
      fill_values_at_float_locations_from_rect_hmrg_binary(filename,out_gauge->lon,out_gauge->lat,values[f],out_gauge->num_list);
    }

    if(type==POLAR)
    {
      fill_values_at_float_locations_from_polar_hmrg_binary(filename,out_gauge->lon,out_gauge->lat,values[f],out_gauge->num_list);
    }
  }

//----------------------------------------------------------------------------
//    Output processing for the json fields.
//----------------------------------------------------------------------------

  int **return_gauge;
  return_gauge = new int* [out_gauge->num_list];
  for(int i=0;i<out_gauge->num_list;++i) { return_gauge[i] = new int [num_hrs]; }

  for(int i=0;i<out_gauge->num_list;++i) {
    for(int h=0;h<out_gauge->num_hours;++h) {
      if(out_gauge->precip_ts[i][h]<0) { return_gauge[i][h]=-9; }
      else { return_gauge[i][h]=(int)(0.5+out_gauge->precip_ts[i][h]*100); }
      }
    }

  int **return_qpe;
  return_qpe = new int* [out_gauge->num_list];
  for(int i=0;i<out_gauge->num_list;++i) { return_qpe[i] = new int [num_hrs]; }

    //--Note index reversal in 'values' array below, we're converting to integer 
    //   increments of 0.01", but also taking the 'transpose' of 'values' at the same time

  for(int i=0;i<out_gauge->num_list;++i) {
    for(int h=0;h<out_gauge->num_hours;++h) {
      if(values[h][i]<0) { return_qpe[i][h]=-9; }
      else { return_qpe[i][h]=(int)(0.5+values[h][i]/conversion); }
      }
    }

//----------------------------------------------------------------------------
//    JSON output
//----------------------------------------------------------------------------

  printf("{");
  printf("\"status\":\"success\"");
  printf(",");
  printf("\"count\":%d",out_gauge->num_list);
  printf(",");
  WriteJSONBlock_StringArray("id",out_gauge->num_list,out_gauge->names);
  printf(",");
  WriteJSONBlock_1D_FloatArray("lon",out_gauge->num_list,out_gauge->lon,4);
  printf(",");
  WriteJSONBlock_1D_FloatArray("lat",out_gauge->num_list,out_gauge->lat,4);
  printf(",");
  WriteJSONBlock_1D_IntArray("bad_flag",out_gauge->num_list,hrs_bad);
  printf(",");
  WriteJSONBlock_1D_IntArray("missing",out_gauge->num_list,missing_cnt);
  printf(",");
  WriteJSONBlock_2D_IntArray("qc_ts",out_gauge->num_list,num_hrs,out_qc->qc_flag_ts);
  printf(",");
  WriteJSONBlock_1D_IntArray("min_shift",out_gauge->num_list,out_gauge->min_min_shift);
  printf(",");
  WriteJSONBlock_1D_IntArray("max_shift",out_gauge->num_list,out_gauge->max_min_shift);
  printf(",");
  WriteJSONBlock_2D_IntArray("precip_ts",out_gauge->num_list,num_hrs,return_gauge);
  printf(",");
  WriteJSONBlock_2D_IntArray("qpe_ts",out_gauge->num_list,num_hrs,return_qpe);
  printf("}");

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  for(int i=0;i<num_hrs;++i) { delete [] values[i]; }
  delete [] values;
  for(int i=0;i<out_gauge->num_list;++i) { delete [] return_qpe[i]; }
  delete [] return_qpe;
  for(int i=0;i<out_gauge->num_list;++i) { delete [] return_gauge[i]; }
  delete [] return_gauge;

  DestroyTSGauge(out_gauge);
  DestroyTSFlag(out_qc);

  return 0;
  } 

