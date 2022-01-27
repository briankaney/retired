//############################################################################
//       extract_level3_metadata.cc
//
//       by Brian Kaney
//       modified April 2014
//############################################################################

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <zlib.h>

#include "../../c++_library/general/library_time.epoch1960.h"
#include "../../c++_library/general/library_read_text_place_list.h"

#define MAX_PATH_FILENAME_LEN  128
#define MAX_INPUT_LINE_LEN  128
#define DEBUG 0

using namespace std;

int main(int argc, char *argv[])
  {
  int year,month,day,hour,minute;

  char base_dir[64];
  strcpy(base_dir,"/qvs-storage2/VMRMS/");
  char ref_dir[64];
  strcpy(ref_dir,"/web_data/qvs/map_data/ll_points/");

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
    cout<<"extract_level3_metadata year month day hour minute"<<endl<<endl;
    return -1;
    }

//--------------------------------------------------------------------------------------
//    Read in command line args and a little processing (like read general config file).
//    No use of argc or argv beyond this block.
//--------------------------------------------------------------------------------------

  sscanf(argv[1],"%d",&year);
  sscanf(argv[2],"%d",&month);
  sscanf(argv[3],"%d",&day);
  sscanf(argv[4],"%d",&hour);
  sscanf(argv[5],"%d",&minute);

//--------------------------------------------------------------------------------------
//     Read Radar List
//--------------------------------------------------------------------------------------

  char radar_list[MAX_PATH_FILENAME_LEN];

  sprintf(radar_list,"%sMRMS_Level3_Radar_List.159.txt",ref_dir);

  PlaceList *pt;
  pt = LoadPlaceListFromFile(radar_list,LON_LAT,0);

  float *be,*mpr,*zmc,*zpc;
  float *cc,*dsc,*gc,*rhc,*wsc,*zzmc,*pcz,*pczdr;
  float *zerm,*zesm,*zebm;
  
  be  = new float [pt->num_list];
  mpr = new float [pt->num_list];
  zmc = new float [pt->num_list];
  zpc = new float [pt->num_list];

  cc    = new float [pt->num_list];
  dsc   = new float [pt->num_list];
  gc    = new float [pt->num_list];
  rhc   = new float [pt->num_list];
  wsc   = new float [pt->num_list];
  zzmc  = new float [pt->num_list];
  pcz   = new float [pt->num_list];
  pczdr = new float [pt->num_list];

  zerm  = new float [pt->num_list];
  zesm  = new float [pt->num_list];
  zebm  = new float [pt->num_list];

  for(int i=0;i<pt->num_list;++i) {
    be[i]  = -999;
    mpr[i] = -999;
    zmc[i] = -999;
    zpc[i] = -999;

    cc[i]    = -999;
    dsc[i]   = -999;
    gc[i]    = -999;
    rhc[i]   = -999;
    wsc[i]   = -999;
    zzmc[i]  = -999;
    pcz[i]   = -999;
    pczdr[i] = -999;

    zerm[i]  = -999;
    zesm[i]  = -999;
    zebm[i]  = -999;
    }

  int last_year,last_month,last_day,last_hour,last_minute;
  AddMinutesToTimeStamp(minute,hour,day,month,year,-5,&last_minute,&last_hour,&last_day,&last_month,&last_year);

//--------------------------------------------------------------------------------------
//     We will read both the DHR and STA metadata files.  These variables are common to both
//--------------------------------------------------------------------------------------

  char metadata_file[MAX_PATH_FILENAME_LEN];

  FILE *inf;
  char buffer[MAX_INPUT_LINE_LEN];

  char dummy[64],timestr[16],radar[5],min_str[3];
  int index,num_lines,min;
  float test;

//--------------------------------------------------------------------------------------
//     Open DHR metadata file and extract data fields
//--------------------------------------------------------------------------------------

  sprintf(metadata_file,"%s%d/%02d/radar/level3/ProductMetadata/DHR/AllRadar_DHR_00.00.%d%02d%02d.%02d0000.dat",
          base_dir,last_year,last_month,last_year,last_month,last_day,last_hour);
 
  if( (inf=fopen(metadata_file,"r"))==NULL ) {
    if(DEBUG==1)  cout<<"Failure opening input file: "<<endl;
    }
  else {
    while(fgets(buffer,MAX_INPUT_LINE_LEN-2,inf)!=NULL) {  
      sscanf(buffer,"%s %s %s %s %d",radar,dummy,dummy,timestr,&num_lines);

      min_str[0] = timestr[11];
      min_str[1] = timestr[12];
      min_str[2] = 0;
      sscanf(min_str,"%d",&min);
      if(min<last_minute)  {     //   Skip ahead if too early
        for(int i=0;i<num_lines;++i)  fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
        continue;
        }
      if(min>last_minute)  {     //   Done if too late
        break;
        }

      index = -1;
      for(int i=0;i<pt->num_list;++i) {   //  To get here, minutes must match.  Now find the radar index.
        if(strcmp(radar,pt->names[i])==0) {
          index = i;
          break;
          }
        }

      if(index<0) { cout<<"DHR-5 Radar "<<radar<<" not found"<<endl;   return -1; }

      for(int i=0;i<num_lines;++i) {
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        sscanf(buffer,"%s %f",dummy,&test);
        if(strcmp(dummy,"BiasEstimate")==0)       { be[index]  = test;  continue; }
        if(strcmp(dummy,"MaxPrecRateAllow")==0)   { mpr[index] = test;  continue; }
        if(strcmp(dummy,"ZRMultCoef")==0)         { zmc[index] = test;  continue; }
        if(strcmp(dummy,"ZRPowerCoef")==0)        { zpc[index] = test;  continue; }
        }
      fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
      } 
    fclose(inf);
    }

//--------------------------------------------------------------------------------------

  sprintf(metadata_file,"%s%d/%02d/radar/level3/ProductMetadata/DHR/AllRadar_DHR_00.00.%d%02d%02d.%02d0000.dat",
          base_dir,year,month,year,month,day,hour);
 
  if( (inf=fopen(metadata_file,"r"))==NULL ) {
    if(DEBUG==1)  cout<<"Failure opening input file: "<<endl;
    }

  else {
    while(fgets(buffer,MAX_INPUT_LINE_LEN-2,inf)!=NULL) {  
      sscanf(buffer,"%s %s %s %s %d",radar,dummy,dummy,timestr,&num_lines);

      min_str[0] = timestr[11];
      min_str[1] = timestr[12];
      min_str[2] = 0;
      sscanf(min_str,"%d",&min);
      if(min<minute)  {     //   Skip ahead if too early
        for(int i=0;i<num_lines;++i)  fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
        continue;
        }
      if(min>minute)  {     //   Done if too late
        break;
        }

      index = -1;
      for(int i=0;i<pt->num_list;++i) {   //  To get here, minutes must match.  Now find the radar index.
        if(strcmp(radar,pt->names[i])==0) {
          index = i;
          break;
          }
        }

      if(index<0) { cout<<"DHR Radar "<<radar<<" not found"<<endl;   return -1; }

      for(int i=0;i<num_lines;++i) {
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        sscanf(buffer,"%s %f",dummy,&test);
        if(strcmp(dummy,"BiasEstimate")==0)       { be[index]  = test;  continue; }
        if(strcmp(dummy,"MaxPrecRateAllow")==0)   { mpr[index] = test;  continue; }
        if(strcmp(dummy,"ZRMultCoef")==0)         { zmc[index] = test;  continue; }
        if(strcmp(dummy,"ZRPowerCoef")==0)        { zpc[index] = test;  continue; }
        }
      fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
      } 
    fclose(inf);
    }

//--------------------------------------------------------------------------------------
//     Open DHR metadata file and extract more data fields
//--------------------------------------------------------------------------------------

  sprintf(metadata_file,"%s%d/%02d/radar/level3/ProductMetadata/STA/AllRadar_STA_00.00.%d%02d%02d.%02d0000.dat",
          base_dir,last_year,last_month,last_year,last_month,last_day,last_hour);
 
  if( (inf=fopen(metadata_file,"r"))==NULL ) {
    if(DEBUG==1)  cout<<"Failure opening input file: "<<endl;
    }

  else {
    while(fgets(buffer,MAX_INPUT_LINE_LEN-2,inf)!=NULL) {  
      sscanf(buffer,"%s %s %s %s %d",radar,dummy,dummy,timestr,&num_lines);

      min_str[0] = timestr[11];
      min_str[1] = timestr[12];
      min_str[2] = 0;
      sscanf(min_str,"%d",&min);
      if(min<last_minute)  {     //   Skip ahead if too early
        for(int i=0;i<num_lines;++i)  fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
        continue;
        }
      if(min>last_minute)  {     //   Done if too late
        break;
        }

      index = -1;
      for(int i=0;i<pt->num_list;++i) {   //  To get here, minutes must match.  Now find the radar index.
        if(strcmp(radar,pt->names[i])==0) {
          index = i;
          break;
          }
        }

      if(index<0) { cout<<"STA-5 Radar "<<radar<<" not found"<<endl;   return -1; }

      for(int i=0;i<num_lines;++i) {
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        sscanf(buffer,"%s %f",dummy,&test);
        if(strcmp(dummy,"CrystalsMultCoeff")==0)     { cc[index]    = test;  continue; }
        if(strcmp(dummy,"DrySnowMultCoeff")==0)      { dsc[index]   = test;  continue; }
        if(strcmp(dummy,"GraupelMultCoeff")==0)      { gc[index]    = test;  continue; }
        if(strcmp(dummy,"RainHailMultCoeff")==0)     { rhc[index]   = test;  continue; }
        if(strcmp(dummy,"WetSnowMultCoeff")==0)      { wsc[index]   = test;  continue; }
        if(strcmp(dummy,"ZdrZMultiplierCoeff")==0)   { zzmc[index]  = test;  continue; }
        if(strcmp(dummy,"ZdrZPowerCoeffForZ")==0)    { pcz[index]   = test;  continue; }
        if(strcmp(dummy,"ZdrZPowerCoeffForZdr")==0)  { pczdr[index] = test;  continue; }
        }
      fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
      } 
    fclose(inf);
    }

//--------------------------------------------------------------------------------------

  sprintf(metadata_file,"%s%d/%02d/radar/level3/ProductMetadata/STA/AllRadar_STA_00.00.%d%02d%02d.%02d0000.dat",
          base_dir,year,month,year,month,day,hour);
 
  if( (inf=fopen(metadata_file,"r"))==NULL ) {
    if(DEBUG==1)  cout<<"Failure opening input file: "<<endl;
    }

  else  {
    while(fgets(buffer,MAX_INPUT_LINE_LEN-2,inf)!=NULL) {  
      sscanf(buffer,"%s %s %s %s %d",radar,dummy,dummy,timestr,&num_lines);

      min_str[0] = timestr[11];
      min_str[1] = timestr[12];
      min_str[2] = 0;
      sscanf(min_str,"%d",&min);
      if(min<minute)  {     //   Skip ahead if too early
        for(int i=0;i<num_lines;++i)  fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
        continue;
        }
      if(min>minute)  {     //   Done if too late
        break;
        }

      index = -1;
      for(int i=0;i<pt->num_list;++i) {   //  To get here, minutes must match.  Now find the radar index.
        if(strcmp(radar,pt->names[i])==0) {
          index = i;
          break;
          }
        }

      if(index<0) { cout<<"STA Radar "<<radar<<" not found"<<endl;   return -1; }

      for(int i=0;i<num_lines;++i) {
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        sscanf(buffer,"%s %f",dummy,&test);
        if(strcmp(dummy,"CrystalsMultCoeff")==0)     { cc[index]    = test;  continue; }
        if(strcmp(dummy,"DrySnowMultCoeff")==0)      { dsc[index]   = test;  continue; }
        if(strcmp(dummy,"GraupelMultCoeff")==0)      { gc[index]    = test;  continue; }
        if(strcmp(dummy,"RainHailMultCoeff")==0)     { rhc[index]   = test;  continue; }
        if(strcmp(dummy,"WetSnowMultCoeff")==0)      { wsc[index]   = test;  continue; }
        if(strcmp(dummy,"ZdrZMultiplierCoeff")==0)   { zzmc[index]  = test;  continue; }
        if(strcmp(dummy,"ZdrZPowerCoeffForZ")==0)    { pcz[index]   = test;  continue; }
        if(strcmp(dummy,"ZdrZPowerCoeffForZdr")==0)  { pczdr[index] = test;  continue; }
        }
      fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
      } 
    fclose(inf);
    }

//--------------------------------------------------------------------------------------
//     Open DHR metadata file and extract more data fields
//--------------------------------------------------------------------------------------

  sprintf(metadata_file,"%s%d/%02d/radar/level3/ProductMetadata/RSL/AllRadar_RSL_00.00.%d%02d%02d.%02d0000.dat",
          base_dir,last_year,last_month,last_year,last_month,last_day,last_hour);
 
  if( (inf=fopen(metadata_file,"r"))==NULL ) {
    if(DEBUG==1)  cout<<"Failure opening input file: "<<endl;
    }

  else  {
    while(fgets(buffer,MAX_INPUT_LINE_LEN-2,inf)!=NULL) {  
      sscanf(buffer,"%s %s %s %s %d",radar,dummy,dummy,timestr,&num_lines);

      min_str[0] = timestr[11];
      min_str[1] = timestr[12];
      min_str[2] = 0;
      sscanf(min_str,"%d",&min);
      if(min<last_minute)  {     //   Skip ahead if too early
        for(int i=0;i<num_lines;++i)  fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
        continue;
        }
      if(min>last_minute)  {     //   Done if too late
        break;
        }

      index = -1;
      for(int i=0;i<pt->num_list;++i) {   //  To get here, minutes must match.  Now find the radar index.
        if(strcmp(radar,pt->names[i])==0) {
          index = i;
          break;
          }
        }

      if(index<0) { cout<<"RSL-5 Radar "<<radar<<" not found"<<endl;   return -1; }

      for(int i=0;i<num_lines;++i) {
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        sscanf(buffer,"%s %f",dummy,&test);
        if(strcmp(dummy,"ZdrErrorRainMethod")==0)     { zerm[index] = test;  continue; }
        if(strcmp(dummy,"ZdrErrorDrySnowMethod")==0)  { zesm[index] = test;  continue; }
        if(strcmp(dummy,"ZdrErrorBraggMethod")==0)    { zebm[index] = test;  continue; }
        }
      fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
      } 
    fclose(inf);
    }

//--------------------------------------------------------------------------------------

  sprintf(metadata_file,"%s%d/%02d/radar/level3/ProductMetadata/RSL/AllRadar_RSL_00.00.%d%02d%02d.%02d0000.dat",
          base_dir,year,month,year,month,day,hour);
 
  if( (inf=fopen(metadata_file,"r"))==NULL ) {
    if(DEBUG==1)  cout<<"Failure opening input file: "<<endl;
    }

  else  {
    while(fgets(buffer,MAX_INPUT_LINE_LEN-2,inf)!=NULL) {  
      sscanf(buffer,"%s %s %s %s %d",radar,dummy,dummy,timestr,&num_lines);

      min_str[0] = timestr[11];
      min_str[1] = timestr[12];
      min_str[2] = 0;
      sscanf(min_str,"%d",&min);
      if(min<minute)  {     //   Skip ahead if too early
        for(int i=0;i<num_lines;++i)  fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
        continue;
        }
      if(min>minute)  {     //   Done if too late
        break;
        }

      index = -1;
      for(int i=0;i<pt->num_list;++i) {   //  To get here, minutes must match.  Now find the radar index.
        if(strcmp(radar,pt->names[i])==0) {
          index = i;
          break;
          }
        }

      if(index<0) { cout<<"RSL Radar "<<radar<<" not found"<<endl;   return -1; }

      for(int i=0;i<num_lines;++i) {
        fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);
        sscanf(buffer,"%s %f",dummy,&test);
        if(strcmp(dummy,"ZdrErrorRainMethod")==0)     { zerm[index] = test;  continue; }
        if(strcmp(dummy,"ZdrErrorDrySnowMethod")==0)  { zesm[index] = test;  continue; }
        if(strcmp(dummy,"ZdrErrorBraggMethod")==0)    { zebm[index] = test;  continue; }
        }
      fgets(buffer,MAX_INPUT_LINE_LEN-2,inf);   //--skip one blank line
      } 
    fclose(inf);
    }

//--------------------------------------------------------------------------------------
//    Print output
//--------------------------------------------------------------------------------------

  printf("{\"count\":%d,",pt->num_list);
  printf("\"names\":[");
  for(int i=0;i<pt->num_list;++i) {
    printf("\"%s\"",pt->names[i]);
    if(i<pt->num_list-1)  printf(",");
    else                  printf("],");
    }
  printf("\"lat\":[");
  for(int i=0;i<pt->num_list;++i) {
    printf("%7.4f",pt->lat[i]);
    if(i<pt->num_list-1)  printf(",");
    else                  printf("],");
    }
  printf("\"lon\":[");
  for(int i=0;i<pt->num_list;++i) {
    printf("%9.4f",pt->lon[i]);
    if(i<pt->num_list-1)  printf(",");
    else                  printf("],");
    }

  printf("\"mdata\":[");
  for(int i=0;i<pt->num_list;++i) {
    printf("[%5.3f,%4.2f,%4.2f,%4.2f,%5.3f,%4.2f,%4.2f,%4.2f,%4.2f,%4.3f,%4.3f,%4.2f,%4.2f,%4.2f,%4.2f]",be[i],mpr[i],zmc[i],zpc[i],cc[i],dsc[i],gc[i],rhc[i],wsc[i],zzmc[i],pcz[i],pczdr[i],zerm[i],zesm[i],zebm[i]);
    if(i<pt->num_list-1)  printf(",");
    else                  printf("]}");
    }

  }

