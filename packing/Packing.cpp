/*
*   The MIT License (MIT)

*   Copyright (c) 2018 Wei Shi

*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:

*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.

*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*/
#include "src/BrutalPacking.h"
#include "src/argvparser.h"
#include "src/Log.h"
#include "src/Types.h"
#include <cstdlib>
#include <sstream>
#include <stdio.h>
#include <cstdio>
#include "src/rapidjson/filewritestream.h"
#include "src/rapidjson/writer.h"
#include <fstream>

using namespace CommandLineProcessing;
using namespace std;
using namespace rapidjson;

int main(int argc, char* argv[])
{
     // finally print a generated help page
    ArgvParser parser;
    parser.setIntroductoryDescription("Octagon Texture Packing Tool");
    parser.defineOption("version","Display Version", ArgvParser::NoOptionAttribute);
    parser.defineOption("f", "Polygon Json File", ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);
    parser.defineOption("size", "Atlas Image Max Size", ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);
    parser.defineOption("lp", "GPU loop limit default is 1000",ArgvParser::NoOptionAttribute | ArgvParser::OptionRequiresValue);
    parser.defineOption("debug", "debug flag",ArgvParser::NoOptionAttribute);

    
    auto parseResult = parser.parse(argc, argv);
    if(parser.foundOption("version")){
        cout << "Version 0.8.3" << endl;
        return true;
    }
    if(parseResult != ArgvParser::NoParserError){
        cout << endl << parser.usageDescription() << endl;
        return false;
    }


    if(parser.foundOption("f") && parser.foundOption("size")){
        string path = parser.optionValue("f");
        auto maxSize = atoi(parser.optionValue("size").c_str());

        LogInfo("Processing Polygon file: %s", path.c_str());
        LogInfo("Atlas max size is %d x %d", maxSize, maxSize);
        int loopLimit = 0;
        if(parser.foundOption("loopLimit")){
            loopLimit = atoi(parser.optionValue("loopLimit").c_str());
        }
        BrutalPacking pk(maxSize, loopLimit);

        stringstream fileName;
        fileName << path << "/polygon.json";
        cout << "Processing Polygon file: " << fileName.str() << endl;

        ifstream file;
        file.open(fileName.str().c_str(), ifstream::in);
        std::stringstream buffer;
        buffer << file.rdbuf();
//        cout << buffer.str() << endl;
        pk.readPolygon(buffer.str().c_str());
        file.close();
        pk.initDevice();
        if(parser.foundOption("debug")){
            stringstream debugFile;
            debugFile << path << "/packing_debug.json";
            file.open(debugFile.str().c_str(), ifstream::in);
            std::stringstream buffer;
            buffer << file.rdbuf();
            pk.startDebugPacking(BrutalPacking::ROTATION_90, buffer.str().c_str(), path);
            file.close();
            return true;
        }else{
            int ret = pk.startPacking(BrutalPacking::ROTATION_90);
            if(ret == 1){
                Document json = pk.toJson();
                stringstream output;
                output << path << "/packing.json";
                cout << "Save Results to file: " << output.str() << endl;
                FILE* fp = fopen(output.str().c_str(), "w");
                char writeBuffer[65536];

                FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
                Writer<FileWriteStream> writer(os);
                json.Accept(writer);
                fclose(fp);
            }else if(ret == -1){
                Document json = pk.toDebugJson();
                stringstream output;
                output << path << "/packing_debug.json";
                cout << "Save Results to file: " << output.str() << endl;
                FILE* fp = fopen(output.str().c_str(), "w");
                char writeBuffer[65536];

                FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
                Writer<FileWriteStream> writer(os);
                json.Accept(writer);
                fclose(fp);
                return false;
            }else{
                return false;
            }
        }

    }else{
        LogInfo("Something wrong with input parameter");
        return false;
    }
}

