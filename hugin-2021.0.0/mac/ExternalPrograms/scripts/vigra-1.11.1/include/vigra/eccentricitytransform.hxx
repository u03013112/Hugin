/************************************************************************/
/*                                                                      */
/*     Copyright 2014 by Philip Schill and Ullrich Koethe               */
/*                                                                      */
/*    This file is part of the VIGRA computer vision library.           */
/*    The VIGRA Website is                                              */
/*        http://hci.iwr.uni-heidelberg.de/vigra/                       */
/*    Please direct questions, bug reports, and contributions to        */
/*        ullrich.koethe@iwr.uni-heidelberg.de    or                    */
/*        vigra@informatik.uni-hamburg.de                               */
/*                                                                      */
/*    Permission is hereby granted, free of charge, to any person       */
/*    obtaining a copy of this software and associated documentation    */
/*    files (the "Software"), to deal in the Software without           */
/*    restriction, including without limitation the rights to use,      */
/*    copy, modify, merge, publish, distribute, sublicense, and/or      */
/*    sell copies of the Software, and to permit persons to whom the    */
/*    Software is furnished to do so, subject to the following          */
/*    conditions:                                                       */
/*                                                                      */
/*    The above copyright notice and this permission notice shall be    */
/*    included in all copies or substantial portions of the             */
/*    Software.                                                         */
/*                                                                      */
/*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND    */
/*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   */
/*    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          */
/*    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT       */
/*    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,      */
/*    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      */
/*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR     */
/*    OTHER DEALINGS IN THE SOFTWARE.                                   */
/*                                                                      */
/************************************************************************/


#ifndef VIGRA_ECCENTRICITYTRANSFORM_HXX
#define VIGRA_ECCENTRICITYTRANSFORM_HXX

/*std*/
#include <algorithm>
#include <set>

/*vigra*/
#include "accumulator.hxx"
#include "multi_labeling.hxx"
#include "multi_distance.hxx"
#include "multi_resize.hxx"
#include "graph_algorithms.hxx"


namespace vigra
{


template <class Graph, class WeightType,
          class EdgeMap, class Shape>
TinyVector<MultiArrayIndex, Shape::static_size>
eccentricityCentersOneRegionImpl(ShortestPathDijkstra<Graph, WeightType> & pathFinder,
                        const EdgeMap & weights, WeightType maxWeight,
                        Shape anchor, Shape const & start, Shape const & stop)
{
    int maxIterations = 4;
    for(int k=0; k < maxIterations; ++k)
    {
        pathFinder.run(start, stop, weights, anchor, lemon::INVALID, maxWeight);
        anchor = pathFinder.target();
        // FIXME: implement early stopping when source and target don't change anymore
    }

    Polygon<TinyVector<float, Shape::static_size> > path;
    path.push_back_unsafe(anchor);
    while(pathFinder.predecessors()[path.back()] != path.back())
        path.push_back_unsafe(pathFinder.predecessors()[path.back()]);
    return path[roundi(path.arcLengthQuantile(0.5))];
}

template <unsigned int N, class T, class S, class Graph,
          class ACCUMULATOR, class DIJKSTRA, class Array>
void
eccentricityCentersImpl(const MultiArrayView<N, T, S> & src,
                        Graph const & g,
                        ACCUMULATOR const & r,
                        DIJKSTRA & pathFinder,
                        Array & centers)
{
    using namespace acc;
    typedef typename MultiArrayShape<N>::type Shape;
    typedef typename Graph::Node Node;
    typedef typename Graph::EdgeIt EdgeIt;
    typedef float WeightType;

    typename Graph::template EdgeMap<WeightType> weights(g);
    WeightType maxWeight = 0.0,
               minWeight = N;
    {
        AccumulatorChainArray<CoupledArrays<N, WeightType, T>,
                              Select< DataArg<1>, LabelArg<2>, Maximum> > a;

        MultiArray<N, WeightType> distances(src.shape());
        boundaryMultiDistance(src, distances, true);
        extractFeatures(distances, src, a);
        for (EdgeIt edge(g); edge != lemon::INVALID; ++edge)
        {
            const Node u(g.u(*edge)), v(g.v(*edge));
            const T label = src[u];
            if(label != src[v])
            {
                weights[*edge] = NumericTraits<WeightType>::max();
            }
            else
            {
                WeightType weight = norm(u - v) *
                                  (get<Maximum>(a, label) + minWeight - 0.5*(distances[u] + distances[v]));
                weights[*edge] = weight;
                maxWeight = std::max(weight, maxWeight);
            }
        }
    }
    maxWeight *= src.size();

    T maxLabel = r.maxRegionLabel();
    centers.resize(maxLabel+1);

    for (T i=0; i <= maxLabel; ++i)
    {
        if(get<Count>(r, i) == 0)
            continue;
        centers[i] = eccentricityCentersOneRegionImpl(pathFinder, weights, maxWeight,
                                             get<RegionAnchor>(r, i),
                                             get<Coord<Minimum> >(r, i),
                                             get<Coord<Maximum> >(r, i) + Shape(1));
    }
}

/** \addtogroup DistanceTransform
*/
//@{

    /** \brief Find the (approximate) eccentricity center in each region of a labeled image.

        <b> Declarations:</b>

        pass arbitrary-dimensional array views:
        \code
        namespace vigra {
            template <unsigned int N, class T, class S, class Array>
            void
            eccentricityCenters(MultiArrayView<N, T, S> const & src,
                                Array & centers);
        }
        \endcode

        \param[in] src : labeled array
        \param[out] centers : list of eccentricity centers (required interface:
                               <tt>centers[k] = TinyVector<int, N>()</tt> must be supported)

        <b> Usage:</b>

        <b>\#include</b> \<vigra/eccentricitytransform.hxx\><br/>
        Namespace: vigra

        \code
        Shape3 shape(width, height, depth);
        MultiArray<3, UInt32> labels(shape);
        ArrayVector<TinyVector<Int32, N> > centers;
        ...

        eccentricityCenters(labels, centers);
        \endcode
    */
template <unsigned int N, class T, class S, class Array>
void
eccentricityCenters(const MultiArrayView<N, T, S> & src,
                    Array & centers)
{
    using namespace acc;
    typedef GridGraph<N> Graph;
    typedef float WeightType;

    Graph g(src.shape(), IndirectNeighborhood);
    ShortestPathDijkstra<Graph, WeightType> pathFinder(g);

    AccumulatorChainArray<CoupledArrays<N, T>,
                          Select< DataArg<1>, LabelArg<1>,
                                  Count, BoundingBox, RegionAnchor> > a;
    extractFeatures(src, a);

    eccentricityCentersImpl(src, g, a, pathFinder, centers);
}

    /** \brief Computes the (approximate) eccentricity transform on each region of a labeled image.

        <b> Declarations:</b>

        pass arbitrary-dimensional array views:
        \code
        namespace vigra {
            // compute only the accentricity transform
            template <unsigned int N, class T, class S>
            void
            eccentricityTransformOnLabels(MultiArrayView<N, T> const & src,
                                          MultiArrayView<N, S> dest);

            // also return the eccentricity center of each region
            template <unsigned int N, class T, class S, class Array>
            void
            eccentricityTransformOnLabels(MultiArrayView<N, T> const & src,
                                          MultiArrayView<N, S> dest,
                                          Array & centers);
        }
        \endcode

        \param[in] src : labeled array
        \param[out] dest : eccentricity transform of src
        \param[out] centers : (optional) list of eccentricity centers (required interface:
                               <tt>centers[k] = TinyVector<int, N>()</tt> must be supported)

        <b> Usage:</b>

        <b>\#include</b> \<vigra/eccentricitytransform.hxx\><br/>
        Namespace: vigra

        \code
        Shape3 shape(width, height, depth);
        MultiArray<3, UInt32> labels(shape);
        MultiArray<3, float> dest(shape);
        ArrayVector<TinyVector<Int32, N> > centers;
        ...

        eccentricityTransformOnLabels(labels, dest, centers);
        \endcode
    */
template <unsigned int N, class T, class S, class Array>
void
eccentricityTransformOnLabels(MultiArrayView<N, T> const & src,
                              MultiArrayView<N, S> dest,
                              Array & centers)
{
    using namespace acc;
    typedef typename MultiArrayShape<N>::type Shape;
    typedef GridGraph<N> Graph;
    typedef typename Graph::Node Node;
    typedef typename Graph::EdgeIt EdgeIt;
    typedef float WeightType;

    vigra_precondition(src.shape() == dest.shape(),
        "eccentricityTransformOnLabels(): Shape mismatch between src and dest.");

    Graph g(src.shape(), IndirectNeighborhood);
    ShortestPathDijkstra<Graph, WeightType> pathFinder(g);

    using namespace acc;
    AccumulatorChainArray<CoupledArrays<N, T>,
                          Select< DataArg<1>, LabelArg<1>,
                                  Count, BoundingBox, RegionAnchor> > a;
    extractFeatures(src, a);

    eccentricityCentersImpl(src, g, a, pathFinder, centers);

    typename Graph::template EdgeMap<WeightType> weights(g);
    for (EdgeIt edge(g); edge != lemon::INVALID; ++edge)
    {
        const Node u(g.u(*edge)), v(g.v(*edge));
        const T label = src[u];
        if(label != src[v])
            weights[*edge] = NumericTraits<WeightType>::max();
        else
            weights[*edge] = norm(u - v);
    }
    ArrayVector<Shape> filtered_centers;
    for (T i=0; i <= a.maxRegionLabel(); ++i)
        if(get<Count>(a, i) > 0)
            filtered_centers.push_back(centers[i]);
    pathFinder.runMultiSource(weights, filtered_centers.begin(), filtered_centers.end());
    dest = pathFinder.distances();
}

template <unsigned int N, class T, class S>
inline void
eccentricityTransformOnLabels(MultiArrayView<N, T> const & src,
                              MultiArrayView<N, S> dest)
{
    ArrayVector<TinyVector<MultiArrayIndex, N> > centers;
    eccentricityTransformOnLabels(src, dest, centers);
}

//@}

} // namespace vigra


#endif // VIGRA_ECCENTRICITYTRANSFORM_HXX
