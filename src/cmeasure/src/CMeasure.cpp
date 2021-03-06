#include "../include/Measurement.hpp"


extern double VSUPPLY;
extern double SHUNT_GAIN;
extern long SAMPLE_RATE;


/**
 * Parses command line commands
 * @argv cmd tokens
 * @argc cmd tokens array size
 * @measurement_type Measurement type to be set
 * @numberOfSample number of samples to be set
 * @outFile output file to be set
 */
void parseCmd (char* argv[], int argc, int& measurement_type, 
                       long& numberOfSamples, char* outFile) {
  bool expectS = false;
  bool displayMessage = false;

  //analyse parameters
  if (argc == 1) {
    // no parameter
    cout << endl << "No command line arguments provided. Using default options." << endl;
    cout << "Use -help for more informantion." << endl;
  } else {
     // toDo: check if i+1 is valid access
    for (int i  = 1; i < argc; i++) {
      // check for the measurement type flag
      if (strcmp(argv[i],"-o") == 0) {
        strcpy(outFile, argv[++i]);
      } else if ((strcmp(argv[i],"-h") == 0) || (strcmp(argv[i],"-help") == 0)) {
        displayMessage = true;
        break;

      } else if (strcmp(argv[i],"-AP") == 0) {
        measurement_type = TRIGGERED_ALL_POINTS;

      } else if (strcmp(argv[i],"-LV") == 0) {
        measurement_type = TRIGGERED_LAST_VALUE;
        expectS = true;

      } else if (strcmp(argv[i],"-TE") == 0) {
        measurement_type = TRIGGERED_TOTAL_ENERGY;

      } else if (strcmp(argv[i],"-AT") == 0) {
        measurement_type = TRIGGERED_AUTOMATIC;
        expectS = true;

      } else if (strcmp(argv[i],"-NT") == 0) {
        measurement_type = NO_TRIGGER;
        expectS = true;

      } else if (strcmp(argv[i],"-v") == 0) {
        VSUPPLY = atof(argv[++i]);

      } else if (strcmp(argv[i],"-sg") == 0) {
        SHUNT_GAIN = atof(argv[++i]);

      } else if (strcmp(argv[i],"-sr") == 0) {
        SAMPLE_RATE = atof(argv[++i]);
        if (SAMPLE_RATE > 48000) { SAMPLE_RATE = 48000;}

      } else if (strcmp(argv[i],"-s") == 0 && expectS) {
        numberOfSamples = atoi(argv[++i]);

      } else {
        cout << "Invalid Syntax with token: " << argv[i] << endl;
        displayMessage = true;
        break;
      }
    }
    

    if (displayMessage) {
      cout << R"(
        Usage: CMeasure.exe RunningType -s Samples -o Output <OPTIONS>
        where:
           * RunningType is one of -AP -LV -NT -TE
           * Samples is maximum number of samples to be collected (Only for NT, LV, AT)
           * Output is the name for the output file           
        ----------------------------------------------------------------------
           -AP = Triggered by All Points (Continuous Sample Collection) (default)
           -LV = Triggered by Last Value
           -NT = No Trigger
           -TE = Triggered by Total Energy
           -AT = Triggered Automatically)" << endl << endl;

        cout << R"(
                    <OPTIONS> -v | -sg | -sr 
                           Use this to set the board operating voltage,
                           SHUNT GAIN of the resistor and sampling rate.
                           (ex: use -v 12 for 12 Volts)
                           (ex: use -sg 10 for a gain of 10))
                           (ex: use -sr 48000 for rate of 48KHz))" << endl;
      exit(1);
    }
  }

  cout << "=======================================================" << endl;
  cout << "Running CMeasure with configurations:" << endl;
  cout << "Measurement Type = " << Measurement::name(measurement_type) << endl;
  if (measurement_type == NO_TRIGGER || measurement_type == TRIGGERED_LAST_VALUE ||
      measurement_type == TRIGGERED_AUTOMATIC )
    cout << "Maximum Number of Samples = " << numberOfSamples << endl;
  cout << "Output File = " << outFile << endl;
  cout << "Target Device Voltage = " << VSUPPLY << endl;
  cout << "Shunt Gain = " << SHUNT_GAIN << endl;
  cout << "Raw Sample Rate = " << SAMPLE_RATE << endl;
  cout << "=======================================================" << endl;

}


int main (int argc, char* argv[]){
  //default values
  int codes_running = 33;
  int curr_code = 0;
  int measurement_type = TRIGGERED_ALL_POINTS;
  long numberOfSamples = 33;
  int result_view_mode = RESULT_VIEW_FILE;
  char output_file_name[MAX_FILE_NAME_SIZE] = "output\0";
  std::string channel = "canalPrincipal";

  parseCmd(argv, argc, measurement_type, numberOfSamples, output_file_name);
   
  //initialize device
  Measurement test("Dev1", SAMPLE_RATE, 60);
  test.addChannel("canalPrincipal","Dev1/ai0", 1.0);
  test.addChannel("canalTrigger","Dev1/ai1", 10.0, 1.0, false);
  // test.addChannel("canalHD","Dev1/ai2",1.0);
  // test.addChannel("canalnulo","Dev1/ai3",1.0);
           
  cout << " Measurement Started "<<endl;
  //start the measurement
	test.startMeasure(measurement_type ,numberOfSamples, result_view_mode, output_file_name, channel );
      
  cout << "Finished";

  return 0;
}
