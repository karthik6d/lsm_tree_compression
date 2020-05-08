#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include "server.h"

using namespace std;

component sort(vector<component> components);
vector<subcomponent> mergeFiles(vector<string> input_files, int k);