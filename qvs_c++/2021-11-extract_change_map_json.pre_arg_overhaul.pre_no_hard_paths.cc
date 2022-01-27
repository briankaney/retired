//######################################################################################
//       extract_change_map_json.cc
//       by Brian T Kaney, 2018
//######################################################################################

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <zlib.h>

#include "../../c++_library/general/library_json_output.h"
#include "../../c++_library/general/library_time.epoch1960.h"
#include "../../c++_library/general/library_read_text_place_list.h"
#include "../../c++_library/binary_array/library_place_binary_array.h"

#define NUM_PAIRS 587
#define MAX_PATH_FILENAME_LEN  256
#define DEBUG 0

using namespace std;

int main(int argc, char *argv[])
  {
  char data_type[32];
  int before_end_hour,before_end_day,before_end_month,before_end_year,before_num_hours;
  int after_end_hour,after_end_day,after_end_month,after_end_year,after_num_hours;
  int start_hour_block,end_hour_block;
  float min_dbz,max_dbz,max_dbz_diff;
  int min_bin_count;

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
    cout<<"Usage:"<<endl;
    cout<<"  ./extract_change_map_json data_type before_end_year before_end_month before_end_day before_end_hour before_num_hours after_end_year after_end_month after_end_day after_end_hour after_num_hours start_hour_block end_hour_block min_dbz max_dbz max_dbz_diff min_bin_count"<<endl<<endl;
    cout<<"Example:"<<endl;
    cout<<"  ./extract_change_map_json Refl 2018 7 15 0 240 2018 7 5 0 240 0 0 10 70 60 0"<<endl;
    cout<<"  ./extract_change_map_json ZDR 2018 7 15 0 240 2018 7 5 0 240 0 0 10 70 60 0"<<endl<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Read in command line args.
//    No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  strcpy(data_type,argv[1]);
  sscanf(argv[2],"%d",&before_end_year);
  sscanf(argv[3],"%d",&before_end_month);
  sscanf(argv[4],"%d",&before_end_day);
  sscanf(argv[5],"%d",&before_end_hour);
  sscanf(argv[6],"%d",&before_num_hours);
  sscanf(argv[7],"%d",&after_end_year);
  sscanf(argv[8],"%d",&after_end_month);
  sscanf(argv[9],"%d",&after_end_day);
  sscanf(argv[10],"%d",&after_end_hour);
  sscanf(argv[11],"%d",&after_num_hours);
  sscanf(argv[12],"%d",&start_hour_block);
  sscanf(argv[13],"%d",&end_hour_block);
  sscanf(argv[14],"%f",&min_dbz);
  sscanf(argv[15],"%f",&max_dbz);
  sscanf(argv[16],"%f",&max_dbz_diff);
  sscanf(argv[17],"%d",&min_bin_count);


/*  changes to back propogate to rrct version.
 *  added memory clean up for dbz1, dbz2, and files.   */


//--------------------------------------------------------------------------------------
//    Read placelist file
//--------------------------------------------------------------------------------------

  char radar_list[MAX_PATH_FILENAME_LEN];
  strcpy(radar_list,"/web_data/qvs/ref_data/rrct/RRCT_Radar_List.txt");

  PlaceList *pt;
  pt = LoadPlaceListFromFile(radar_list,LON_LAT,0);

//--------------------------------------------------------------------------------------
//   Set up array for json output fields that need to be calculated from stored data.
//   Use of 'NUM_PAIRS' not ideal, weird mix of hard-coded values and values read from 
//   config list (pt->num_list).
//--------------------------------------------------------------------------------------

  float *before_pair_diff,*after_pair_diff;

  before_pair_diff = new float [NUM_PAIRS];   
  after_pair_diff  = new float [NUM_PAIRS];   
  for(int i=0;i<NUM_PAIRS;++i) {
    before_pair_diff[i] = -99;
    after_pair_diff[i] = -99;
    }

//--------------------------------------------------------------------------------------

  if(before_num_hours<1 || after_num_hours<1) {
    printf("{\"site_count\":0,\"pair_count\":0}");
    return -1;
    }

  int before_start_hour,before_start_day,before_start_month,before_start_year;
  AddHoursToTimeStamp(before_end_hour,before_end_day,before_end_month,before_end_year,
                      -1*before_num_hours,&before_start_hour,&before_start_day,&before_start_month,&before_start_year);

  int num_files = NumberMonthsInYYYYMMSequence(before_start_month,before_start_year,before_end_month,before_end_year);

  int file_year,file_month;
  char **files;
  files = new char* [num_files];
  for(int i=0;i<num_files;++i) {
    files[i] = new char [MAX_PATH_FILENAME_LEN];

    AddMonthsToYYYYMM(before_start_month,before_start_year,i,&file_month,&file_year);
    if(strcmp(data_type,"Refl")==0)  sprintf(files[i],"/web_data/qvs_data/rrct/%4d/%02d/RRCT_Refl.%d%02d.gz",file_year,file_month,file_year,file_month);
    if(strcmp(data_type,"ZDR")==0)   sprintf(files[i],"/web_data/qvs_data/rrct/%4d/%02d/RRCT_ZDR.%d%02d.gz",file_year,file_month,file_year,file_month);
    }

//--------------------------------------------------------------------------------------

  int num_times = 12*(HoursSince1960(before_end_hour,before_end_day,before_end_month,before_end_year) - 
                      HoursSince1960(before_start_hour,before_start_day,before_start_month,before_start_year));

  float **dbz1,**dbz2;
  dbz1 = new float* [NUM_PAIRS];
  dbz2 = new float* [NUM_PAIRS];
  for(int i=0;i<NUM_PAIRS;++i) {
    dbz1[i] = new float [num_times];
    dbz2[i] = new float [num_times];
    for(int j=0;j<num_times;++j) {
      dbz1[i][j] = -99;
      dbz2[i][j] = -99;
      }
    }

//--------------------------------------------------------------------------------------

  PlaceBinaryArray *dat;

  int t=0;
  for(int f=0;f<num_files;++f) {

    if( (dat = CreatePlaceBinaryArrayFromFile(files[f]))==NULL ) {
      printf("{\"site_count\":0,\"pair_count\":0}");
      return -1;
      }

    int start_index = 0;
    int end_index = dat->number_values[0];
    if(f==0)            start_index = (before_start_day-1)*288 + before_start_hour*12;
    if(f==num_files-1)  end_index = (before_end_day-1)*288 + before_end_hour*12;

    int *var_offset;
    var_offset = GetOffsetsToAllVariableBlocks(dat);

    int read_hr_of_day;
    float read_dbz1,read_dbz2;
    for(int i=0;i<dat->number_list;++i) {
      for(int j=start_index;j<end_index;++j) {
        read_hr_of_day = (j/12)%24;
        if(start_hour_block>end_hour_block)  {
          if(read_hr_of_day>=start_hour_block || read_hr_of_day<end_hour_block)  continue;
          }
        if(end_hour_block>start_hour_block)  {
          if(read_hr_of_day>=start_hour_block && read_hr_of_day<end_hour_block)  continue;
          }

        read_dbz1 = ((float)(dat->data[var_offset[1] + dat->number_values[1]*i + j]))/dat->variable_scales[1];
        read_dbz2 = ((float)(dat->data[var_offset[2] + dat->number_values[2]*i + j]))/dat->variable_scales[2];

        if(strcmp(data_type,"Refl")==0 && (read_dbz1<min_dbz || read_dbz2<min_dbz || read_dbz1>max_dbz || read_dbz2>max_dbz))  continue;
        if(strcmp(data_type,"Refl")==0 && abs(read_dbz1-read_dbz2) > max_dbz_diff)  continue;
        if(dat->data[var_offset[3] + dat->number_values[3]*i + j] < min_bin_count)  continue;

        dbz1[i][t+j-start_index] = read_dbz1;
        dbz2[i][t+j-start_index] = read_dbz2;
        }
      }
    t = t + end_index - start_index;

    DestroyPlaceBinaryArray(dat);
    }

  int count;
  float sum;
  for(int i=0;i<NUM_PAIRS;++i) {
    count = 0;
    sum = 0.0;
    for(int j=0;j<num_times;++j) {
      if(dbz1[i][j]<-90 || dbz2[i][j]<-90)  continue;
      sum = sum + dbz1[i][j] - dbz2[i][j];
      ++count;
      }
    if(count>0) before_pair_diff[i] = sum/(float)(count);
    }

//--------------------------------------------------------------------------------------

  for(int i=0;i<num_files;++i)  delete [] files[i];
  delete [] files;

  for(int i=0;i<NUM_PAIRS;++i) {
    delete [] dbz1[i];
    delete [] dbz2[i];
    }
  delete [] dbz1;
  delete [] dbz2;

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

  int after_start_hour,after_start_day,after_start_month,after_start_year;
  AddHoursToTimeStamp(after_end_hour,after_end_day,after_end_month,after_end_year,
                      -1*after_num_hours,&after_start_hour,&after_start_day,&after_start_month,&after_start_year);

  num_files = NumberMonthsInYYYYMMSequence(after_start_month,after_start_year,after_end_month,after_end_year);

  files = new char* [num_files];
  for(int i=0;i<num_files;++i) {
    files[i] = new char [MAX_PATH_FILENAME_LEN];

    AddMonthsToYYYYMM(after_start_month,after_start_year,i,&file_month,&file_year);
    if(strcmp(data_type,"Refl")==0)  sprintf(files[i],"/web_data/qvs_data/rrct/%4d/%02d/RRCT_Refl.%d%02d.gz",file_year,file_month,file_year,file_month);
    if(strcmp(data_type,"ZDR")==0)   sprintf(files[i],"/web_data/qvs_data/rrct/%4d/%02d/RRCT_ZDR.%d%02d.gz",file_year,file_month,file_year,file_month);
    }

//--------------------------------------------------------------------------------------

  num_times = 12*(HoursSince1960(after_end_hour,after_end_day,after_end_month,after_end_year) - 
                  HoursSince1960(after_start_hour,after_start_day,after_start_month,after_start_year));

  dbz1 = new float* [NUM_PAIRS];
  dbz2 = new float* [NUM_PAIRS];
  for(int i=0;i<NUM_PAIRS;++i) {
    dbz1[i] = new float [num_times];
    dbz2[i] = new float [num_times];
    for(int j=0;j<num_times;++j) {
      dbz1[i][j] = -99;
      dbz2[i][j] = -99;
      }
    }

//--------------------------------------------------------------------------------------

//  PlaceBinaryArray *dat;

  t=0;
  for(int f=0;f<num_files;++f) {

    if( (dat = CreatePlaceBinaryArrayFromFile(files[f]))==NULL ) {
      printf("{\"site_count\":0,\"pair_count\":0}");
      return -1;
      }

    int start_index = 0;
    int end_index = dat->number_values[0];
    if(f==0)            start_index = (after_start_day-1)*288 + after_start_hour*12;
    if(f==num_files-1)  end_index = (after_end_day-1)*288 + after_end_hour*12;

    int *var_offset;
    var_offset = GetOffsetsToAllVariableBlocks(dat);

    int read_hr_of_day;
    float read_dbz1,read_dbz2;
    for(int i=0;i<dat->number_list;++i) {
      for(int j=start_index;j<end_index;++j) {
        read_hr_of_day = (j/12)%24;
        if(start_hour_block>end_hour_block)  {
          if(read_hr_of_day>=start_hour_block || read_hr_of_day<end_hour_block)  continue;
          }
        if(end_hour_block>start_hour_block)  {
          if(read_hr_of_day>=start_hour_block && read_hr_of_day<end_hour_block)  continue;
          }

        read_dbz1 = ((float)(dat->data[var_offset[1] + dat->number_values[1]*i + j]))/dat->variable_scales[1];
        read_dbz2 = ((float)(dat->data[var_offset[2] + dat->number_values[2]*i + j]))/dat->variable_scales[2];

        if(strcmp(data_type,"Refl")==0 && (read_dbz1<min_dbz || read_dbz2<min_dbz || read_dbz1>max_dbz || read_dbz2>max_dbz))  continue;
        if(strcmp(data_type,"Refl")==0 && abs(read_dbz1-read_dbz2) > max_dbz_diff)  continue;
        if(dat->data[var_offset[3] + dat->number_values[3]*i + j] < min_bin_count)  continue;

        dbz1[i][t+j-start_index] = read_dbz1;
        dbz2[i][t+j-start_index] = read_dbz2;
        }
      }
    t = t + end_index - start_index;

    DestroyPlaceBinaryArray(dat);
    }

  for(int i=0;i<NUM_PAIRS;++i) {
    count = 0;
    sum = 0.0;
    for(int j=0;j<num_times;++j) {
      if(dbz1[i][j]<-90 || dbz2[i][j]<-90)  continue;
      sum = sum + dbz1[i][j] - dbz2[i][j];
      ++count;
      }
    if(count>0) after_pair_diff[i] = sum/(float)(count);
    }

//--------------------------------------------------------------------------------------
//   Print out outputs to json
//--------------------------------------------------------------------------------------

  printf("{");
  WriteJSONBlock_1D_FloatArray("before_pair_diff",NUM_PAIRS,before_pair_diff,1);
  printf(",");
  WriteJSONBlock_1D_FloatArray("after_pair_diff",NUM_PAIRS,after_pair_diff,1);
  printf("}");
  }

