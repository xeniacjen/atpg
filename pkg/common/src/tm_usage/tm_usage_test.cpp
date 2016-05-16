/*
 * =====================================================================================
 *
 *       Filename:  tm_usage_test.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/11/2016 11:59:33 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include <iostream>

#include "tm_usage.h"

using namespace std; 
using namespace CommonNs; 

int main(int argc, char** argv) { 
    
    TmUsage tmusg; 

    cout << "Hello, World! \n"; 

    TmStat stat; 
    tmusg.getPeriodUsage(stat); 
    cout << "#  Finished executing program";
    cout << "    " << (double)stat.rTime / 1000000.0 << " s";
    cout << "    " << (double)stat.vmSize / 1024.0   << " MB" << endl;

    return 0; 
}
