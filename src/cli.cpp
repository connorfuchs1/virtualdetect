/** 
  VM detection library. Use to increase honeypot efficacy.
 */

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

#if ( defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER) )
    #define WINDOWS 1
#else
    #define WINDOWS 0
#endif

#if ( defined(__GNUC__) || defined(__linux__) )
    #include <unistd.h>
    #define LINUX 1
#else   
    #define LINUX 0
#endif


//=======================================Functions==============================================
/**
    Function for checking out system IO devices.
 */
int IOdevices(int OS){
    cout<<"=====Checking IO Devices====="<<endl;
    vector<string> vm_signatures = {"VMware", "VirtualBox", "QEMU", "Microsoft"};
    //Windows
    if(OS == 1)
    {

    }
    //Linux
    else if(OS == 2)
    {
        ifstream io_device_file("/proc/bus/input/devices");
        if(io_device_file.is_open()) 
        {
            string line;
            while ( getline(io_device_file, line) )
            {
                for (const auto& signature : vm_signatures)
                {
                    if(line.find(signature) != string::npos)
                    {
                        cout << "Detected VM Vendor in IO devices: " << line << endl;
                    }
                }
            }
        }

        return 1;
    }
    //Mac
    else if (OS ==3)
    {

    }
    return 0;
}

/**
    Hub Function for calling all of our tests
 */
int runTests(int OS)
{
    int pass = 0;
    if(IOdevices(OS) == -1)
    {
        return -1;
    }
    else{
        pass++;
    }
    
    return pass;
}
/**
    Main
 */
int main(int argc, char* argv[]) 
{
    int OS, result, numTests = 1;
    if(WINDOWS) 
    {
        cout << "Running Windows" << endl;
        OS = 1;
    }
    else if(LINUX)
    {
        cout << "Running Linux" << endl;
        OS = 2;
    }
    else 
    {
        cout << "Unknown OS" << endl;
        OS = -1;
    }


    result = runTests(OS);

    cout<<"\n\t   ==============================================" << endl;
    cout << "\tResult: " << result << " of " << numTests << " tests found virtualization artifacts" << endl;
    cout << "\n\n" << argc << endl;
    cout << argv << endl;
    return 0;
}

