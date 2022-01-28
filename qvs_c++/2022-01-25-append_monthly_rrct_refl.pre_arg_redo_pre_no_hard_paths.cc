//######################################################################################
//       append_monthly_rrct_refl.cc
//       by Brian T Kaney, 2018
//######################################################################################

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <zlib.h>

#include "../../c++_library/binary_array/library_place_binary_array.h"
#include "../../c++_library/general/library_time.epoch1960.h"

#define MAX_PATH_FILENAME_LEN  256
#define MAX_INPUT_LINE_LEN     256
#define DEBUG 0

using namespace std;

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
    cout<<"append_monthly_rrct_refl year month"<<endl<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Read in command line args and a little processing (like read general config file).
//    No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  sscanf(argv[1],"%d",&year);
  sscanf(argv[2],"%d",&month);

//--------------------------------------------------------------------------------------

  char bin_file[MAX_PATH_FILENAME_LEN];
  sprintf(bin_file,"/web_data/qvs_data/rrct/%d/%02d/RRCT_Refl.%d%02d.gz",year,month,year,month);

  PlaceBinaryArray *dat;
  dat = CreatePlaceBinaryArrayFromFile(bin_file);

//--------------------------------------------------------------------------------------

  int total_offset;
  int list_offset = 288*DaysInMonth(year,month);

  int *var_offset;
  var_offset = new int [dat->number_variables];

  var_offset[0] = 0;
  for(int i=1;i<dat->number_variables;++i) {   //  I think there is a lib function for this
    var_offset[i] = var_offset[i-1] + dat->number_list*dat->number_values[i-1];
    }

//--------------------------------------------------------------------------------------

  char rrct_refl_file[MAX_PATH_FILENAME_LEN];

  FILE *inf;
  char buffer[MAX_INPUT_LINE_LEN];

  char dummy[32],timestamp[64],substr[8];
  int read_day,read_hour,read_minute,read_min_diff,read_bin_count;
  float read_dbz1,read_dbz2;

//20180701.000000 1 22.46 23.14 27.17 26.68 3886
//20180701.000500 1 24.64 25.51 28.99 30.07 4243

  for(int i=0;i<dat->number_list;++i) {
    sprintf(rrct_refl_file,"/web_data/qvs_data/rrct/%d/%02d/dbz/%s_%d%02d_Refl_Comparison",year,month,dat->list_names[i],year,month);
 
    if( (inf=fopen(rrct_refl_file,"r"))==NULL ) {
      if(DEBUG==1)  cout<<"Failure opening input file: "<<rrct_refl_file<<endl;
      continue;
      }

    while(fgets(buffer,MAX_INPUT_LINE_LEN-2,inf)!=NULL) {  
      sscanf(buffer,"%s %d %s %s %f %f %d",timestamp,&read_min_diff,dummy,dummy,&read_dbz1,&read_dbz2,&read_bin_count);

      substr[0] = timestamp[6];
      substr[1] = timestamp[7];
      substr[2] = 0;
      sscanf(substr,"%d",&read_day);
      substr[0] = timestamp[9];
      substr[1] = timestamp[10];
      substr[2] = 0;
      sscanf(substr,"%d",&read_hour);
      substr[0] = timestamp[11];
      substr[1] = timestamp[12];
      substr[2] = 0;
      sscanf(substr,"%d",&read_minute);

      total_offset = var_offset[0] + list_offset*i + (read_day-1)*288 + read_hour*12 + read_minute/5;
      dat->data[total_offset] = (short int)(read_min_diff);

      total_offset = var_offset[1] + list_offset*i + (read_day-1)*288 + read_hour*12 + read_minute/5;
      dat->data[total_offset] = (short int)(read_dbz1*dat->variable_scales[1]);

      total_offset = var_offset[2] + list_offset*i + (read_day-1)*288 + read_hour*12 + read_minute/5;
      dat->data[total_offset] = (short int)(read_dbz2*dat->variable_scales[2]);

      total_offset = var_offset[3] + list_offset*i + (read_day-1)*288 + read_hour*12 + read_minute/5;
      dat->data[total_offset] = (short int)(read_bin_count);
      }

    fclose(inf);
    }

//--------------------------------------------------------------------------------------

  WritePlaceBinaryArrayToFile(dat,bin_file);

  DestroyPlaceBinaryArray(dat);
  }

