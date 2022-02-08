//############################################################################
//       extract_data_histo_and_header_json.cc
//       by Brian T Kaney, [2017-2018]
//############################################################################

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cmath>
#include <zlib.h>

#include "../c++_library/hmrg_binary/library_read_hmrg_rect_binary.h"
#include "../c++_library/hmrg_binary/library_read_hmrg_polar_binary.h"
#include "../c++_library/general/library_json_output.h"
#include "../c++_library/general/library_stats_single_array.h"

#define MAX_PATH_FILENAME_LEN 128

using namespace std;

int main(int argc, char *argv[])
  {
  char filename[MAX_PATH_FILENAME_LEN];

  if(argc<2) {
    cout<<endl<<endl;
    cout<<"Usage:"<<endl<<endl;
    cout<<"  extract_data_histo_and_header_json path/filename";
    cout<<endl<<endl<<"Sample:"<<endl;
    cout<<"  ./extract_data_histo_and_header_json /qvs-storage/VMRMS/2018/01/CONUS/cref/CREF.20180112.120000.gz"<<endl;
    cout<<"  ./extract_data_histo_and_header_json /qvs-storage2/VMRMS/2018/01/radar/level3/KIND/DHR/hsr/DHR.20180112.120000.gz"<<endl<<endl;
    return -1;
    }

  strcpy(filename,argv[1]);

//----------------------------------------------------------------------------
//    Test for file geometry
//----------------------------------------------------------------------------

  int type = AutoDetectHMRGFileType(filename);

  if(type==NO_FILE) {
    cout<<"{\"status\":\"no_file\"}"<<endl;
    return -1;
    }
  if(type==FAIL) {
    cout<<"{\"status\":\"failed\"}"<<endl;
    return -1;
    }

//----------------------------------------------------------------------------
//    Open input data file
//----------------------------------------------------------------------------

  gzFile inzip;     
  inzip=gzopen(filename,"rb");

  HMRGRectBinaryHeader *rheader;
  HMRGPolarBinaryHeader *pheader;

//----------------------------------------------------------------------------
//    Rect case
//----------------------------------------------------------------------------

  if(type==RECT) {
    rheader = ReadHMRGRectBinaryHeaderFromFile(inzip);

    printf("{");
    printf("\"status\":\"rect\"");
    printf(",");
    printf("\"year\":%d,\"month\":%d,\"day\":%d,\"hour\":%d,\"minute\":%d,\"second\":%d",rheader->year,rheader->month,rheader->day,rheader->hour,rheader->minute,rheader->second);
    printf(",");
    printf("\"num_x\":%d,\"num_y\":%d,\"num_z\":%d",rheader->num_x,rheader->num_y,rheader->num_z);
    printf(",");
    printf("\"projection\":\"%s\",\"map_scale\":%d",rheader->projection,rheader->map_scale);
    printf(",");
    printf("\"trulat1_raw\":%d,\"trulat2_raw\":%d,\"trulon_raw\":%d",rheader->trulat1_raw,rheader->trulat2_raw,rheader->trulon_raw);
    printf(",");
    printf("\"nw_lon_raw\":%d,\"nw_lat_raw\":%d,\"xy_scale\":%d",rheader->nw_lon_raw,rheader->nw_lat_raw,rheader->xy_scale);
    printf(",");
    printf("\"dx_raw\":%d,\"dy_raw\":%d,\"dxy_scale\":%d",rheader->dx_raw,rheader->dy_raw,rheader->dxy_scale);
    printf(",");
    printf("\"z_scale\":%d",rheader->z_scale);
    printf(",");
    WriteJSONBlock_1D_IntArray("z_level_raw",rheader->num_z,rheader->z_level);
    printf(",");
    WriteJSONBlock_1D_IntArray("temp",10,rheader->temp);
    printf(",");
    printf("\"var_name\":\"%s\",\"var_units\":\"%s\",\"var_scale\":%d,\"missing_flag\":%d",rheader->var_name,rheader->var_units,rheader->var_scale,rheader->missing_flag);
    printf(",");
    printf("\"num_radars\":%d",rheader->num_radars);
    printf(",");
    WriteJSONBlock_StringArray("radar_names",rheader->num_radars,rheader->radars);
    }

//----------------------------------------------------------------------------
//    Polar case
//----------------------------------------------------------------------------

  if(type==POLAR) {
    pheader = ReadHMRGPolarBinaryHeaderFromFile(inzip);

    printf("{");
    printf("\"status\":\"polar\"");
    printf(",");
    printf("\"radar_name\":\"%s\",\"header_scale\":%d",pheader->radar_name,pheader->header_scale);
    printf(",");
    printf("\"raw_radarHgt\":%d",pheader->raw_radarHgt);
    printf(",");
    printf("\"radarHgt\":%3.1f,\"radarLat\":%6.4f,\"radarLon\":%6.4f",pheader->radarHgt,pheader->radarLat,pheader->radarLon);
    printf(",");
    printf("\"year\":%d,\"month\":%d,\"day\":%d,\"hour\":%d,\"minute\":%d,\"second\":%d",pheader->year,pheader->month,pheader->day,pheader->hour,pheader->minute,pheader->second);
    printf(",");
    printf("\"raw_nyqVel\":%d,\"nyqVel\":%4.2f",pheader->raw_nyqVel,pheader->nyqVel);
    printf(",");
    printf("\"vcpnum\":%d,\"tiltnum\":%d",pheader->vcpnum,pheader->tiltnum);
    printf(",");
    printf("\"raw_tiltElev\":%d",pheader->raw_tiltElev);
    printf(",");
    printf("\"tiltElev\":%4.2f",pheader->tiltElev);
    printf(",");
    printf("\"num_rays\":%d,\"num_gates\":%d",pheader->num_rays,pheader->num_gates);
    printf(",");
    printf("\"raw_start_edge_ray1\":%d,\"raw_ray_spacing\":%d",pheader->raw_start_edge_ray1,pheader->raw_ray_spacing);
    printf(",");
    printf("\"start_edge_ray1\":%4.2f,\"ray_spacing\":%4.2f",pheader->start_edge_ray1,pheader->ray_spacing);
    printf(",");
    printf("\"raw_start_edge_gate1\":%d,\"raw_gate_spacing\":%d",pheader->raw_start_edge_gate1,pheader->raw_gate_spacing);
    printf(",");
    printf("\"start_edge_gate1\":%4.2f,\"gate_spacing\":%4.2f",pheader->start_edge_gate1,pheader->gate_spacing);
    printf(",");
    printf("\"var_name\":\"%s\",\"var_units\":\"%s\",\"var_scale\":%d,\"missing_flag\":%d",pheader->var_name,pheader->var_units,pheader->var_scale,pheader->missing_flag);
    }

//----------------------------------------------------------------------------
//    Read contents of rest of file
//----------------------------------------------------------------------------

  short int *data;
  int data_count;

  if(type==RECT) {
    data = ReadHMRGRectBinaryShortIntArrayFromFile(inzip,rheader,0);
    data_count = rheader->num_x*rheader->num_y;
    DestroyHMRGRectBinaryHeader(rheader);
    }

  if(type==POLAR) {
    data = ReadHMRGPolarBinaryShortIntArrayFromFile(inzip,pheader);
    data_count = pheader->num_rays*pheader->num_gates;
    DestroyHMRGPolarBinaryHeader(pheader);
    }

  gzclose(inzip);

//----------------------------------------------------------------------------
//    Set up bins to count all occurrences of allowed data values
//----------------------------------------------------------------------------

  int *data_bins,num_bins = 65536;
  data_bins = new int [num_bins];

  for(int i=0;i<num_bins;++i)  data_bins[i] = 0;

  for(int i=0;i<data_count;++i) {
    ++data_bins[32768 + (int)data[i]];
    }

  delete [] data;

//----------------------------------------------------------------------------
//    Figure out how many bins have a non-zero count and then
//    set up two arrays, one the non-zero counts and one for 
//    indicies where they are found.
//----------------------------------------------------------------------------

  int diff_values_used = 65536 - CountValueInIntArray(data_bins,65536,0);

  int *used_bins,*bin_index;
  used_bins = new int [diff_values_used];
  bin_index = new int [diff_values_used];

  int j=0;
  for(int i=0;i<65536;++i) {
    if(data_bins[i]>0)  {
      used_bins[j] = data_bins[i];
      bin_index[j] = i;
      ++j;
      }
    }

//----------------------------------------------------------------------------
//    Output more json
//----------------------------------------------------------------------------

  printf(",");
  WriteJSONBlock_1D_IntArray("bin_index",diff_values_used,bin_index);
  printf(",");
  WriteJSONBlock_1D_IntArray("bin_count",diff_values_used,used_bins);
  printf("}");

  return 0;
  }


