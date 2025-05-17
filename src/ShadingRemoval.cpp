// ShadingRemoval.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ShadingRemoval.h"

#include <boost/program_options.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
namespace po = boost::program_options;


int main(int ac, char **av)
{

	int result = 0;

    try {

        po::positional_options_description p;
        p.add("input", 1);
        p.add("output", 1);

        bool printHeader = false;
        std::string inputFilename;
        std::string outputFilename;
        std::string background;
        std::string outputType;

        Params params;

        params.K_NEIGHBORS = 4; //ok
        params.MAX_COORDS = 16; //ok
        params.GRID_DISTANCE = 8; //ok
        params.CELL_SEARCH_LENGTH = 8; //ok

        params.DIFF_BUC = 6;
        params.PERC_BUC = 82;
        params.MIN_VOTES_PERCENTAGE = 10;
        params.bgRed = -1;
        params.bgGreen = -1;
        params.bgBlue = -1;
        params.output = NORMALIZED;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("input", po::value<string>(&inputFilename), "Input file")
            ("output", po::value<string>(&outputFilename), "Output file")
            ("header", po::value<bool>(&printHeader), "Timing header")
            ("type", po::value<string>(&outputType), "Background color")
            ("K", po::value<short>(&params.K_NEIGHBORS), "K neighbors")
            ("MC", po::value<short>(&params.MAX_COORDS), "Max coordinates")
            ("GD", po::value<int>(&params.GRID_DISTANCE), "CELL_SEARCH_LENGTH")
            ("SL", po::value<int>(&params.CELL_SEARCH_LENGTH), "CELL_SEARCH_LENGTH")
            ("DIFF_BUC", po::value<short>(&params.DIFF_BUC), "DIFF_BUC")
            ("PERC_BUC", po::value<short>(&params.PERC_BUC), "PERC_BUC")
            ("MVP", po::value<int>(&params.MIN_VOTES_PERCENTAGE), "MIN_VOTES_PERCENTAGE")
            ("BG", po::value<string>(&background), "Background color")
        ;

        po::variables_map vm;
        po::store(po::command_line_parser(ac, av).
                  options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << std::endl;
            return 1;
        }

        int result = -1;

        //std::cout << "Loading '" << inputFilename << "' file!" << std::endl;
        cv::Mat imIn = cv::imread(inputFilename, 1);

        if (!imIn.data)
        {
            std::cerr << "ExceptionObject caught while reading image '" << inputFilename << "' !" << std::endl;
            return -1;
        }

        if (background.length() > 0){
        	if (background.length() != 6) {
        		cerr << "Background value should have 6 digits!\n";
        		throw 1;
        	}
        	try {

        		int bg;

                std::istringstream iss(background);
                iss >> std::setbase(16) >> bg;

                params.bgRed = bg & 0xFF;
                params.bgGreen = (bg >> 8) & 0xFF;
                params.bgBlue = (bg >> 16) & 0xFF;

        	}
            catch(...) {
                cerr << "Invalid background value!\n";
                throw 1;
            }
        }

        if (outputType.length() > 0) {
        	boost::unordered_map<std::string, OutputType> types = boost::assign::map_list_of
        	    ("NORMALIZED", NORMALIZED)
				("SHADING", SHADING)
				("BUCS", BUCS)
				("BUCS_BG", BUCS_BG)
        	    ;
        	params.output = types[outputType];
        }

        ShadingRemoval shadingRemoval; //SHADING, NORMALIZED

        shadingRemoval.startStopWatch();
        if (!shadingRemoval.normalize(imIn, params)) {
        	std::cerr << "Command not executed with success" << std::endl;
        	throw 1;
        }
        shadingRemoval.doneStopWatch();
#ifndef SR_RESEARCH
        if (printHeader) {
            std::cout << "params,bgcolor";
            shadingRemoval.printHeader(cout);
            std::cout << std::endl;
        }
#endif
        std::cout << params;
#ifndef SR_RESEARCH
        std::cout << "," << std::setbase(16) << shadingRemoval.getBgColor();
        shadingRemoval.printStopWatch(cout);
#endif


        std::vector<int>	saveOptions;

        std::string extension = outputFilename.substr(outputFilename.length()-3, 3);

        if (extension == "png") {
            saveOptions.push_back(CV_IMWRITE_PNG_COMPRESSION);
            saveOptions.push_back(9);
        }
        if (extension == "jpg") {
            saveOptions.push_back(CV_IMWRITE_JPEG_QUALITY);
            saveOptions.push_back(100);
        }

        if (!cv::imwrite(outputFilename, shadingRemoval.getNormalized(), vector<int>())) {
            std::cerr << "Error " << result << " while writting image:\n"
                        << "  " << outputFilename << "\n";
            return -1;
        }

    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        result = -1;
    }
    catch(...) {
    	result = -1;
        cerr << "Exception of unknown type!\n";
    }

    exit(result);

    return result;

}
