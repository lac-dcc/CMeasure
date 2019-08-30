#include "stdafx.h"
#include "measurements.hpp"
#include <iostream>
#include <fstream>

//#define NO_TRIGGER 0
//#define TRIGGERED_ALL_POINTS 1
//#define TRIGGERED_LAST_VALUE 2
//#define TRIGGERED_TOTAL_ENERGY 3
//#define TRIGGERED_AUTOMATIC 4
//
//#define RESULT_VIEW_SCREEN 0
//#define RESULT_VIEW_FILE 1


/**--------SAMPLE RATE----------*/
//#define SAMPLE_RATE 10.0


int main(int argc, char* argv[]){

  //default values
  int codes_running = 33;
  int curr_code = 0;
  int measurement_type = TRIGGERED_LAST_VALUE;
  long numberOfSamples = 33;
  int result_view_mode = RESULT_VIEW_FILE;
  char output_file_name[MAX_FILE_NAME_SIZE];
  strcpy(output_file_name,"output");
  std::string channel = "canalPrincipal";


    //analyse parameters
    switch(argc)
    {
    //no parameter
    case(1): { break; }
    
    //type passed
    case(2): 
      {
        if(strcmp(argv[1],"-AP") == 0){measurement_type = TRIGGERED_ALL_POINTS; break;}
        else if(strcmp(argv[1],"-LV") == 0){measurement_type = TRIGGERED_LAST_VALUE; break;}
        else if(strcmp(argv[1],"-NT") == 0){measurement_type = NO_TRIGGER; break;}
        else if(strcmp(argv[1],"-TE") == 0){measurement_type = TRIGGERED_TOTAL_ENERGY; break;}
		else if(strcmp(argv[1],"-AT") == 0){measurement_type = TRIGGERED_AUTOMATIC; break;}
        else { printf("\nInvalid Paramenter\n Format: Measure.exe RunningType[-AP|-LV|-NT|-TE] [NumberofSamples] [output_filename]\n"); system("pause");  exit(1); }
      }
    case(3):
      {
        if(strcmp(argv[1],"-AP") == 0){measurement_type = TRIGGERED_ALL_POINTS; }
        else if(strcmp(argv[1],"-LV") == 0){measurement_type = TRIGGERED_LAST_VALUE; }
        else if(strcmp(argv[1],"-NT") == 0){measurement_type = NO_TRIGGER; }
        else if(strcmp(argv[1],"-TE") == 0){measurement_type = TRIGGERED_TOTAL_ENERGY; }
		else if(strcmp(argv[1],"-AT") == 0){measurement_type = TRIGGERED_AUTOMATIC; break;}
        else { printf("\nInvalid Paramenter\n Format: Measure.exe RunningType[-AP|-LV|-NT|-TE] [NumberofSamples] [output_filename]\n"); system("pause");  exit(1); }

        if(strcmp(argv[1],"-NT") == 0){numberOfSamples = atoi(argv[2]); break; }
        else if(strcmp(argv[1],"-LV") == 0){numberOfSamples = atoi(argv[2]); break;}
        else if(strcmp(argv[1],"-AT") == 0){numberOfSamples = atoi(argv[2]); break;}
      }

    case(4):
      {
        if(strcmp(argv[1],"-AP") == 0){measurement_type = TRIGGERED_ALL_POINTS; }
        else if(strcmp(argv[1],"-LV") == 0){measurement_type = TRIGGERED_LAST_VALUE; }
        else if(strcmp(argv[1],"-NT") == 0){measurement_type = NO_TRIGGER; }
        else if(strcmp(argv[1],"-TE") == 0){measurement_type = TRIGGERED_TOTAL_ENERGY; }
		else if(strcmp(argv[1],"-AT") == 0){measurement_type = TRIGGERED_AUTOMATIC; break;}
        else { printf("\nInvalid Paramenter\n Format: Measure.exe RunningType[-AP|-LV|-NT|-TE] [NumberofSamples] [output_filename]\n"); system("pause"); exit(1); }

        if(strcmp(argv[1],"-NT") == 0){numberOfSamples = atoi(argv[2]); }
        else if(strcmp(argv[1],"-LV") == 0){numberOfSamples = atoi(argv[2]); }
        else if(strcmp(argv[1],"-AT") == 0){numberOfSamples = atoi(argv[2]); }

        strcpy(output_file_name,argv[3]);
      }


    }


    //currCh->second->partial_HD_energy += (VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel); //sum energy

      //initialize device
      Measurement test("Dev1",SAMPLE_RATE,60);
      test.addChannel("canalPrincipal","Dev1/ai0",1.0);
      test.addChannel("canalTrigger","Dev1/ai1",10.0,1.0,false);
      test.addChannel("canalHD","Dev1/ai2",1.0);
      test.addChannel("canalnulo","Dev1/ai3",1.0);
           
      cout<<"Started"<<endl;

      //start the measure

	  //for(curr_code=0; curr_code<codes_running; curr_code++) 
		  test.startMeasure(measurement_type ,numberOfSamples, result_view_mode, output_file_name, channel );
      
      cout << "Finished";

      return 0;
}
