#pragma once

#include "stdafx.h"
#include "accumulators.h"
#include <valarray>

class StopWatch {

    struct timeval startTime;

    std::stringstream values;
    std::stringstream header;

public:

   void start() {
       gettimeofday(&startTime, NULL);
   }

   void mark(const std::string& msg) {

       this->header << "," << msg;

       struct timeval now;
       gettimeofday(&now, NULL);

       double diff = (now.tv_sec-startTime.tv_sec+1e-6*(now.tv_usec-startTime.tv_usec));

       this->values << "," << diff;

       //std::cerr << msg << std::endl;

   }

   void printHeader(std::ostream& out) {
	   out << this->header.str();
   }

   void printValues(std::ostream& out) {
	   out << this->values.str();
   }

   inline void debug(const std::string& msg) {
       if (STOPWATCH_DEBUG) {
           std::cout << msg << std::endl;
       }
   }

};


template<typename T> inline T isqrt(T v) {
    T rem = 0;
    T root = 0;
    for (int i = 15; i >= 0; i--) {
        root <<= 1;
        rem = ((rem << 2) + (v >> 30));
        v <<= 2;
        root++;
        if (root <= rem) {
            rem -= root;
            root++;
        }
        else {
            root--;
        }
    }
    return (root >> 1);
}

enum OutputType {
    NORMALIZED,
    SHADING,
    BUCS,
    BUCS_BG
};

class Params {
public:
	short K_NEIGHBORS;
	short MAX_COORDS;
	int GRID_DISTANCE;
	int CELL_SEARCH_LENGTH;
	short DIFF_BUC;
	short PERC_BUC;
	int MIN_VOTES_PERCENTAGE;
    short bgRed;
    short bgGreen;
    short bgBlue;
    OutputType output;
};

std::ostream&
operator<<(std::ostream& os,
           const Params& p) {

	os <<
			"K" << std::setfill('0') << std::setw(2) << p.K_NEIGHBORS <<
			"_MC" << std::setfill('0') << std::setw(2) << p.MAX_COORDS <<
			"_GD" << std::setfill('0') << std::setw(2) << p.GRID_DISTANCE <<
			"_SL" << std::setfill('0') << std::setw(2) << p.CELL_SEARCH_LENGTH <<
			"_DBUC" << std::setfill('0') << std::setw(2) << p.DIFF_BUC <<
			"_PBUC" << std::setfill('0') << std::setw(2) << p.PERC_BUC <<
			"_VP" << std::setfill('0') << std::setw(2) << p.MIN_VOTES_PERCENTAGE
			;

	return os;


}


class ShadingRemoval
{

private:

    StopWatch stopWatch;

	static const short THREADS = 1;
	static const CoordType TOP_X = -1;
	static const CoordType TOP_Y = -1;

    cv::Mat imNorm;
    cv::Mat imBinary;
    int bgColor;

public:

    int getBgColor() {
    	return bgColor;
    }

    const cv::Mat& getNormalized() {
        return imNorm;
    }

    const cv::Mat& getBinary() {
        return imBinary;
    }

    void startStopWatch() {
    	stopWatch.start();
    }

    void doneStopWatch() {
    	stopWatch.mark("done");
    }

    void printHeader(std::ostream& out) {
    	stopWatch.printHeader(out);
    }

    void printStopWatch(std::ostream& out) {
    	stopWatch.printValues(out);
    }

	bool normalize(const cv::Mat& im, const Params params);

	ShadingRemoval(void);
	~ShadingRemoval(void);

};



class ClusterInfo {
public:

	int32_t _blockInCenterCount;
	int32_t _clusterCount;
	uint16_t _clusterCode;
	int32_t _totalRed;
	int32_t _totalGreen;
	int32_t _totalBlue;

	ClusterInfo() : _blockInCenterCount(0), _clusterCount(0), _clusterCode(0),
			_totalRed(0), _totalGreen(0), _totalBlue(0) {

	};

    void merge(const ClusterInfo& c) {
        this->_blockInCenterCount += c._blockInCenterCount;
        this->_clusterCount += c._clusterCount;
        this->_totalRed += c._totalRed;
        this->_totalGreen += c._totalGreen;
        this->_totalBlue += c._totalBlue;
    }

};

#define calcColor(bodyStatement) \
    {const int coordX = -1; const int coordY = -1; bodyStatement;} \
    {const int coordX =  0; const int coordY = -1; bodyStatement;} \
    {const int coordX = +1; const int coordY = -1; bodyStatement;} \
    {const int coordX = +2; const int coordY = -1; bodyStatement;} \
    {const int coordX = -1; const int coordY =  0; bodyStatement;} \
    {const int coordX =  0; const int coordY =  0; bodyStatement;} \
    {const int coordX = +1; const int coordY =  0; bodyStatement;} \
    {const int coordX = +2; const int coordY =  0; bodyStatement;} \
    {const int coordX = -1; const int coordY = +1; bodyStatement;} \
    {const int coordX =  0; const int coordY = +1; bodyStatement;} \
    {const int coordX = +1; const int coordY = +1; bodyStatement;} \
    {const int coordX = +2; const int coordY = +1; bodyStatement;} \
    {const int coordX = -1; const int coordY = +2; bodyStatement;} \
    {const int coordX =  0; const int coordY = +2; bodyStatement;} \
    {const int coordX = +1; const int coordY = +2; bodyStatement;} \
    {const int coordX = +2; const int coordY = +2; bodyStatement;}


struct BGCounter {
    int incR, incG, incB;
    int count;
};

struct PointColor {
    uchar red;
    uchar green;
    uchar blue;
};

inline bool toClose(NNPointInfo* c1, NNPointInfo* c2, short d) {
		    return 
                (abs(c1->red - c2->red) <= d) &&
		    (abs(c1->green - c2->green) <= d) &&
            (abs(c1->blue - c2->blue) <= d)
		    ;
};

inline void extrapolate(ShadingTriangulation& dt, CGALPoint ghost, CGALPoint superGhost) {

    Vertex_handle spHandle;
    {
        spHandle = dt.insert(superGhost);
        CGALPointInfo* info = &(spHandle->info()); 
        info->red = -0.00001f;
        info->green = -0.00001f;
        info->blue = -0.00001f;
    }

    InterpolationInfo interInfo;

    ResultTripleCheck normTriple = 
        CGAL::natural_neighbor_coordinates_vertex_2(
        dt, 
        ghost,
        InterpolationInfoIterator<CoordType, true>(&interInfo, superGhost)
    );

    if (normTriple.third) {

        Vertex_handle handle = dt.insert(ghost);
        CGALPointInfo* info = &(handle->info());

        info->red   = interInfo.red/interInfo.totalWeight;
        info->green = interInfo.green/interInfo.totalWeight;
        info->blue  = interInfo.blue/interInfo.totalWeight;

    }
    else {
    	std::cerr << "ghost=" << ghost.x() << "," << ghost.y() << std::endl;
    	std::cerr << "superGhost=" << superGhost.x() << "," << superGhost.y() << std::endl;
    	exit(-1);
    	assert(normTriple.third);
    }

    dt.remove(spHandle);

}

inline void printInfo(const std::string& title, int value) {
	std::cout << value << ",";
}

bool ShadingRemoval::normalize(const cv::Mat& srcImg, const Params params) {

    const CoordType _XI = 1;
    const CoordType _YI = 0;

    cv::Size srcSize = srcImg.size();

    printInfo("width", srcSize.width);
    printInfo("height", srcSize.height);
    uchar* srcData = srcImg.data;

    stopWatch.debug("finding ngbs");

	CoordType dtWidth = srcSize.width/params.GRID_DISTANCE + 2;
    CoordType dtHeight = srcSize.height/params.GRID_DISTANCE + 2;

    std::vector<Point> vertices;
    std::vector<NNPointInfo> pointInfos;

#ifdef SR_RESEARCH

#define cleanhist { \
    blocksIn = 0; \
	blocksTotal = 0; \
    for (int i = 255; i >= 0; i--) { \
    	hist[0][i] = 0; \
    	hist[1][i] = 0; \
    	hist[2][i] = 0; \
    	hist[3][i] = 0; \
    } \
}

    int32_t hist[4][256];
    int32_t blocksIn;
    int32_t blocksTotal;
    cleanhist
#endif

    {

        CoordType hor_cells = srcSize.width/params.GRID_DISTANCE;
        CoordType ver_cells = srcSize.height/params.GRID_DISTANCE;

        unsigned int maxSize = (hor_cells-3)*(hor_cells-3);

        vertices.reserve(maxSize);
        pointInfos.reserve(maxSize);

        BIB zOrderIterator;

		InterleaveType nji;
		InterleaveType fjo;

		{

			const short topInterleave =
					params.CELL_SEARCH_LENGTH/params.GRID_DISTANCE + 1;

			InterleaveType minI;
			AcessMethod::interleave(minI, Point(topInterleave,topInterleave));
			//std::cerr << "minI=" << minI;

			InterleaveType maxI;
			AcessMethod::interleave(maxI, Point(ver_cells-topInterleave, hor_cells-topInterleave));
			//std::cerr << "maxI=" << maxI;

			zOrderIterator.setup(minI, maxI);

			fjo = minI;
			nji = minI;

		}

	    short redHistogram[256];
	    short greenHistogram[256];
	    short blueHistogram[256];

		while (zOrderIterator.next(fjo, nji)) {

			for (; nji < fjo; nji = nji + InterleaveType(1)) {

				Point cellCoord;
				AcessMethod::uninterleave(nji, cellCoord);

				int xCenter = cellCoord[_XI]*params.GRID_DISTANCE;
				int yCenter = cellCoord[_YI]*params.GRID_DISTANCE;

				int minY = yCenter - params.CELL_SEARCH_LENGTH;
				int maxY = yCenter + params.CELL_SEARCH_LENGTH;

				if (minY < 0) {
					std::cerr << "yCenter=" << yCenter << ", nji=" << nji << ", minY=" << minY << std::endl;
					//zOrderIterator.printDebug();
				}

				if (maxY > srcSize.height) {
					std::cerr << "yCenter=" << yCenter << ", nji=" << nji << ", maxY=" << maxY << std::endl;
					//zOrderIterator.printDebug();
				}

				{

					std::fill_n(&redHistogram[0], 256, 0);
					std::fill_n(&greenHistogram[0], 256, 0);
					std::fill_n(&blueHistogram[0], 256, 0);

					CoordType minX = xCenter - params.CELL_SEARCH_LENGTH;
					CoordType maxX = xCenter + params.CELL_SEARCH_LENGTH;

					if (minX < 0) {
						std::cerr << "xCenter=" << xCenter << ", nji=" << nji << ", minX=" << minX << std::endl;
						//zOrderIterator.printDebug();
					}

					if (maxX > srcSize.width) {
						std::cerr << "xCenter=" << xCenter << ", nji=" << nji << ", maxX=" << maxX << std::endl;
						//zOrderIterator.printDebug();
					}


					//assert(minX >= 0);

					//assert(maxX < srcSize.width);

					int colorsCount = 0;
					int yOffset = minY*srcSize.width;
					for (CoordType y = minY; y <= maxY; y++) {
						for (CoordType x = minX; x <= maxX; x++) {
							uchar* value = &srcData[(yOffset+x)*3];
							redHistogram[value[0]]++;
							greenHistogram[value[1]]++;
							blueHistogram[value[2]]++;
							colorsCount++;
						}
						yOffset += srcSize.width;
					}


					NNPointInfo mode;
					mode.red = 0;
					mode.green = 0;
					mode.blue = 0;

					for (int i = 255; i >= 0; i--) {
						if (redHistogram[i] > redHistogram[mode.red]) {
							mode.red = i;
						}
						if (greenHistogram[i] > greenHistogram[mode.green]) {
							mode.green = i;
						}
						if (blueHistogram[i] > blueHistogram[mode.blue]) {
							mode.blue = i;
						}
					}

#ifdef SR_RESEARCH

					for (CoordType y = minY; y <= maxY; y++) {
						for (CoordType x = minX; x <= maxX; x++) {
							uchar* value = &srcData[(yOffset+x)*3];
							int32_t diff0 = abs(value[0]-mode.red);
							int32_t diff1 = abs(value[1]-mode.green);
							int32_t diff2 = abs(value[2]-mode.blue);
							hist[0][diff0]++;
							hist[1][diff1]++;
							hist[2][diff2]++;
							hist[3][my_max(my_max(diff0, diff1), diff2)]++;
						}
					}
					blocksTotal++;

#endif
					int insideGuassianRed;
					int insideGuassianGreen;
					int insideGuassianBlue;
				
					{
						int regionCount = 0;
						const short maxAllowed = my_min(255, mode.red + params.DIFF_BUC);
						for (short i = my_max(0, mode.red - params.DIFF_BUC); i <= maxAllowed; i++) {
							regionCount += redHistogram[i];
						}
						insideGuassianRed = regionCount;
					}
								
					{
						int regionCount = 0;
						const short maxAllowed = my_min(255, mode.green + params.DIFF_BUC);
						for (short i = my_max(0, mode.green - params.DIFF_BUC); i <= maxAllowed; i++) {
							regionCount += greenHistogram[i];
						}
						insideGuassianGreen = regionCount;
					}
				
					{
						int regionCount = 0;
						const short maxAllowed = my_min(255, mode.blue + params.DIFF_BUC);
						for (short i = my_max(0, mode.blue - params.DIFF_BUC); i <= maxAllowed; i++) {
							regionCount += blueHistogram[i];
						}
						insideGuassianBlue = regionCount;
					}

					const int minBackgroundCount = (colorsCount*params.PERC_BUC)/100;

					if ( (insideGuassianRed >= minBackgroundCount) &&
						 (insideGuassianGreen >= minBackgroundCount) && 
						 (insideGuassianBlue >= minBackgroundCount) ) {

#ifdef SR_RESEARCH

					blocksIn++;

#endif
						mode.clusterCode = 0;
						vertices.push_back(cellCoord);
						pointInfos.push_back(mode);

					}

				}

			}


       }
       
    }

    //std::cerr << "vertices.size1()=" << vertices.size() << std::endl;

#ifdef SR_RESEARCH

#define printhist(prefix, reverse) \
	    for (int32_t comp = 0; comp <= 3; comp++) {  \
	        std::cout << prefix << comp;             \
	        for (int32_t i = 0; i <= 255; i++) { \
	        	int32_t index = (reverse)?(255-i):(i); \
	            std::cout << "," <<  hist[comp][index];  \
	        } \
	        std::cout << std::endl; \
	    } \
        std::cout << prefix << "Total," << blocksTotal << "," <<  blocksIn << "," __FILE__ << ":" << __LINE__ << std::endl;

printhist("comp", false)

#endif

   if (params.output == BUCS) {

	   std::vector<bool> bucs;

	   CoordType hor_bucs = srcSize.width/params.GRID_DISTANCE + 1;
	   CoordType ver_bucs = srcSize.height/params.GRID_DISTANCE + 1;

	   bucs.reserve(hor_bucs*ver_bucs);

	   for (int i = hor_bucs*ver_bucs - 1; i >= 0; i--) {
		   bucs.push_back(false);
	   }

	   for (int i = vertices.size()-1; i >= 0; i--) {
		   bucs[vertices[i][_YI]*hor_bucs + vertices[i][_XI]] = true;
	   }

	   for (int vBUC = ver_bucs-1; vBUC >= 0; vBUC--) {
		   for (int hBUC = hor_bucs-1; hBUC >= 0; hBUC--) {
			   if (!bucs[vBUC*hor_bucs + hBUC]) {
				   int minX = hBUC*params.GRID_DISTANCE;
				   int minY = vBUC*params.GRID_DISTANCE;
				   int maxX = my_min(minX + params.GRID_DISTANCE, srcSize.width);
				   int maxY = my_min(minY + params.GRID_DISTANCE, srcSize.height);
				   for (int y = minY; y < maxY; y++) {
					   for (int x = minX; x < maxX; x++) {
						   uchar* destPixel = &srcData[(y*srcSize.width + x)*3];
						   destPixel[0] = 0xFF;
						   destPixel[1] = 0xFF;
						   destPixel[2] = 0xFF;
					   }
				   }
			   }
		   }
	   }
       this->imNorm = srcImg;

	   return true;

	}

    if (vertices.size() <= 20) {
    	return false;
    }
    //std::cout << "#" << vertices.size()  << " vertices added!" << std::endl;
    //std::cout << "#" << pointInfos.size()  << " pointInfos added!" << std::endl;
    stopWatch.mark("bucs");

    sfcnn_knng<Point, Point::__DIM, Point::__NumType> knn(&vertices[0], vertices.size(), params.K_NEIGHBORS, THREADS);
    //std::cout << "# nn computed!" << std::endl;
    stopWatch.mark("knn comp.");

    printInfo("BUCs", vertices.size());


    ClusterInfo choosenCluster;
	CGALPointInfo globalBg;
	ShadingTriangulation dtNgbs;

    {

		int currentClusterCode = 0;

        int maxVotes = -1;

        ClusterInfo* maxCluster = NULL;

		std::list<ClusterInfo> clusterVoting;
		
		const int centerCellH = dtWidth/2;
		const int centerCellV = dtHeight/2;

		const int distanceFromCenterV = dtHeight/6;
		const int distanceFromCenterH = dtWidth/6;

        for (int vIndex = vertices.size()-1; vIndex >= 0; vIndex--) {

			if (pointInfos[vIndex].clusterCode <= 0) {

                currentClusterCode++;

                ClusterInfo clusterInfo = ClusterInfo();
				clusterInfo._clusterCode = currentClusterCode;

				std::list<unsigned int> floodList;
				floodList.push_back(vIndex);

				while (!floodList.empty()) {

                    unsigned int nIndex = floodList.front();
                    floodList.pop_front();
                    NNPointInfo* nInfo = &pointInfos[nIndex];
					if (nInfo->clusterCode > 0) {
						continue;
					}
                    nInfo->clusterCode = currentClusterCode;

                    Point point = vertices[nIndex];
                    
                    clusterInfo._totalRed   += nInfo->red;
                    clusterInfo._totalGreen += nInfo->green;
                    clusterInfo._totalBlue  += nInfo->blue;
					clusterInfo._clusterCount++;

					if (
							(abs(point[_YI] - centerCellV) < distanceFromCenterV)  &&
							(abs(point[_XI] - centerCellH) < distanceFromCenterH)
						) {
						clusterInfo._blockInCenterCount++;
					}

                    std::vector<long unsigned int> neighbors = knn[nIndex];

                    //std::cout << "# BEFORE" << std::endl;
                    for (int k = params.K_NEIGHBORS-1; k >= 0; k--) {
                        unsigned int kIndex = neighbors[k];
                        NNPointInfo* kInfo = &pointInfos[kIndex];
                        if (toClose(kInfo, nInfo, params.DIFF_BUC)) {
                            floodList.push_back(kIndex);                                                        
                        }
                    }

				}

				clusterVoting.push_back(clusterInfo);
				if (clusterInfo._clusterCount > maxVotes) {
                    maxVotes   = clusterInfo._clusterCount;
					maxCluster = &clusterVoting.back();
				}

            }
        }

        //std::cout << "currentClusterCode = " << currentClusterCode << std::endl;

        ClusterInfo* clusterCloserToCenter = maxCluster;

		{
            const int _50percent_arroundCenter = (vertices.size()/2) + 1; //=(ver_cells/3)*(hor_cells/3)*(1/2)
            const int minVotes  = (vertices.size()*params.MIN_VOTES_PERCENTAGE)/100;

            //std::cout << "clusterVoting.size() = " << clusterVoting.size() << std::endl;
			
			int clusterCloserToCenterCount = maxCluster->_blockInCenterCount;
			//the cluster with maximum votes has less than 50% of center blocks?
			if (clusterCloserToCenterCount < _50percent_arroundCenter) {
                //std::cout << "it find " << std::endl;
				std::list<ClusterInfo>::iterator it;
				for ( it = clusterVoting.begin() ; it != clusterVoting.end(); it++ ) {
					ClusterInfo* currentCluster = &(*it);
					if (currentCluster->_clusterCount >= minVotes ) //&& maxCluster != currentCluster)
					{
						if (currentCluster->_blockInCenterCount > clusterCloserToCenterCount) {
							//current cluster is closer to center
							clusterCloserToCenter = currentCluster;
							clusterCloserToCenterCount = currentCluster->_blockInCenterCount;
							if (clusterCloserToCenterCount >= _50percent_arroundCenter) {
								//this regions has more than 50% of the center blocks
								break;
							}
						}
					}
				}
			}

	        choosenCluster = *clusterCloserToCenter;

            //clusterVoting.clear();

            /*
            std::cout << 
                "_clusterCode=" << clusterCloserToCenter->_clusterCode << ", " << 
                "_clusterCount=" << clusterCloserToCenter->_clusterCount << ", " << 
                "_blockInCenterCount=" << clusterCloserToCenter->_blockInCenterCount  
                << std::endl;
            */

        }

    }
    stopWatch.mark("cluster selected");
    printInfo("BGs", choosenCluster._clusterCount);

    int blockInfoWidth  = (dtWidth +1-TOP_X)+1;
    int blockInfoHeight = (dtHeight+1-TOP_Y)+1;

    int blockInfoSize = blockInfoWidth*blockInfoHeight;
    std::vector<CGALPointInfo> blockInfos(blockInfoSize);
    for (int index = blockInfoSize - 1; index >= 0; index--) {
        blockInfos[index].ngb = false;
    }

#ifdef SR_RESEARCH
    cleanhist
#endif

    //std::cerr << "out2" << std::endl;
    {

		int meanBgRed = choosenCluster._totalRed/(choosenCluster._clusterCount);
		int meanBgGreen = choosenCluster._totalGreen/(choosenCluster._clusterCount);
		int meanBgBlue = choosenCluster._totalBlue/(choosenCluster._clusterCount);

        short choosenClusterCode = choosenCluster._clusterCode;

        short minDistance = 10000;

        for (int vIndex = vertices.size() - 1; vIndex >= 0; vIndex--) {

            NNPointInfo vInfo = pointInfos[vIndex];

#ifdef SR_RESEARCH
			blocksTotal++;
#endif

			if (vInfo.clusterCode == choosenClusterCode) {

#ifdef SR_RESEARCH
			blocksIn++;
#endif
                Point vPoint = vertices[vIndex];

                int index = (vPoint[_YI]-TOP_Y)*blockInfoWidth + (vPoint[_XI]-TOP_X);
                CGALPointInfo& hInfo = blockInfos[index];

                hInfo.red = vInfo.red;
                hInfo.green = vInfo.green;
                hInfo.blue = vInfo.blue;
                hInfo.ngb = true;

                //std::cout << " vertices added with delaunay!" << std::endl;

                short distance =
                    abs(meanBgRed-vInfo.red) + abs(meanBgGreen-vInfo.green) + abs(meanBgBlue-vInfo.blue);

                if (distance < minDistance) {
                    minDistance = distance;
                    globalBg = hInfo;
                }

            }

        }

    	if (params.bgRed >= 0) {

            globalBg.red = params.bgRed;
            globalBg.green = params.bgGreen;
            globalBg.blue = params.bgBlue;
            globalBg.ngb = false;

    	}

    	{
    		int r = globalBg.red;
    		int g = globalBg.green;
    		int b = globalBg.blue;
        	bgColor = r | (g << 8) | (b << 16);
    	}

#ifdef SR_RESEARCH
    	{
			const int delta = 1;
			for (int bX = blockInfoWidth-1-delta; bX >= delta; bX--) {
				for (int bY = blockInfoHeight-1-delta; bY >= delta; bY--) {
					int index1 = bY*blockInfoWidth + bX;
					if (blockInfos[index1].ngb) {
						for (int dX = -delta; dX <= delta; dX++) {
							for (int dY = -delta; dY <= delta; dY++) {
								int index2 = (bY+dY)*blockInfoWidth + (bX+dX);
								if (blockInfos[index2].ngb) {
									int diffR = abs(blockInfos[index1].red-blockInfos[index2].red);
									int diffG = abs(blockInfos[index1].green-blockInfos[index2].green);
									int diffB = abs(blockInfos[index1].blue-blockInfos[index2].blue);
									hist[0][diffB]++;
									hist[1][diffG]++;
									hist[2][diffR]++;
									hist[3][my_max(my_max(diffR, diffG), diffB)]++;
								}
							}
						}
					}
				}
			}
			printhist("diff", false)
    	}
#endif

        for (int index = blockInfoWidth*blockInfoHeight-1; index >= 0; index--) {

            if (blockInfos[index].ngb) {

                bool arroundMissing =
                    
                    !blockInfos[index-blockInfoWidth-1].ngb ||
                    !blockInfos[index-blockInfoWidth+0].ngb ||
                    !blockInfos[index-blockInfoWidth+1].ngb ||

                    !blockInfos[index-1].ngb ||
                    !blockInfos[index+1].ngb ||

                    !blockInfos[index+blockInfoWidth-1].ngb ||
                    !blockInfos[index+blockInfoWidth+0].ngb ||
                    !blockInfos[index+blockInfoWidth+1].ngb
                ;

                if (arroundMissing)
                {
                    CoordType x = (index % blockInfoWidth) + TOP_X;
                    CoordType y = (index / blockInfoWidth) + TOP_Y;
                    Vertex_handle handle = dtNgbs.insert(CGALPoint(x, y));
                    handle->info() = blockInfos[index];
                }

            }
        }

        vertices.clear();
        pointInfos.clear();

        stopWatch.mark("added to delaunay");

    }
    printInfo("delaunay", dtNgbs.number_of_vertices());

    if (params.output == BUCS_BG) {

 	    CoordType hor_bucs = srcSize.width/params.GRID_DISTANCE + 1;
 	    CoordType ver_bucs = srcSize.height/params.GRID_DISTANCE + 1;

 	    for (int vBUC = ver_bucs-1; vBUC >= 0; vBUC--) {
 		    for (int hBUC = hor_bucs-1; hBUC >= 0; hBUC--) {
 			    int index = (vBUC-TOP_Y)*blockInfoWidth + (hBUC-TOP_X);
 			    if (!blockInfos[index].ngb) {
 				    int minX = hBUC*params.GRID_DISTANCE;
 				    int minY = vBUC*params.GRID_DISTANCE;
 				    int maxX = my_min(minX + params.GRID_DISTANCE, srcSize.width);
 				    int maxY = my_min(minY + params.GRID_DISTANCE, srcSize.height);
 				    for (int y = minY; y < maxY; y++) {
 					    for (int x = minX; x < maxX; x++) {
 						    uchar* destPixel = &srcData[(y*srcSize.width + x)*3];
 						    destPixel[0] = 0xFF;
 						    destPixel[1] = 0xFF;
 						    destPixel[2] = 0xFF;
 					    }
 				    }
 			    }
 		    }
 	    }

        this->imNorm = srcImg;

 	    return true;

    }

    //std::cerr << "dtNgbs.number_of_vertices=" << dtNgbs.number_of_vertices() << std::endl;

    {

        extrapolate(dtNgbs, CGALPoint(TOP_X-5, TOP_Y-5), CGALPoint(TOP_X-100, TOP_Y-100));
        extrapolate(dtNgbs, CGALPoint(TOP_X-5, dtHeight  +  5), CGALPoint(TOP_X-100, dtHeight   + 100));
        extrapolate(dtNgbs, CGALPoint(dtWidth  +   5, TOP_Y-5), CGALPoint(dtWidth   +  100, TOP_Y-100));
        extrapolate(dtNgbs, CGALPoint(dtWidth  +   5, dtHeight  +  5), CGALPoint(dtWidth   +  100, dtHeight  +  100));

        stopWatch.debug("vertices extrapolated!");

        this->imNorm = srcImg;
        uchar* destData = srcData;

        {

        	
    		InterleaveType nji;
    		InterleaveType fjo;

            BIB zOrderIterator;

    		{

    			AcessMethod::interleave(fjo, Point(0,0));

    			InterleaveType maxI;
    			AcessMethod::interleave(maxI, Point(dtHeight-TOP_X, dtWidth-TOP_Y));

    			zOrderIterator.setup(fjo, maxI);

    			nji = fjo;

    		}

    		while (zOrderIterator.next(fjo, nji)) {

    			for (; nji < fjo; nji = nji + InterleaveType(1)) {

    				Point cellCoord;
    				AcessMethod::uninterleave(nji, cellCoord);

    				int x = cellCoord[_XI];
    				int y = cellCoord[_YI];

    				const int yOffset = y*blockInfoWidth;

                    int index = yOffset + x;

                    CGALPointInfo& bColor = blockInfos[index];

                    if (bColor.ngb) {
                        continue;
                    }

                    //Point pprime = zOrderIterator.getCurrent();
                    CGALPoint p(x+TOP_X, y+TOP_Y);

                    InterpolationInfo interInfo;

                    ResultTripleDontCheck normTriple = 
                        CGAL::natural_neighbor_coordinates_vertex_2(
                        dtNgbs, 
                        p,
                        InterpolationInfoIterator<CoordType, false>(&interInfo, p)
                        );

                    /*
                    if (!normTriple.third) {
                        std::cout << "interInfo.count=" << interInfo.count << std::endl;
                        continue;
                    }
                    */

                    bColor.red   = interInfo.red/interInfo.totalWeight;
                    bColor.green = interInfo.green/interInfo.totalWeight;
                    bColor.blue  = interInfo.blue/interInfo.totalWeight;

                    if (interInfo.count > params.MAX_COORDS) {

                        Vertex_handle handle = dtNgbs.insert(p);
                        handle->info() = bColor;

                    	int xMiddle = (p.x() + interInfo.furthermostP.x())/2;
                    	int yMiddle = (p.y() + interInfo.furthermostP.y())/2;

                    	if (0 <= xMiddle && xMiddle < dtWidth &&
                    			0 <= yMiddle && yMiddle < dtHeight) {

                        	CGALPoint pMiddle(xMiddle, yMiddle);

                        	InterpolationInfo infoMiddle;

                        	//std::cerr << "teste1" << std::endl;

                        	CGAL::natural_neighbor_coordinates_vertex_2(
                        	                        dtNgbs,
                        	                        pMiddle,
                        	                        InterpolationInfoIterator<CoordType, false>(&infoMiddle, pMiddle)
                        	                        );

                        	CGALPointInfo middleColor;
                        	middleColor.ngb = true;
                        	middleColor.red   = infoMiddle.red/infoMiddle.totalWeight;
                        	middleColor.green = infoMiddle.green/infoMiddle.totalWeight;
                        	middleColor.blue  = infoMiddle.blue/infoMiddle.totalWeight;

                            Vertex_handle handle = dtNgbs.insert(pMiddle);
                            handle->info() = bColor;

                        	//std::cerr << "teste2,pMiddle(" << pMiddle.x() << "," << pMiddle.y() << ")" << std::endl;

                            blockInfos[((yMiddle - TOP_Y)*blockInfoWidth) +
                                       xMiddle - TOP_X] = middleColor;

                    	}

                    }

                }
            }
        }

        //std::cerr << "dtNgbs.number_of_vertices=" << dtNgbs.number_of_vertices() << std::endl;

        dtNgbs.clear();

        stopWatch.mark("grid estimated");

        Weights<ShadingTriangulation, TOP_X, TOP_Y> weights(params.GRID_DISTANCE);
        stopWatch.debug("small weights estimated");

        {

            for (CoordType y = srcSize.height-1; y >= 0; y--) {

                CoordType dy = y % params.GRID_DISTANCE;
                CoordType blockY = (y / params.GRID_DISTANCE) - TOP_Y;
                int dyOffset = dy*params.GRID_DISTANCE;
                int yOffset = y*srcSize.width;
                int blockYoffset = blockY*blockInfoWidth;

                for (CoordType x = srcSize.width-1; x >= 0; x--) {

                    CoordType dx = x % params.GRID_DISTANCE;
                    CoordType blockX = (x / params.GRID_DISTANCE) - TOP_X;

                    interpolation_type totalWeight = 0;
                    interpolation_type red = 0;
                    interpolation_type green = 0;
                    interpolation_type blue = 0;

                    interpolation_type* weightsValues = &weights.values[dyOffset + dx];

                    register int center = blockYoffset + blockX;

                    calcColor(
                    {
                    	const int topOffset = -TOP_Y*4 - TOP_X;
                        float weight = weightsValues[coordY*4 + coordX + topOffset];
                        //if (weight > 0)
                        {
                            CGALPointInfo& info = 
                                blockInfos[center + coordX + coordY*blockInfoWidth];
                            red   += weight*info.red;
                            green += weight*info.green;
                            blue  += weight*info.blue;
                            totalWeight += weight;
                        }
                    }
                    );

                    /*
                    red = red/(globalBg.red*totalWeight);
                    green = green/(globalBg.green*totalWeight);
                    blue = blue/(globalBg.blue*totalWeight);

                    destPixel[0] = validPixel(srcPixel[0], red);
                    destPixel[1] = validPixel(srcPixel[1], green);
                    destPixel[2] = validPixel(srcPixel[2], blue);
                    */

                    uchar* destPixel = &destData[(yOffset + x)*3];

                    if (params.output == SHADING) {
                        destPixel[0] = validPixel(red, totalWeight);
                        destPixel[1] = validPixel(green, totalWeight);
                        destPixel[2] = validPixel(blue, totalWeight);
                    }
                    else {
                        uchar* srcPixel = destPixel;
                        destPixel[0] = validPixel(srcPixel[0]*totalWeight*globalBg.red, red);
                        destPixel[1] = validPixel(srcPixel[1]*totalWeight*globalBg.green, green);
                        destPixel[2] = validPixel(srcPixel[2]*totalWeight*globalBg.blue, blue);
                    }

                }
            }

        }

#ifdef SR_RESEARCH

		cleanhist
		for (int index = srcSize.height*srcSize.width-1; index >= 0; index--) {
			uchar* destPixel = &destData[index*3];
			hist[0][destPixel[0]]++;
			hist[1][destPixel[1]]++;
			hist[2][destPixel[2]]++;
			hist[3][my_min(my_min(destPixel[0], destPixel[1]), destPixel[2])]++;
		}
		printhist("res", true)

#endif

    }

	return true;

}

ShadingRemoval::ShadingRemoval(void)
{
}

ShadingRemoval::~ShadingRemoval(void)
{
}

