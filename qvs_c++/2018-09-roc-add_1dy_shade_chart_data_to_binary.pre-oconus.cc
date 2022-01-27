//############################################################################
//       extract_shade_chart_data.cc
//
//       by Brian Kaney
//       modified Oct 2015
//############################################################################

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <zlib.h>

#include "../../c++_library/binary_array/library_place_binary_array.h"

#define MAX_PATH_FILENAME_LENGTH  128
#define DEBUG 0

using namespace std;

int DaysInMonth(int year,int month);
int DayIndexWithinYear(int year,int month,int day);
float GetMedianOfIntArrayAboveThreshold(int *array,int num_pts,int threshold);

int main(int argc, char *argv[])
  {
  int year,month,day;

//--------------------------------------------------------------------------------------
//    If in debug mode, dump out all command line args.
//--------------------------------------------------------------------------------------

  if(DEBUG==1) {
    cout<<"Command Line: ";
    for(int i=0;i<argc;++i)  cout<<argv[i]<<" ";
    cout<<"<br>"<<endl;
    }

//--------------------------------------------------------------------------------------
//    Usage message if run with no command line args
//--------------------------------------------------------------------------------------

  if(argc<2) {
    cout<<endl<<endl;
    cout<<"extract_shade_chart_data year month day"<<endl<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Read in command line args.  No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  sscanf(argv[1],"%d",&year);
  sscanf(argv[2],"%d",&month);
  sscanf(argv[3],"%d",&day);

//--------------------------------------------------------------------------------------
//     Read Radar List
//--------------------------------------------------------------------------------------

//  PlaceList *pt;
//  pt = LoadPlaceListFromFile("/home/qpeverif/DrawCode/RefData/radar_lists/US_DP_Radar_List.txt");

  char in_file[MAX_PATH_FILENAME_LENGTH];
  sprintf(in_file,"/web_data/qvs_data/level3/%d/%02d/RSL_Metadata.%d%02d.gz",year,month,year,month);

  PlaceBinaryArray *in_dat;

  if( (in_dat = CreatePlaceBinaryArrayFromFile(in_file))==NULL ) {
    cout<<"Fatal error opening: "<<in_file<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Set up array for all radars
//--------------------------------------------------------------------------------------

  float *rain_median;
  float *snow_median;
  float *bragg_median;

  rain_median = new float [in_dat->number_list];
  snow_median = new float [in_dat->number_list];
  bragg_median = new float [in_dat->number_list];

  for(int i=0;i<in_dat->number_list;++i)  {
    rain_median[i] = -99;
    snow_median[i] = -99;
    bragg_median[i] = -99;
    }

//--------------------------------------------------------------------------------------
//    Set up input file read variables
//--------------------------------------------------------------------------------------

  int total_offset;
  float value;

  int num_variables = 3;
  int *var_offset;
  var_offset = new int [num_variables];

  int *holder;
  holder = new int [288];

//--------------------------------------------------------------------------------------
//   Loop thru input file, copy data sections, and find medians
//--------------------------------------------------------------------------------------

  var_offset[0] = 0;
  for(int i=1;i<in_dat->number_variables;++i) {
    var_offset[i] = var_offset[i-1] + in_dat->number_list*in_dat->number_values[i-1];
    }

  for(int list_index=0;list_index<in_dat->number_list;++list_index) {
    for(int i=0;i<288;++i)  {
      total_offset = var_offset[0] + in_dat->number_values[0]*list_index + (day-1)*288 + i;
      holder[i] = (int)in_dat->data[total_offset];
      }

    value = GetMedianOfIntArrayAboveThreshold(holder,288,-90);
    if(value<-90)  rain_median[list_index] = -99.0;
    else           rain_median[list_index] = (float)value/(float)in_dat->variable_scales[0];

    for(int i=0;i<288;++i)  {
      total_offset = var_offset[1] + in_dat->number_values[1]*list_index + (day-1)*288 + i;
      holder[i] = (int)in_dat->data[total_offset];
      }

    value = GetMedianOfIntArrayAboveThreshold(holder,288,-90);
    if(value<-90)  snow_median[list_index] = -99.0;
    else           snow_median[list_index] = (float)value/(float)in_dat->variable_scales[1];

    for(int i=0;i<288;++i)   {
      total_offset = var_offset[2] + in_dat->number_values[2]*list_index + (day-1)*288 + i;
      holder[i] = (int)in_dat->data[total_offset];
      }
    value = GetMedianOfIntArrayAboveThreshold(holder,288,-90);
    if(value<-90)  bragg_median[list_index] = -99.0;
    else           bragg_median[list_index] = (float)value/(float)in_dat->variable_scales[2];
    }

  DestroyPlaceBinaryArray(in_dat);

//--------------------------------------------------------------------------------------
//      Open output binary and write data
//--------------------------------------------------------------------------------------

  char out_file[MAX_PATH_FILENAME_LENGTH];
  sprintf(out_file,"/web_data/qvs_data/shade_chart/%d/ZDR_Error_ShadeChartData.%d.gz",year,year);

  PlaceBinaryArray *out_dat;
  if( (out_dat = CreatePlaceBinaryArrayFromFile(out_file))==NULL ) {
    cout<<"Fatal error opening: "<<out_file<<endl;
    return -1;
    }

  int output_day_offset = DayIndexWithinYear(year,month,day);

  var_offset[0] = 0;
  for(int i=1;i<out_dat->number_variables;++i) {
    var_offset[i] = var_offset[i-1] + out_dat->number_list*out_dat->number_values[i-1];
    }

  for(int i=0;i<out_dat->number_list;++i) {
    total_offset = var_offset[0] + out_dat->number_values[0]*i + output_day_offset;
    if(rain_median[i]<-90)  out_dat->data[total_offset] = -99;
    else                    out_dat->data[total_offset] = (int)(0.5+rain_median[i]*out_dat->variable_scales[0]);

    total_offset = var_offset[1] + out_dat->number_values[1]*i + output_day_offset;
    if(snow_median[i]<-90)  out_dat->data[total_offset] = -99;
    else                    out_dat->data[total_offset] = (int)(0.5+snow_median[i]*out_dat->variable_scales[1]);

    total_offset = var_offset[2] + out_dat->number_values[2]*i + output_day_offset;
    if(bragg_median[i]<-90)  out_dat->data[total_offset] = -99;
    else                     out_dat->data[total_offset] = (int)(0.5+bragg_median[i]*out_dat->variable_scales[2]);
    }

  WritePlaceBinaryArrayToFile(out_dat,out_file);

  DestroyPlaceBinaryArray(out_dat);

//--------------------------------------------------------------------------------------
//    Clean up memory
//--------------------------------------------------------------------------------------

  delete [] holder;

  delete [] rain_median;
  delete [] snow_median;
  delete [] bragg_median;
  }

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------

int DaysInMonth(int year,int month) {
  int days_in_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

  if(year%4==0) ++days_in_month[1];
  return days_in_month[month-1];
  }

int DayIndexWithinYear(int year,int month,int day) {
  int total_index = 0;

  for(int m=1;m<month;++m) {
    total_index = total_index + DaysInMonth(year,m);
    }
  total_index = total_index + (day-1);

  return total_index;
  }

//--------------------------------------------------------------------------------------

float GetMedianOfIntArrayAboveThreshold(int *array,int num_pts,int threshold) {
  if(num_pts<=0)  return -99;

  int count = 0;
  for(int n=0;n<num_pts;++n)  {
    if(array[n]>threshold)  ++count;
    }

  if(count==0)  return -99;

  int *work,i=0;
  work = new int [count];
  for(int n=0;n<num_pts;++n)  {
    if(array[n]>threshold)  {
      work[i] = array[n];
      ++i;
      }
    }

  if(count==1)  return (float)work[0];

  int temp,num_swaps = 999;
  while(num_swaps>0) {
    num_swaps = 0;
    for(int n=1;n<count;++n)  {
      if(work[n]<work[n-1]) {
        temp = work[n];
        work[n] = work[n-1];
        work[n-1] = temp;
        ++num_swaps;
        }
      }
    }

  float median = -99;

  if(count%2==1)  median = (float)(work[(int)((count-1)/2)]);
  if(count%2==0)  median = (float)((work[(int)(count/2)]+work[(int)(count/2-1)])/2);

  delete [] work;

  return median;
  }

//----------------------------------------------------------------------


