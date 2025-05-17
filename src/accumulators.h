
#pragma once

#include "stdafx.h"

#include <stdio.h>

template <CoordType topX, CoordType topY>
class WeightIterator {
	typedef CoordType __CoordType;
    typedef WeightIterator self;
    interpolation_type* weights;
    int _gridSize;
public:
    WeightIterator(interpolation_type* f, int gridSize)
      : weights(f), _gridSize(gridSize) {}

    inline WeightIterator& operator=(const std::pair< Vertex_handle, float >& x) {
        float weight = x.second;
        CGALPoint point = x.first->point();
        __CoordType realX = point.x()/_gridSize - topX;
        __CoordType realY = point.y()/_gridSize - topY;
        this->weights[realY*4 + realX] = weight;
        return *this; 
    }

    inline WeightIterator& operator*() { return *this; }
    inline self& operator++() { return *this; } 
    inline self& operator++(int) { return *this; }
    inline self& operator=(std::pair< CGALPoint, __CoordType >& x) { return *this; }

};

template <typename T, int topX, int topY>
class Weights {

	static const int MAX_GRID_DISTANCE = 64;

	typedef typename T::Point TPoint;

public:

	interpolation_type values[MAX_GRID_DISTANCE*MAX_GRID_DISTANCE*4*4];
    int _gridSize;

    void printAll() {
        for (int i = MAX_GRID_DISTANCE*MAX_GRID_DISTANCE*4*4-1; i >= 0; i--) {
            if (values[i] > 0) {
                std::cout << values[i] << std::endl;
            }
        }
    }

    Weights(int gridSize) {

    	_gridSize = gridSize;

	    std::fill_n(&values[0], gridSize*gridSize*4*4, 0);

        T dtWeight;

        dtWeight.insert(TPoint(-gridSize, -gridSize));
        dtWeight.insert(TPoint(-gridSize,              0));
        dtWeight.insert(TPoint(-gridSize, +gridSize));

        dtWeight.insert(TPoint(             0, -gridSize));
        dtWeight.insert(TPoint(             0,              0));
        dtWeight.insert(TPoint(             0, +gridSize));

        dtWeight.insert(TPoint(+gridSize, -gridSize));
        dtWeight.insert(TPoint(+gridSize,              0));
        dtWeight.insert(TPoint(+gridSize, +gridSize));

        dtWeight.insert(TPoint(2*gridSize, -gridSize));
        dtWeight.insert(TPoint(2*gridSize,              0));
        dtWeight.insert(TPoint(2*gridSize, +gridSize));

        dtWeight.insert(TPoint(-gridSize,2*gridSize));
        dtWeight.insert(TPoint(             0,2*gridSize));
        dtWeight.insert(TPoint(+gridSize,2*gridSize));

        dtWeight.insert(TPoint(2*gridSize,2*gridSize));

        for (int x = 0; x < gridSize; x++) {
            for (int y = 0; y < gridSize; y++) {

                CGAL::natural_neighbor_coordinates_vertex_2(
                    dtWeight, 
                    CGALPoint(x, y),
					WeightIterator<topX, topY>(&values[y*gridSize + x], gridSize)
                );

            }
        }

        //TODO verificar se tem peso 0

    }


};

struct InterpolationInfo {
    InterpolationInfo() : red(0), green(0), blue(0), totalWeight(0), count(0) {
    }
    interpolation_type red;
    interpolation_type green;
    interpolation_type blue;
    interpolation_type totalWeight;
    int count;
    CGALPoint furthermostP;
};

template <typename TCoord, bool CHECK_NEGATIVE = false>
class InterpolationInfoIterator {

	typedef TCoord __CoordType;
    typedef InterpolationInfoIterator self;

  public:

    InterpolationInfoIterator(InterpolationInfo* f, CGALPoint p)
      : info(f), furthermostDist(0), currentP(p) {}

    /*
    template <class T> InterpolationInfoIterator& operator=(const T& value) {
      //info(value); 
      return *this; 
    }
    */
    inline InterpolationInfoIterator& operator=(const std::pair< Vertex_handle, interpolation_type >& x) {
        interpolation_type weight = x.second;
        CGALPoint p = x.first->point();
        CGALPointInfo info = x.first->info();
        int dist = square(p.x()-currentP.x()) + square(p.y()-currentP.y());
        if (dist > furthermostDist) {
        	furthermostDist = dist;
        	this->info->furthermostP = p;
        }
        if (CHECK_NEGATIVE && info.red < 0) {
        //if (info.red < 0) {
            return *this;
        }
        //std::cout << "weight=" << weight << std::endl;
        this->info->red += (info.red * weight);
        this->info->green += (info.green * weight);
        this->info->blue += (info.blue * weight);
        this->info->totalWeight += weight;
        this->info->count++;
        return *this; 
    }

    inline InterpolationInfoIterator& operator*() { return *this; }
    inline self& operator++() { return *this; } 
    inline self& operator++(int) { return *this; }
    inline self& operator=(std::pair< CGALPoint, __CoordType >& x) { return *this; }
  private:
    InterpolationInfo* info;
    int furthermostDist;
    CGALPoint currentP;
  };


typedef CGAL::Triple< InterpolationInfoIterator<CoordType, true> , ShadingTriangulation::Geom_traits::FT, bool > ResultTripleCheck;
typedef CGAL::Triple< InterpolationInfoIterator<CoordType, false>, ShadingTriangulation::Geom_traits::FT, bool > ResultTripleDontCheck;

