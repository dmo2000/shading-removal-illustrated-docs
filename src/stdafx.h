// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <limits>
#include <stdio.h>
#include <tchar.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <time.h>
#include <iostream>
#include <string>
#include <list>
#include <iterator>
#include <highgui.h>
#include <dpoint.hpp>
#include <sfcnn_knng.hpp>
#include <sfcnn.hpp>

#include <boost/function_output_iterator.hpp>
#include <boost/lexical_cast.hpp>

#include <CGAL/Triangulation_hierarchy_vertex_base_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Cartesian.h>
#include <CGAL/utility.h>
#include <CGAL/natural_neighbor_coordinates_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_hierarchy_2.h>

#include "lipoint.h"
#include "linearizer_bitset.h"

//USER DEFINED PARAMS

struct NNPointInfo {
    bool nearNonNgb;
    short clusterCode;
    short red;
    short green;
    short blue;
};

/*
class CGALPointInfo {
public:
	static const short FIXED_POINT = 8;
	static const int MUL = 1 << FIXED_POINT;
	int32_t red;
	int32_t green;
	int32_t blue;
    bool ngb;
};
*/

typedef float interpolation_type;

struct CGALPointInfo {
	interpolation_type red;
	interpolation_type green;
	interpolation_type blue;
    bool ngb;
};

//typedef CGAL::Filtered_kernel< CGAL::Simple_cartesian<int> > K;
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<CGALPointInfo,K> Vinfo;
//typedef CGAL::Triangulation_vertex_base_2<K, Vinfo> Vb;
typedef CGAL::Triangulation_hierarchy_vertex_base_2<Vinfo> Vb;
typedef CGAL::Triangulation_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> InnerTriangulation;
typedef CGAL::Triangulation_hierarchy_2<InnerTriangulation> ShadingTriangulation;

//typedef CGAL::Delaunay_mesh_size_criteria_2<ShadingTriangulation> Criteria;
//typedef CGAL::Delaunay_mesher_2<ShadingTriangulation, Criteria> Delaunay_Mesher;

typedef ShadingTriangulation::Face_handle Face_handle;
typedef ShadingTriangulation::Finite_faces_iterator Finite_faces_iterator;
typedef ShadingTriangulation::Finite_vertices_iterator Finite_vertices_iterator;
typedef ShadingTriangulation::Finite_edges_iterator Finite_edges_iterator;
typedef ShadingTriangulation::Point CGALPoint;
typedef ShadingTriangulation::Vertex_handle Vertex_handle;
typedef ShadingTriangulation::Vertex_circulator Vertex_circulator;

//typedef unsigned __int32 uint32_t;
//typedef signed __int16 int16_t;

typedef int16_t CoordType;
typedef lipoint<CoordType, 2> Point;

typedef BigInteger<uint32_t, 1> InterleaveType;

typedef BitsetAccessMethod<Point, InterleaveType> AcessMethod;
typedef BitsetInterleaveBox<AcessMethod> BIB;


const short DIM = Point::__DIM;
const bool STOPWATCH_DEBUG = false;

template <class T> inline T my_min(const T x, const T y) { return x<y?x:y; };
template <class T> inline T my_max(const T x, const T y) { return x>y?x:y; };
template <class T> inline T square(T x) { return x*x; };
inline uchar validPixel(const float num, const float denom) { 
    if (denom!=0) {
        float value = num/denom;
        if (value <= 0) {
            return 0;
        }
        else if (value >= 255) {
            return 255;
        }
        else {
            return value;
        }
    }
    else {
        return 255;
    }

};





// TODO: reference additional headers your program requires here
