//############################################################################
//       extract_monthly_zdr_error_median_histo.cc
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
//#include "library_read_placelist.h"

#define MAX_PATH_FILENAME_LENGTH  128
#define DEBUG 0

using namespace std;

int DaysInMonth(int year,int month);
float GetMedianOfIntArrayAboveThreshold(int *array,int num_pts,int threshold);

int main(int argc, char *argv[])
  {
  int year,month;
  char output_mode[12];

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
    cout<<"extract_monthly_zdr_error_median_histo year month output_mode(text or json)"<<endl<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Read in command line args.  No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  sscanf(argv[1],"%d",&year);
  sscanf(argv[2],"%d",&month);
  strcpy(output_mode,argv[3]);

//--------------------------------------------------------------------------------------
//     Read Radar List
//--------------------------------------------------------------------------------------

//  PlaceList *pt;
//  pt = LoadPlaceListFromFile("/web_data/qvs/ref_data/radar_lists/US_DP_Radar_List.txt");

//--------------------------------------------------------------------------------------
//    Open up the RSL file
//--------------------------------------------------------------------------------------

  char in_file[MAX_PATH_FILENAME_LENGTH];
  sprintf(in_file,"/web_data/qvs_data/level3/%d/%02d/RSL_Metadata.%d%02d.gz",year,month,year,month);

  PlaceBinaryArray *dat;
  if( (dat = CreatePlaceBinaryArrayFromFile(in_file))==NULL ) {
//    cout<<"Fatal error opening: "<<in_file<<endl;
    printf("{\"count\":0}");
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Set up array for all radars (3 parameters each, then 1 avg/count pair and a 33 histo bins
//--------------------------------------------------------------------------------------

  int ***histo_bins;
  histo_bins = new int** [dat->number_list];
  for(int i=0;i<dat->number_list;++i)  {
    histo_bins[i] = new int* [3];
    for(int j=0;j<3;++j) {
      histo_bins[i][j] = new int [33];
      for(int k=0;k<33;++k) {
        histo_bins[i][j][k] = 0;
        }
      }
    }

  int **rain_err;
  rain_err = new int* [dat->number_list];
  for(int i=0;i<dat->number_list;++i)  {
    rain_err[i] = new int [288*DaysInMonth(year,month)];
    for(int j=0;j<288*DaysInMonth(year,month);++j) {
      rain_err[i][j] = -99;
      }
    }

  float *rain_err_median;
  rain_err_median = new float [dat->number_list];

  int **snow_err;
  snow_err = new int* [dat->number_list];
  for(int i=0;i<dat->number_list;++i)  {
    snow_err[i] = new int [288*DaysInMonth(year,month)];
    for(int j=0;j<288*DaysInMonth(year,month);++j) {
      snow_err[i][j] = -99;
      }
    }

  float *snow_err_median;
  snow_err_median = new float [dat->number_list];

  int **bragg_err;
  bragg_err = new int* [dat->number_list];
  for(int i=0;i<dat->number_list;++i)  {
    bragg_err[i] = new int [288*DaysInMonth(year,month)];
    for(int j=0;j<288*DaysInMonth(year,month);++j) {
      bragg_err[i][j] = -99;
      }
    }

  float *bragg_err_median;
  bragg_err_median = new float [dat->number_list];

//--------------------------------------------------------------------------------------
//   Parse through the whole month of RSL data and fill arrays (ignoring missing flags)
//--------------------------------------------------------------------------------------

  int total_offset;
  float value;

  int *var_offset;
  var_offset = new int [dat->number_variables];

  var_offset[0] = 0;
  for(int i=1;i<dat->number_variables;++i) {
    var_offset[i] = var_offset[i-1] + dat->number_list*dat->number_values[i-1];
    }

  for(int list_index=0;list_index<dat->number_list;++list_index) {
    for(int i=0;i<288*DaysInMonth(year,month);++i) {
      total_offset = var_offset[0] + dat->number_values[0]*list_index + i;
      rain_err[list_index][i] = (int)dat->data[total_offset];
      total_offset = var_offset[1] + dat->number_values[1]*list_index + i;
      snow_err[list_index][i] = (int)dat->data[total_offset];
      total_offset = var_offset[2] + dat->number_values[2]*list_index + i;
      bragg_err[list_index][i] = (int)dat->data[total_offset];
      }

    value = GetMedianOfIntArrayAboveThreshold(rain_err[list_index],288*DaysInMonth(year,month),-90);
    if(value<-90)  rain_err_median[list_index] = -99;
    else           rain_err_median[list_index] = value/(float)dat->variable_scales[0];
    value = GetMedianOfIntArrayAboveThreshold(snow_err[list_index],288*DaysInMonth(year,month),-90);
    if(value<-90)  snow_err_median[list_index] = -99;
    else           snow_err_median[list_index] = value/(float)dat->variable_scales[1];
    value = GetMedianOfIntArrayAboveThreshold(bragg_err[list_index],288*DaysInMonth(year,month),-90);
    if(value<-90)  bragg_err_median[list_index] = -99;
    else           bragg_err_median[list_index] = value/(float)dat->variable_scales[1];
    }

  int bin_index;

  for(int var_index=0;var_index<dat->number_variables;++var_index) {
    for(int list_index=0;list_index<dat->number_list;++list_index) {
      for(int i=0;i<DaysInMonth(year,month);++i) {
        for(int j=0;j<288;++j) {
          total_offset = var_offset[var_index] + dat->number_values[var_index]*list_index + i*288 + j;

          if(dat->data[total_offset]==-99)   continue;
          if(dat->data[total_offset]==-999)  continue;

          value = (float)dat->data[total_offset]/(float)dat->variable_scales[var_index];
 
          if(value <= -2.0625 || value >= 2.0625)  continue;

          bin_index = (int)(33*(value + 2.0625)/4.125);
          ++histo_bins[list_index][var_index][bin_index];
          }
        }
      }
    }

//--------------------------------------------------------------------------------------
//      Write out json
//--------------------------------------------------------------------------------------

  if(strcmp(output_mode,"json")==0) {
    printf("{");

    printf("\"count\":%d",dat->number_list);
    printf(",");

    printf("\"names\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("\"%s\"",dat->list_names[i]);
      if(i<dat->number_list-1)  printf(",");
      }
    printf("],");

    printf("\"lat\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("%6.4f",dat->list_lat[i]);
      if(i<dat->number_list-1)  printf(",");
      }
    printf("],");

    printf("\"lon\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("%6.4f",dat->list_lon[i]);
      if(i<dat->number_list-1)  printf(",");
      }
    printf("],");

    printf("\"median_rain_method\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("%4.2f",rain_err_median[i]);
      if(i<dat->number_list-1)  printf(",");
      }
    printf("],");

    printf("\"median_snow_method\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("%4.2f",snow_err_median[i]);
      if(i<dat->number_list-1)  printf(",");
      }
    printf("],");

    printf("\"median_bragg_method\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("%4.2f",bragg_err_median[i]);
      if(i<dat->number_list-1)  printf(",");
      }
    printf("],");

    printf("\"rain_bins\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("[");
      for(int j=0;j<33;++j) {
        printf("%d",histo_bins[i][0][j]);
        if(j<32)  printf(",");
        }
      if(i<dat->number_list-1)  printf("],");
      else                  printf("]");
      }
    printf("],");

    printf("\"snow_bins\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("[");
      for(int j=0;j<33;++j) {
        printf("%d",histo_bins[i][1][j]);
        if(j<32)  printf(",");
        }
      if(i<dat->number_list-1)  printf("],");
      else                      printf("]");
      }
    printf("],");

    printf("\"bragg_bins\":[");
    for(int i=0;i<dat->number_list;++i) {
      printf("[");
      for(int j=0;j<33;++j) {
        printf("%d",histo_bins[i][2][j]);
        if(j<32)  printf(",");
        }
      if(i<dat->number_list-1)  printf("],");
      else                  printf("]");
      }
    printf("]");
    printf("}");
    }

//--------------------------------------------------------------------------------------
//    Clean up memory
//--------------------------------------------------------------------------------------

  for(int i=0;i<dat->number_list;++i)  {
    delete [] rain_err[i];
    delete [] snow_err[i];
    delete [] bragg_err[i];
    }
  delete [] rain_err;
  delete [] snow_err;
  delete [] bragg_err;

  delete [] rain_err_median;
  delete [] snow_err_median;
  delete [] bragg_err_median;

  for(int i=0;i<dat->number_list;++i)  {
    for(int j=0;j<3;++j) {
      delete [] histo_bins[i][j];
      }
    delete [] histo_bins[i];
    }
  delete [] histo_bins;

  DestroyPlaceBinaryArray(dat);
//  DestroyPlaceList(pt);
  }

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------

int DaysInMonth(int year,int month) {
  int days_in_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

  if(year%4==0) ++days_in_month[1];
  return days_in_month[month-1];
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


