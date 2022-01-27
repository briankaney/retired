//######################################################################################
//       extract_rrct_ts_json.cc
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
  int end_hour,end_day,end_month,end_year,num_hours;
  int time_averaging,start_hour_block,end_hour_block;
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
    cout<<"  ./extract_rrct_ts_json data_type radar end_year end_month end_day end_hour num_hours time_averaging start_hour_block end_hour_block min_dbz max_dbz max_dbz_diff min_bin_count"<<endl<<endl;
    cout<<"Example:"<<endl;
    cout<<"  ./extract_rrct_ts_json Refl KTLX 2018 7 15 0 24 5 0 0 10 70 60 0"<<endl;
    cout<<"  ./extract_rrct_ts_json ZDR KTLX 2018 7 15 0 24 5 0 0 10 70 60 0"<<endl<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Read in command line args.
//    No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  strcpy(data_type,argv[1]);
  strcpy(radar,argv[2]);
  sscanf(argv[3],"%d",&end_year);
  sscanf(argv[4],"%d",&end_month);
  sscanf(argv[5],"%d",&end_day);
  sscanf(argv[6],"%d",&end_hour);
  sscanf(argv[7],"%d",&num_hours);
  sscanf(argv[8],"%d",&time_averaging);
  sscanf(argv[9],"%d",&start_hour_block);
  sscanf(argv[10],"%d",&end_hour_block);
  sscanf(argv[11],"%f",&min_dbz);
  sscanf(argv[12],"%f",&max_dbz);
  sscanf(argv[13],"%f",&max_dbz_diff);
  sscanf(argv[14],"%d",&min_bin_count);

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

  if(num_hours<1) {
    printf("{\"error_flag\":\"invalid_time_period\",");
    printf("\"radar\":\"%s\",",radar);
    printf("\"num_partners\":0}");
    return -1;
    }

  int start_hour,start_day,start_month,start_year;
  AddHoursToTimeStamp(end_hour,end_day,end_month,end_year,-1*num_hours,&start_hour,&start_day,&start_month,&start_year);

  int num_files = NumberMonthsInYYYYMMSequence(start_month,start_year,end_month,end_year);

  int file_year,file_month;
  char **files;
  files = new char* [num_files];
  for(int i=0;i<num_files;++i) {
    files[i] = new char [MAX_PATH_FILENAME_LEN];

    AddMonthsToYYYYMM(start_month,start_year,i,&file_month,&file_year);
    if(strcmp(data_type,"Refl")==0)  sprintf(files[i],"/web_data/qvs_data/rrct/%4d/%02d/RRCT_Refl.%d%02d.gz",file_year,file_month,file_year,file_month);
    if(strcmp(data_type,"ZDR")==0)   sprintf(files[i],"/web_data/qvs_data/rrct/%4d/%02d/RRCT_ZDR.%d%02d.gz",file_year,file_month,file_year,file_month);
    }

//--------------------------------------------------------------------------------------
//   Set up array for dbz value time series for each partner in each pair
//--------------------------------------------------------------------------------------

  int num_times = 12*(HoursSince1960(end_hour,end_day,end_month,end_year) - HoursSince1960(start_hour,start_day,start_month,start_year));

  float **dbz1,**dbz2;
  int **bin_count;
  dbz1 = new float* [number_partners];
  dbz2 = new float* [number_partners];
  bin_count = new int* [number_partners];
  for(int i=0;i<number_partners;++i) {
    dbz1[i] = new float [num_times];
    dbz2[i] = new float [num_times];
    bin_count[i] = new int [num_times];
    for(int j=0;j<num_times;++j) {
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
    if(f==0)            start_index = (start_day-1)*288 + start_hour*12;
    if(f==num_files-1)  end_index = (end_day-1)*288 + end_hour*12;

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

  int num_ts = 5*num_times/time_averaging;
  int num_per_ts = time_averaging/5;

  float **ts_dbz1,**ts_dbz2;
  int **ts_bin_count;
  ts_dbz1 = new float* [number_partners];
  ts_dbz2 = new float* [number_partners];
  ts_bin_count = new int* [number_partners];
  for(int i=0;i<number_partners;++i) {
    ts_dbz1[i] = new float [num_ts];
    ts_dbz2[i] = new float [num_ts];
    ts_bin_count[i] = new int [num_ts];
    for(int j=0;j<num_ts;++j) {
      ts_dbz1[i][j] = -99;
      ts_dbz2[i][j] = -99;
      ts_bin_count[i][j] = -99;
      }
    }

//--------------------------------------------------------------------------------------

  int bcnt,d1cnt,d2cnt,sum_bin_count;
  float sum_dbz1,sum_dbz2;

  for(int i=0;i<number_partners;++i) {
    for(int t=0;t<num_ts;++t) {
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
      if(bcnt>0)   ts_bin_count[i][t] = sum_bin_count;
      if(d1cnt>0)  ts_dbz1[i][t] = sum_dbz1/d1cnt;
      if(d2cnt>0)  ts_dbz2[i][t] = sum_dbz2/d2cnt;
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
  printf("\"num_times\":%d",num_times);
  printf(",");
  printf("\"num_ts\":%d",num_ts);
  printf(",");
  WriteJSONBlock_StringArray("partners",number_partners,partners);
  printf(",");
  WriteJSONBlock_1D_IntArray("order_flag",number_partners,order_flag);
  printf(",");
  WriteJSONBlock_2D_FloatArray("dbz1",number_partners,num_ts,ts_dbz1,1);
  printf(",");
  WriteJSONBlock_2D_FloatArray("dbz2",number_partners,num_ts,ts_dbz2,1);
  printf(",");
  WriteJSONBlock_2D_IntArray("bin_count",number_partners,num_ts,ts_bin_count);
  printf("}");
  }


