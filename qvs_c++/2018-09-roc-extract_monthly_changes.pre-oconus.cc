//############################################################################
//       extract_monthly_changes.cc
//
//       by Brian Kaney
//       modified Sept 2014
//############################################################################

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <zlib.h>

#include "../../c++_library/binary_array/library_place_binary_array.h"

#define MAX_PATH_FILENAME_LEN  256
#define DEBUG 0

using namespace std;

int DaysInMonth(int year,int month);

int main(int argc, char *argv[])
  {
  int year,month;

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
    cout<<"extract_monthly_changes year month"<<endl<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Read in command line args.
//    No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  sscanf(argv[1],"%d",&year);
  sscanf(argv[2],"%d",&month);

//--------------------------------------------------------------------------------------
//    Read placelist file
//--------------------------------------------------------------------------------------

//  char radar_list[MAX_PATH_FILENAME_LEN];
//  strcpy(radar_list,"/web_data/qvs/ref_data/radar_lists/US_DP_Radar_List.txt");

//  PlaceList *pt;
//  pt = LoadPlaceListFromFile_WithLatLonTest(radar_list,55,-130,20,-60);

//--------------------------------------------------------------------------------------
//    Open up the DHR file and fill the info for the first 4 variables
//--------------------------------------------------------------------------------------

  char in_file[MAX_PATH_FILENAME_LEN];
  sprintf(in_file,"/web_data/qvs_data/level3/%d/%02d/DHR_Metadata.%d%02d.gz",year,month,year,month);

  PlaceBinaryArray *dat;
  if( (dat = CreatePlaceBinaryArrayFromFile(in_file))==NULL ) {
    printf("{\"count\":0}");
    return -1;
    }

//--------------------------------------------------------------------------------------
//   Set up array for counting daily changes
//--------------------------------------------------------------------------------------

  int ***change_count;
  change_count = new int** [dat->number_list];
  for(int i=0;i<dat->number_list;++i)  {
    change_count[i] = new int* [DaysInMonth(year,month)];
    for(int j=0;j<DaysInMonth(year,month);++j) {
      change_count[i][j] = new int [15];
      for(int k=0;k<15;++k) {
        change_count[i][j][k] = 0;
        }
      }
    }

//--------------------------------------------------------------------------------------
//   Parse through the whole month of DHR data and record changes (ignoring missing flags)
//--------------------------------------------------------------------------------------

  int total_offset;

  int *var_offset;
  var_offset = new int [dat->number_variables];

  var_offset[0] = 0;
  for(int i=1;i<dat->number_variables;++i) {
    var_offset[i] = var_offset[i-1] + dat->number_list*dat->number_values[i-1];
    }

  float last_value,value;

  int total_count=0;
  for(int var_index=0;var_index<dat->number_variables;++var_index) {
    for(int list_index=0;list_index<dat->number_list;++list_index) {
      last_value = -99;
      for(int i=0;i<DaysInMonth(year,month);++i) {
        for(int j=0;j<288;++j) {
          total_offset = var_offset[var_index] + dat->number_values[var_index]*list_index + i*288 + j;

          if(dat->data[total_offset]==-99)  continue;

          value = (float)dat->data[total_offset]/(float)dat->variable_scales[var_index];
 
          if(last_value!=-99 && value!=last_value) {
            ++total_count;
            ++change_count[list_index][i][var_index];
            }
          last_value = value;
          }
        }
      }
    }

  DestroyPlaceBinaryArray(dat);

//--------------------------------------------------------------------------------------
//    Open up the STA file and fill the info for the next 8 variables
//--------------------------------------------------------------------------------------

  sprintf(in_file,"/web_data/qvs_data/level3/%d/%02d/STA_Metadata.%d%02d.gz",year,month,year,month);

  if( (dat = CreatePlaceBinaryArrayFromFile(in_file))==NULL ) {
    printf("{\"count\":0}");
    return -1;
    }

//--------------------------------------------------------------------------------------
//   Parse through the whole month of STA data and record changes (ignoring missing flags)
//--------------------------------------------------------------------------------------

  delete [] var_offset;
  var_offset = new int [dat->number_variables];

  var_offset[0] = 0;
  for(int i=1;i<dat->number_variables;++i) {
    var_offset[i] = var_offset[i-1] + dat->number_list*dat->number_values[i-1];
    }

  total_count=0;
  for(int var_index=0;var_index<dat->number_variables;++var_index) {
    for(int list_index=0;list_index<dat->number_list;++list_index) {
      last_value = -99;
      for(int i=0;i<DaysInMonth(year,month);++i) {
        for(int j=0;j<288;++j) {
          total_offset = var_offset[var_index] + dat->number_values[var_index]*list_index + i*288 + j;

          if(dat->data[total_offset]==-99)  continue;

          value = (float)dat->data[total_offset]/(float)dat->variable_scales[var_index];
 
          if(last_value!=-99 && value!=last_value) {
            ++total_count;
            ++change_count[list_index][i][4+var_index];
            }
          last_value = value;
          }
        }
      }
    }

  DestroyPlaceBinaryArray(dat);

//--------------------------------------------------------------------------------------
//    Open up the RSL file and fill the info for the last 3 variables
//--------------------------------------------------------------------------------------

  sprintf(in_file,"/web_data/qvs_data/level3/%d/%02d/RSL_Metadata.%d%02d.gz",year,month,year,month);

  if( (dat = CreatePlaceBinaryArrayFromFile(in_file))==NULL ) {
    printf("{\"count\":0}");
    return -1;
    }

//--------------------------------------------------------------------------------------
//   Parse through the whole month of RSL data and record changes (ignoring missing flags)
//--------------------------------------------------------------------------------------

  delete [] var_offset;
  var_offset = new int [dat->number_variables];

  var_offset[0] = 0;
  for(int i=1;i<dat->number_variables;++i) {
    var_offset[i] = var_offset[i-1] + dat->number_list*dat->number_values[i-1];
    }

  total_count=0;
  for(int var_index=0;var_index<dat->number_variables;++var_index) {
    for(int list_index=0;list_index<dat->number_list;++list_index) {
      last_value = -99;
      for(int i=0;i<DaysInMonth(year,month);++i) {
        for(int j=0;j<288;++j) {
          total_offset = var_offset[var_index] + dat->number_values[var_index]*list_index + i*288 + j;

          if(dat->data[total_offset]==-99)   continue;

          value = (float)dat->data[total_offset]/(float)dat->variable_scales[var_index];
 
          if(last_value!=-99 && value!=last_value) {
            ++total_count;
            ++change_count[list_index][i][12+var_index];
            }
          last_value = value;
          }
        }
      }
    }


//--------------------------------------------------------------------------------------
//   Print out the daily change stats to js file
//--------------------------------------------------------------------------------------

  int num_days = DaysInMonth(year,month);
  int num_variables = 15;

  printf("{\"count\":%d,",dat->number_list);
  printf("\"names\":[");
  for(int i=0;i<dat->number_list;++i) {
    printf("\"%s\"",dat->list_names[i]);
    if(i<dat->number_list-1)  printf(",");
    else                      printf("]");
    }
  printf(",");

  printf("\"lat\":[");
  for(int i=0;i<dat->number_list;++i) {
    printf("%7.4f",dat->list_lat[i]);
    if(i<dat->number_list-1)  printf(",");
    else                      printf("]");
    }
  printf(",");

  printf("\"lon\":[");
  for(int i=0;i<dat->number_list;++i) {
    printf("%9.4f",dat->list_lon[i]);
    if(i<dat->number_list-1)  printf(",");
    else                      printf("]");
    }
  printf(",");

  printf("\"dc\":[");
  for(int i=0;i<dat->number_list;++i) {
    printf("[");
    for(int j=0;j<num_days;++j) {
      printf("[");
      for(int k=0;k<num_variables;++k) {
        printf("%d",change_count[i][j][k]);
        if(k<num_variables-1)  printf(",");
        else                   printf("]");
        }
      if(j<num_days-1)  printf(",");
      else              printf("]");
      }
    if(i<dat->number_list-1)  printf(",");
    else                      printf("]");
    }
  printf(",");

//--------------------------------------------------------------------------------------
//    Set up array for all 12 parameters for whole monthly change totals and sum up the dailys
//--------------------------------------------------------------------------------------

  int **change_monthly;
  change_monthly = new int* [dat->number_list];
  for(int i=0;i<dat->number_list;++i)  {
    change_monthly[i] = new int [15];
    for(int j=0;j<15;++j) {
      change_monthly[i][j] = 0;
      for(int k=0;k<DaysInMonth(year,month);++k) {
        change_monthly[i][j] = change_monthly[i][j] + change_count[i][k][j];
        }
      }
    }

//--------------------------------------------------------------------------------------
//   Print out the daily change stats to js file
//--------------------------------------------------------------------------------------

  printf("\"mc\":[");
  for(int i=0;i<dat->number_list;++i) {
    printf("[");
    for(int j=0;j<num_variables;++j) {
      printf("%d",change_monthly[i][j]);
      if(j<num_variables-1)  printf(",");
      else                   printf("]");
      }
    if(i<dat->number_list-1)  printf(",");
    else                      printf("]");
    }
  printf("}");

//--------------------------------------------------------------------------------------

  DestroyPlaceBinaryArray(dat);
  }

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------

int DaysInMonth(int year,int month) {
  int days_in_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

  if(year%4==0) ++days_in_month[1];
  return days_in_month[month-1];
  }

//--------------------------------------------------------------------------------------

