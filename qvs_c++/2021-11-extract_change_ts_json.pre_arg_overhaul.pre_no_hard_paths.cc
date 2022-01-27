//######################################################################################
//       extract_change_ts_json.cc
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
  int time_before_averaging,time_after_averaging,start_hour_block,end_hour_block;
  float min_dbz,max_dbz,max_dbz_diff;
  int min_bin_count;
  char radar[8];

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
    cout<<"  ./extract_change_ts_json data_type radar before_end_year before_end_month before_end_day before_end_hour before_num_hours after_end_year after_end_month after_end_day after_end_hour after_num_hours time_before_averaging time_after_averaging start_hour_block end_hour_block min_dbz max_dbz max_dbz_diff min_bin_count"<<endl<<endl;
    cout<<"Example:"<<endl;
    cout<<"  ./extract_change_ts_json Refl KTLX 2018 7 15 0 24 2018 7 16 0 24 5 5 0 0 10 70 60 0"<<endl;
    cout<<"  ./extract_change_ts_json ZDR KTLX 2018 7 15 0 24 2018 7 16 0 24 5 5 0 0 10 70 60 0"<<endl<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Read in command line args.
//    No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  strcpy(data_type,argv[1]);
  strcpy(radar,argv[2]);
  sscanf(argv[3],"%d",&before_end_year);
  sscanf(argv[4],"%d",&before_end_month);
  sscanf(argv[5],"%d",&before_end_day);
  sscanf(argv[6],"%d",&before_end_hour);
  sscanf(argv[7],"%d",&before_num_hours);
  sscanf(argv[8],"%d",&after_end_year);
  sscanf(argv[9],"%d",&after_end_month);
  sscanf(argv[10],"%d",&after_end_day);
  sscanf(argv[11],"%d",&after_end_hour);
  sscanf(argv[12],"%d",&after_num_hours);
  sscanf(argv[13],"%d",&time_before_averaging);
  sscanf(argv[14],"%d",&time_after_averaging);
  sscanf(argv[15],"%d",&start_hour_block);
  sscanf(argv[16],"%d",&end_hour_block);
  sscanf(argv[17],"%f",&min_dbz);
  sscanf(argv[18],"%f",&max_dbz);
  sscanf(argv[19],"%f",&max_dbz_diff);
  sscanf(argv[20],"%d",&min_bin_count);

//--------------------------------------------------------------------------------------
//    Read placelist file
//--------------------------------------------------------------------------------------

  char radar_pairs[MAX_PATH_FILENAME_LEN];
  strcpy(radar_pairs,"/web_data/qvs/ref_data/rrct/RRCT_Pair_Indecies.txt");

//--------------------------------------------------------------------------------------
//KABR_KBIS 0 11
//WVY_KGYX 153 63
//XDR_XWL 158 173
//--------------------------------------------------------------------------------------

  FILE *inf;
  int read_index1,read_index2;
  char buffer[64],dummy[32],radar1[8],radar2[8];

  if( (inf=fopen(radar_pairs,"r"))==NULL ) {
    printf("{\"error_flag\":\"file_not_found\",");
    printf("\"radar\":\"%s\",",radar);
    printf("\"num_partners\":0}");
    return -1;
    }

  int number_partners = 0;
  while(fgets(buffer,62,inf)!=NULL)  { 
    sscanf(buffer,"%s",dummy);
    if(dummy[3]==95)  dummy[3]=32;  //--change '_' to ' '.
    if(dummy[4]==95)  dummy[4]=32;
    sscanf(dummy,"%s %s",radar1,radar2);

    if(strcmp(radar1,radar)==0 || strcmp(radar2,radar)==0)  ++number_partners;
    }

  int *pair_index,*order_flag;
  char **partners;

  pair_index = new int [number_partners];
  order_flag = new int [number_partners];
  partners = new char* [number_partners];
  for(int i=0;i<number_partners;++i)  partners[i] = new char [8];

  if(number_partners==0) {
    printf("{\"error_flag\":\"radar_not_found\",");
    printf("\"radar\":\"%s\",",radar);
    printf("\"num_partners\":0}");
    return -1;
    }

  rewind(inf);
  int a=0;
  int b=0;
  while(fgets(buffer,62,inf)!=NULL)  { 
    sscanf(buffer,"%s %d %d",dummy,&read_index1,&read_index2);
    if(dummy[3]==95)  dummy[3]=32;  //--change '_' to ' '.
    if(dummy[4]==95)  dummy[4]=32;
    sscanf(dummy,"%s %s",radar1,radar2);

    if(strcmp(radar1,radar)==0) {
      pair_index[b] = a;
      order_flag[b] = 0;
      strcpy(partners[b],radar2);
      ++b;
      }
    if(strcmp(radar2,radar)==0) {
      pair_index[b] = a;
      order_flag[b] = 1;
      strcpy(partners[b],radar1);
      ++b;
      }
    ++a;
    }

  fclose(inf);

//--------------------------------------------------------------------------------------
//   Figure out how many and which files will be needed for what can be a long time series
//--------------------------------------------------------------------------------------

  if(before_num_hours<1 || after_num_hours<1) {
    printf("{\"error_flag\":\"invalid_time_period\",");
    printf("\"radar\":\"%s\",",radar);
    printf("\"num_partners\":0}");
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
//   Set up array for dbz value time series for each partner in each pair
//--------------------------------------------------------------------------------------

  int before_num_times = 12*(HoursSince1960(before_end_hour,before_end_day,before_end_month,before_end_year) - 
                             HoursSince1960(before_start_hour,before_start_day,before_start_month,before_start_year));

  float **dbz1,**dbz2;
  int **bin_count;
  dbz1 = new float* [number_partners];
  dbz2 = new float* [number_partners];
  bin_count = new int* [number_partners];
  for(int i=0;i<number_partners;++i) {
    dbz1[i] = new float [before_num_times];
    dbz2[i] = new float [before_num_times];
    bin_count[i] = new int [before_num_times];
    for(int j=0;j<before_num_times;++j) {
      dbz1[i][j] = -99;
      dbz2[i][j] = -99;
      bin_count[i][j] = -99;
      }
    }

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------

  PlaceBinaryArray *dat;

  int t=0;
  for(int f=0;f<num_files;++f) {

    if( (dat = CreatePlaceBinaryArrayFromFile(files[f]))==NULL ) {
      printf("{\"error_flag\":\"data_files_missing\",");
      printf("\"radar\":\"%s\",",radar);
      printf("\"num_partners\":0}");
      return -1;
      }

    int start_index = 0;
    int end_index = dat->number_values[0];
    if(f==0)            start_index = (before_start_day-1)*288 + before_start_hour*12;
    if(f==num_files-1)  end_index = (before_end_day-1)*288 + before_end_hour*12;
//    if(f==0)            start_index = (start_day-1)*288 + start_hour*12;
//    if(f==num_files-1)  end_index = (end_day-1)*288 + end_hour*12;

    int *var_offset;
    var_offset = GetOffsetsToAllVariableBlocks(dat);

    int read_hr_of_day,read_bin_count;
    float read_dbz1,read_dbz2;

    for(int i=0;i<number_partners;++i) {
      for(int j=start_index;j<end_index;++j) {
        read_hr_of_day = (j/12)%24;
        if(start_hour_block>end_hour_block)  {
          if(read_hr_of_day>=start_hour_block || read_hr_of_day<end_hour_block)  continue;
          }
        if(end_hour_block>start_hour_block)  {
          if(read_hr_of_day>=start_hour_block && read_hr_of_day<end_hour_block)  continue;
          }

        read_dbz1 = ((float)(dat->data[var_offset[1] + dat->number_values[1]*pair_index[i] + j]))/dat->variable_scales[1];
        read_dbz2 = ((float)(dat->data[var_offset[2] + dat->number_values[2]*pair_index[i] + j]))/dat->variable_scales[2];
        read_bin_count = (int)(dat->data[var_offset[3] + dat->number_values[3]*pair_index[i] + j]);

        if(strcmp(data_type,"Refl")==0 && (read_dbz1<min_dbz || read_dbz2<min_dbz || read_dbz1>max_dbz || read_dbz2>max_dbz))  continue;
        if(strcmp(data_type,"Refl")==0 && abs(read_dbz1-read_dbz2) > max_dbz_diff)  continue;
        if(read_bin_count < min_bin_count)  continue;

        dbz1[i][t+j-start_index] = read_dbz1;
        dbz2[i][t+j-start_index] = read_dbz2;
        bin_count[i][t+j-start_index] = read_bin_count;
        }
      }
    t = t + end_index - start_index;

    DestroyPlaceBinaryArray(dat);
    }

//--------------------------------------------------------------------------------------

  int before_num_ts = 5*before_num_times/time_before_averaging;
  int num_per_ts = time_before_averaging/5;

  float **before_ts_dbz1,**before_ts_dbz2;
  int **before_ts_bin_count;
  before_ts_dbz1 = new float* [number_partners];
  before_ts_dbz2 = new float* [number_partners];
  before_ts_bin_count = new int* [number_partners];
  for(int i=0;i<number_partners;++i) {
    before_ts_dbz1[i] = new float [before_num_ts];
    before_ts_dbz2[i] = new float [before_num_ts];
    before_ts_bin_count[i] = new int [before_num_ts];
    for(int j=0;j<before_num_ts;++j) {
      before_ts_dbz1[i][j] = -99;
      before_ts_dbz2[i][j] = -99;
      before_ts_bin_count[i][j] = -99;
      }
    }

//--------------------------------------------------------------------------------------

  int bcnt,d1cnt,d2cnt,sum_bin_count;
  float sum_dbz1,sum_dbz2;

  for(int i=0;i<number_partners;++i) {
    for(int t=0;t<before_num_ts;++t) {
      sum_bin_count = 0;
      sum_dbz1 = 0;
      sum_dbz2 = 0;
      bcnt = 0;  d1cnt = 0;  d2cnt = 0;
      for(int j=0;j<num_per_ts;++j) {
        if(bin_count[i][t*num_per_ts+j]>-90) {  
          sum_bin_count = sum_bin_count + bin_count[i][t*num_per_ts+j];
          ++bcnt;
          }
        if(dbz1[i][t*num_per_ts+j]>-90) {  
          sum_dbz1 = sum_dbz1 + dbz1[i][t*num_per_ts+j];
          ++d1cnt;
          }
        if(dbz2[i][t*num_per_ts+j]>-90) {  
          sum_dbz2 = sum_dbz2 + dbz2[i][t*num_per_ts+j];
          ++d2cnt;
          }
        }
      if(bcnt>0)   before_ts_bin_count[i][t] = sum_bin_count;
      if(d1cnt>0)  before_ts_dbz1[i][t] = sum_dbz1/d1cnt;
      if(d2cnt>0)  before_ts_dbz2[i][t] = sum_dbz2/d2cnt;
      }
    }

//--------------------------------------------------------------------------------------

  for(int i=0;i<num_files;++i)  delete [] files[i];
  delete [] files;

  for(int i=0;i<number_partners;++i) {
    delete [] bin_count[i];
    delete [] dbz1[i];
    delete [] dbz2[i];
    }
  delete [] bin_count;
  delete [] dbz1;
  delete [] dbz2;

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//   Figure out how many and which files will be needed for what can be a long time series
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
//   Set up array for dbz value time series for each partner in each pair
//--------------------------------------------------------------------------------------

  int after_num_times = 12*(HoursSince1960(after_end_hour,after_end_day,after_end_month,after_end_year) - 
                            HoursSince1960(after_start_hour,after_start_day,after_start_month,after_start_year));

  dbz1 = new float* [number_partners];
  dbz2 = new float* [number_partners];
  bin_count = new int* [number_partners];
  for(int i=0;i<number_partners;++i) {
    dbz1[i] = new float [after_num_times];
    dbz2[i] = new float [after_num_times];
    bin_count[i] = new int [after_num_times];
    for(int j=0;j<after_num_times;++j) {
      dbz1[i][j] = -99;
      dbz2[i][j] = -99;
      bin_count[i][j] = -99;
      }
    }

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------

  t=0;
  for(int f=0;f<num_files;++f) {

    if( (dat = CreatePlaceBinaryArrayFromFile(files[f]))==NULL ) {
      printf("{\"error_flag\":\"data_files_missing\",");
      printf("\"radar\":\"%s\",",radar);
      printf("\"num_partners\":0}");
      return -1;
      }

    int start_index = 0;
    int end_index = dat->number_values[0];
    if(f==0)            start_index = (after_start_day-1)*288 + after_start_hour*12;
    if(f==num_files-1)  end_index = (after_end_day-1)*288 + after_end_hour*12;

    int *var_offset;
    var_offset = GetOffsetsToAllVariableBlocks(dat);

    int read_hr_of_day,read_bin_count;
    float read_dbz1,read_dbz2;

    for(int i=0;i<number_partners;++i) {
      for(int j=start_index;j<end_index;++j) {
        read_hr_of_day = (j/12)%24;
        if(start_hour_block>end_hour_block)  {
          if(read_hr_of_day>=start_hour_block || read_hr_of_day<end_hour_block)  continue;
          }
        if(end_hour_block>start_hour_block)  {
          if(read_hr_of_day>=start_hour_block && read_hr_of_day<end_hour_block)  continue;
          }

        read_dbz1 = ((float)(dat->data[var_offset[1] + dat->number_values[1]*pair_index[i] + j]))/dat->variable_scales[1];
        read_dbz2 = ((float)(dat->data[var_offset[2] + dat->number_values[2]*pair_index[i] + j]))/dat->variable_scales[2];
        read_bin_count = (int)(dat->data[var_offset[3] + dat->number_values[3]*pair_index[i] + j]);

        if(strcmp(data_type,"Refl")==0 && (read_dbz1<min_dbz || read_dbz2<min_dbz || read_dbz1>max_dbz || read_dbz2>max_dbz))  continue;
        if(strcmp(data_type,"Refl")==0 && abs(read_dbz1-read_dbz2) > max_dbz_diff)  continue;
        if(read_bin_count < min_bin_count)  continue;

        dbz1[i][t+j-start_index] = read_dbz1;
        dbz2[i][t+j-start_index] = read_dbz2;
        bin_count[i][t+j-start_index] = read_bin_count;
        }
      }
    t = t + end_index - start_index;

    DestroyPlaceBinaryArray(dat);
    }

//--------------------------------------------------------------------------------------

  int after_num_ts = 5*after_num_times/time_after_averaging;
  num_per_ts = time_after_averaging/5;

  float **after_ts_dbz1,**after_ts_dbz2;
  int **after_ts_bin_count;
  after_ts_dbz1 = new float* [number_partners];
  after_ts_dbz2 = new float* [number_partners];
  after_ts_bin_count = new int* [number_partners];
  for(int i=0;i<number_partners;++i) {
    after_ts_dbz1[i] = new float [after_num_ts];
    after_ts_dbz2[i] = new float [after_num_ts];
    after_ts_bin_count[i] = new int [after_num_ts];
    for(int j=0;j<after_num_ts;++j) {
      after_ts_dbz1[i][j] = -99;
      after_ts_dbz2[i][j] = -99;
      after_ts_bin_count[i][j] = -99;
      }
    }

//--------------------------------------------------------------------------------------

  for(int i=0;i<number_partners;++i) {
    for(int t=0;t<after_num_ts;++t) {
      sum_bin_count = 0;
      sum_dbz1 = 0;
      sum_dbz2 = 0;
      bcnt = 0;  d1cnt = 0;  d2cnt = 0;
      for(int j=0;j<num_per_ts;++j) {
        if(bin_count[i][t*num_per_ts+j]>-90) {  
          sum_bin_count = sum_bin_count + bin_count[i][t*num_per_ts+j];
          ++bcnt;
          }
        if(dbz1[i][t*num_per_ts+j]>-90) {  
          sum_dbz1 = sum_dbz1 + dbz1[i][t*num_per_ts+j];
          ++d1cnt;
          }
        if(dbz2[i][t*num_per_ts+j]>-90) {  
          sum_dbz2 = sum_dbz2 + dbz2[i][t*num_per_ts+j];
          ++d2cnt;
          }
        }
      if(bcnt>0)   after_ts_bin_count[i][t] = sum_bin_count;
      if(d1cnt>0)  after_ts_dbz1[i][t] = sum_dbz1/d1cnt;
      if(d2cnt>0)  after_ts_dbz2[i][t] = sum_dbz2/d2cnt;
      }
    }

//--------------------------------------------------------------------------------------
//   Print out outputs to json
//--------------------------------------------------------------------------------------

  printf("{");
  printf("\"error_flag\":\"none\"");
  printf(",");
  printf("\"radar\":\"%s\"",radar);
  printf(",");
  printf("\"num_partners\":%d",number_partners);
  printf(",");
  WriteJSONBlock_StringArray("partners",number_partners,partners);
  printf(",");
  WriteJSONBlock_1D_IntArray("order_flag",number_partners,order_flag);
  printf(",");
  printf("\"before_num_times\":%d",before_num_times);
  printf(",");
  printf("\"before_num_ts\":%d",before_num_ts);
  printf(",");
  WriteJSONBlock_2D_FloatArray("before_dbz1",number_partners,before_num_ts,before_ts_dbz1,1);
  printf(",");
  WriteJSONBlock_2D_FloatArray("before_dbz2",number_partners,before_num_ts,before_ts_dbz2,1);
  printf(",");
  WriteJSONBlock_2D_IntArray("before_bin_count",number_partners,before_num_ts,before_ts_bin_count);
  printf(",");
  printf("\"after_num_times\":%d",after_num_times);
  printf(",");
  printf("\"after_num_ts\":%d",after_num_ts);
  printf(",");
  WriteJSONBlock_2D_FloatArray("after_dbz1",number_partners,after_num_ts,after_ts_dbz1,1);
  printf(",");
  WriteJSONBlock_2D_FloatArray("after_dbz2",number_partners,after_num_ts,after_ts_dbz2,1);
  printf(",");
  WriteJSONBlock_2D_IntArray("after_bin_count",number_partners,after_num_ts,after_ts_bin_count);
  printf("}");
  }


