/************************************************************************/
/*                                                                      */
/*     Copyright 2003-2007 by Kasim Terzic, Christian-Dennis Rahn,      */
/*                            Philipp Schubert and Ullrich Koethe       */
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

#ifndef VIGRA_MULTI_DISTANCE_HXX
#define VIGRA_MULTI_DISTANCE_HXX

#include <vector>
#include <functional>
#include "array_vector.hxx"
#include "multi_array.hxx"
#include "accessor.hxx"
#include "numerictraits.hxx"
#include "navigator.hxx"
#include "metaprogramming.hxx"
#include "multi_pointoperators.hxx"
#include "functorexpression.hxx"

#include "multi_gridgraph.hxx"     //for boundaryGraph & boundaryMultiDistance
#include "union_find.hxx"        //for boundaryGraph & boundaryMultiDistance

namespace vigra
{

namespace detail
{

template <class Value>
struct DistParabolaStackEntry
{
    double left, center, right;
    Value apex_height;

    DistParabolaStackEntry(Value const & p, double l, double c, double r)
    : left(l), center(c), right(r), apex_height(p)
    {}
};

/********************************************************/
/*                                                      */
/*                distParabola                          */
/*                                                      */
/*  Version with sigma (parabola spread) for morphology */
/*                                                      */
/********************************************************/

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor >
void distParabola(SrcIterator is, SrcIterator iend, SrcAccessor sa,
                  DestIterator id, DestAccessor da, double sigma )
{
    // We assume that the data in the input is distance squared and treat it as such
    double w = iend - is;
    if(w <= 0)
        return;

    double sigma2 = sigma * sigma;
    double sigma22 = 2.0 * sigma2;

    typedef typename SrcAccessor::value_type SrcType;
    typedef DistParabolaStackEntry<SrcType> Influence;
    std::vector<Influence> _stack;
    _stack.push_back(Influence(sa(is), 0.0, 0.0, w));

    ++is;
    double current = 1.0;
    for(;current < w; ++is, ++current)
    {
        double intersection;

        while(true)
        {
            Influence & s = _stack.back();
            double diff = current - s.center;
            intersection = current + (sa(is) - s.apex_height - sigma2*sq(diff)) / (sigma22 * diff);

            if( intersection < s.left) // previous point has no influence
            {
                _stack.pop_back();
                if(!_stack.empty())
                    continue;  // try new top of stack without advancing current
                else
                    intersection = 0.0;
            }
            else if(intersection < s.right)
            {
                s.right = intersection;
            }
            break;
        }
        _stack.push_back(Influence(sa(is), intersection, current, w));
    }

    // Now we have the stack indicating which rows are influenced by (and therefore
    // closest to) which row. We can go through the stack and calculate the
    // distance squared for each element of the column.
    typename std::vector<Influence>::iterator it = _stack.begin();
    for(current = 0.0; current < w; ++current, ++id)
    {
        while( current >= it->right)
            ++it;
        da.set(sigma2 * sq(current - it->center) + it->apex_height, id);
    }
}

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
inline void distParabola(triple<SrcIterator, SrcIterator, SrcAccessor> src,
                         pair<DestIterator, DestAccessor> dest, double sigma)
{
    distParabola(src.first, src.second, src.third,
                 dest.first, dest.second, sigma);
}

/********************************************************/
/*                                                      */
/*        internalSeparableMultiArrayDistTmp            */
/*                                                      */
/********************************************************/

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor, class Array>
void internalSeparableMultiArrayDistTmp(
                      SrcIterator si, SrcShape const & shape, SrcAccessor src,
                      DestIterator di, DestAccessor dest, Array const & sigmas, bool invert)
{
    // Sigma is the spread of the parabolas. It determines the structuring element size
    // for ND morphology. When calculating the distance transforms, sigma is usually set to 1,
    // unless one wants to account for anisotropic pixel pitch
    enum { N =  SrcShape::static_size};

    // we need the Promote type here if we want to invert the image (dilation)
    typedef typename NumericTraits<typename DestAccessor::value_type>::RealPromote TmpType;

    // temporary array to hold the current line to enable in-place operation
    ArrayVector<TmpType> tmp( shape[0] );

    typedef MultiArrayNavigator<SrcIterator, N> SNavigator;
    typedef MultiArrayNavigator<DestIterator, N> DNavigator;


    // only operate on first dimension here
    SNavigator snav( si, shape, 0 );
    DNavigator dnav( di, shape, 0 );

    using namespace vigra::functor;

    for( ; snav.hasMore(); snav++, dnav++ )
    {
            // first copy source to temp for maximum cache efficiency
            // Invert the values if necessary. Only needed for grayscale morphology
            if(invert)
                transformLine( snav.begin(), snav.end(), src, tmp.begin(),
                               typename AccessorTraits<TmpType>::default_accessor(),
                               Param(NumericTraits<TmpType>::zero())-Arg1());
            else
                copyLine( snav.begin(), snav.end(), src, tmp.begin(),
                          typename AccessorTraits<TmpType>::default_accessor() );

            detail::distParabola( srcIterRange(tmp.begin(), tmp.end(),
                          typename AccessorTraits<TmpType>::default_const_accessor()),
                          destIter( dnav.begin(), dest ), sigmas[0] );
    }

    // operate on further dimensions
    for( int d = 1; d < N; ++d )
    {
        DNavigator dnav( di, shape, d );

        tmp.resize( shape[d] );


        for( ; dnav.hasMore(); dnav++ )
        {
             // first copy source to temp for maximum cache efficiency
             copyLine( dnav.begin(), dnav.end(), dest,
                       tmp.begin(), typename AccessorTraits<TmpType>::default_accessor() );

             detail::distParabola( srcIterRange(tmp.begin(), tmp.end(),
                           typename AccessorTraits<TmpType>::default_const_accessor()),
                           destIter( dnav.begin(), dest ), sigmas[d] );
        }
    }
    if(invert) transformMultiArray( di, shape, dest, di, dest, -Arg1());
}

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor, class Array>
inline void internalSeparableMultiArrayDistTmp( SrcIterator si, SrcShape const & shape, SrcAccessor src,
                                                DestIterator di, DestAccessor dest, Array const & sigmas)
{
    internalSeparableMultiArrayDistTmp( si, shape, src, di, dest, sigmas, false );
}

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor>
inline void internalSeparableMultiArrayDistTmp( SrcIterator si, SrcShape const & shape, SrcAccessor src,
                                                DestIterator di, DestAccessor dest)
{
    ArrayVector<double> sigmas(shape.size(), 1.0);
    internalSeparableMultiArrayDistTmp( si, shape, src, di, dest, sigmas, false );
}

} // namespace detail

/** \addtogroup DistanceTransform
*/
//@{

/********************************************************/
/*                                                      */
/*             separableMultiDistSquared                */
/*                                                      */
/********************************************************/

/** \brief Euclidean distance squared on multi-dimensional arrays.

    The algorithm is taken from Donald Bailey: "An Efficient Euclidean Distance Transform",
    Proc. IWCIA'04, Springer LNCS 3322, 2004.

    <b> Declarations:</b>

    pass arbitrary-dimensional array views:
    \code
    namespace vigra {
        // explicitly specify pixel pitch for each coordinate
        template <unsigned int N, class T1, class S1,
                                  class T2, class S2,
                  class Array>
        void
        separableMultiDistSquared(MultiArrayView<N, T1, S1> const & source,
                                  MultiArrayView<N, T2, S2> dest,
                                  bool background,
                                  Array const & pixelPitch);

        // use default pixel pitch = 1.0 for each coordinate
        template <unsigned int N, class T1, class S1,
                                  class T2, class S2>
        void
        separableMultiDistSquared(MultiArrayView<N, T1, S1> const & source,
                                  MultiArrayView<N, T2, S2> dest,
                                  bool background);
    }
    \endcode

    \deprecatedAPI{separableMultiDistSquared}
    pass \ref MultiIteratorPage "MultiIterators" and \ref DataAccessors :
    \code
    namespace vigra {
        // explicitly specify pixel pitch for each coordinate
        template <class SrcIterator, class SrcShape, class SrcAccessor,
                  class DestIterator, class DestAccessor, class Array>
        void
        separableMultiDistSquared( SrcIterator s, SrcShape const & shape, SrcAccessor src,
                                   DestIterator d, DestAccessor dest,
                                   bool background,
                                   Array const & pixelPitch);

        // use default pixel pitch = 1.0 for each coordinate
        template <class SrcIterator, class SrcShape, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        separableMultiDistSquared(SrcIterator siter, SrcShape const & shape, SrcAccessor src,
                                  DestIterator diter, DestAccessor dest,
                                  bool background);

    }
    \endcode
    use argument objects in conjunction with \ref ArgumentObjectFactories :
    \code
    namespace vigra {
        // explicitly specify pixel pitch for each coordinate
        template <class SrcIterator, class SrcShape, class SrcAccessor,
                  class DestIterator, class DestAccessor, class Array>
        void
        separableMultiDistSquared( triple<SrcIterator, SrcShape, SrcAccessor> const & source,
                                   pair<DestIterator, DestAccessor> const & dest,
                                   bool background,
                                   Array const & pixelPitch);

        // use default pixel pitch = 1.0 for each coordinate
        template <class SrcIterator, class SrcShape, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        separableMultiDistSquared(triple<SrcIterator, SrcShape, SrcAccessor> const & source,
                                  pair<DestIterator, DestAccessor> const & dest,
                                  bool background);

    }
    \endcode
    \deprecatedEnd

    This function performs a squared Euclidean squared distance transform on the given
    multi-dimensional array. Both source and destination
    arrays are represented by iterators, shape objects and accessors.
    The destination array is required to already have the correct size.

    This function expects a mask as its source, where background pixels are
    marked as zero, and non-background pixels as non-zero. If the parameter
    <i>background</i> is true, then the squared distance of all background
    pixels to the nearest object is calculated. Otherwise, the distance of all
    object pixels to the nearest background pixel is calculated.

    Optionally, one can pass an array that specifies the pixel pitch in each direction.
    This is necessary when the data have non-uniform resolution (as is common in confocal
    microscopy, for example).

    This function may work in-place, which means that <tt>siter == diter</tt> is allowed.
    A full-sized internal array is only allocated if working on the destination
    array directly would cause overflow errors (i.e. if
    <tt> NumericTraits<typename DestAccessor::value_type>::max() < N * M*M</tt>, where M is the
    size of the largest dimension of the array.

    <b> Usage:</b>

    <b>\#include</b> \<vigra/multi_distance.hxx\><br/>
    Namespace: vigra

    \code
    Shape3 shape(width, height, depth);
    MultiArray<3, unsigned char> source(shape);
    MultiArray<3, unsigned int> dest(shape);
    ...

    // Calculate Euclidean distance squared for all background pixels
    separableMultiDistSquared(source, dest, true);
    \endcode

    \see vigra::distanceTransform(), vigra::separableMultiDistance()
*/
doxygen_overloaded_function(template <...> void separableMultiDistSquared)

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor, class Array>
void separableMultiDistSquared( SrcIterator s, SrcShape const & shape, SrcAccessor src,
                                DestIterator d, DestAccessor dest, bool background,
                                Array const & pixelPitch)
{
    int N = shape.size();

    typedef typename SrcAccessor::value_type SrcType;
    typedef typename DestAccessor::value_type DestType;
    typedef typename NumericTraits<DestType>::RealPromote Real;

    SrcType zero = NumericTraits<SrcType>::zero();

    double dmax = 0.0;
    bool pixelPitchIsReal = false;
    for( int k=0; k<N; ++k)
    {
        if(int(pixelPitch[k]) != pixelPitch[k])
            pixelPitchIsReal = true;
        dmax += sq(pixelPitch[k]*shape[k]);
    }

    using namespace vigra::functor;

    if(dmax > NumericTraits<DestType>::toRealPromote(NumericTraits<DestType>::max())
       || pixelPitchIsReal) // need a temporary array to avoid overflows
    {
        // Threshold the values so all objects have infinity value in the beginning
        Real maxDist = (Real)dmax, rzero = (Real)0.0;
        MultiArray<SrcShape::static_size, Real> tmpArray(shape);
        if(background == true)
            transformMultiArray( s, shape, src,
                                 tmpArray.traverser_begin(), typename AccessorTraits<Real>::default_accessor(),
                                 ifThenElse( Arg1() == Param(zero), Param(maxDist), Param(rzero) ));
        else
            transformMultiArray( s, shape, src,
                                 tmpArray.traverser_begin(), typename AccessorTraits<Real>::default_accessor(),
                                 ifThenElse( Arg1() != Param(zero), Param(maxDist), Param(rzero) ));

        detail::internalSeparableMultiArrayDistTmp( tmpArray.traverser_begin(),
                shape, typename AccessorTraits<Real>::default_accessor(),
                tmpArray.traverser_begin(),
                typename AccessorTraits<Real>::default_accessor(), pixelPitch);

        copyMultiArray(srcMultiArrayRange(tmpArray), destIter(d, dest));
    }
    else        // work directly on the destination array
    {
        // Threshold the values so all objects have infinity value in the beginning
        DestType maxDist = DestType(std::ceil(dmax)), rzero = (DestType)0;
        if(background == true)
            transformMultiArray( s, shape, src, d, dest,
                                 ifThenElse( Arg1() == Param(zero), Param(maxDist), Param(rzero) ));
        else
            transformMultiArray( s, shape, src, d, dest,
                                 ifThenElse( Arg1() != Param(zero), Param(maxDist), Param(rzero) ));

        detail::internalSeparableMultiArrayDistTmp( d, shape, dest, d, dest, pixelPitch);
    }
}

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor>
inline
void separableMultiDistSquared( SrcIterator s, SrcShape const & shape, SrcAccessor src,
                                DestIterator d, DestAccessor dest, bool background)
{
    ArrayVector<double> pixelPitch(shape.size(), 1.0);
    separableMultiDistSquared( s, shape, src, d, dest, background, pixelPitch );
}

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor, class Array>
inline void separableMultiDistSquared( triple<SrcIterator, SrcShape, SrcAccessor> const & source,
                                       pair<DestIterator, DestAccessor> const & dest, bool background,
                                       Array const & pixelPitch)
{
    separableMultiDistSquared( source.first, source.second, source.third,
                               dest.first, dest.second, background, pixelPitch );
}

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor>
inline void separableMultiDistSquared( triple<SrcIterator, SrcShape, SrcAccessor> const & source,
                                       pair<DestIterator, DestAccessor> const & dest, bool background)
{
    separableMultiDistSquared( source.first, source.second, source.third,
                               dest.first, dest.second, background );
}

template <unsigned int N, class T1, class S1,
                          class T2, class S2,
          class Array>
inline void
separableMultiDistSquared(MultiArrayView<N, T1, S1> const & source,
                          MultiArrayView<N, T2, S2> dest, bool background,
                          Array const & pixelPitch)
{
    vigra_precondition(source.shape() == dest.shape(),
        "separableMultiDistSquared(): shape mismatch between input and output.");
    separableMultiDistSquared( srcMultiArrayRange(source),
                               destMultiArray(dest), background, pixelPitch );
}

template <unsigned int N, class T1, class S1,
                          class T2, class S2>
inline void
separableMultiDistSquared(MultiArrayView<N, T1, S1> const & source,
                          MultiArrayView<N, T2, S2> dest, bool background)
{
    vigra_precondition(source.shape() == dest.shape(),
        "separableMultiDistSquared(): shape mismatch between input and output.");
    separableMultiDistSquared( srcMultiArrayRange(source),
                               destMultiArray(dest), background );
}

/********************************************************/
/*                                                      */
/*             separableMultiDistance                   */
/*                                                      */
/********************************************************/

/** \brief Euclidean distance on multi-dimensional arrays.

    <b> Declarations:</b>

    pass arbitrary-dimensional array views:
    \code
    namespace vigra {
        // explicitly specify pixel pitch for each coordinate
        template <unsigned int N, class T1, class S1,
                  class T2, class S2, class Array>
        void
        separableMultiDistance(MultiArrayView<N, T1, S1> const & source,
                               MultiArrayView<N, T2, S2> dest,
                               bool background,
                               Array const & pixelPitch);

        // use default pixel pitch = 1.0 for each coordinate
        template <unsigned int N, class T1, class S1,
                  class T2, class S2>
        void
        separableMultiDistance(MultiArrayView<N, T1, S1> const & source,
                               MultiArrayView<N, T2, S2> dest,
                               bool background);
    }
    \endcode

    \deprecatedAPI{separableMultiDistance}
    pass \ref MultiIteratorPage "MultiIterators" and \ref DataAccessors :
    \code
    namespace vigra {
        // explicitly specify pixel pitch for each coordinate
        template <class SrcIterator, class SrcShape, class SrcAccessor,
                  class DestIterator, class DestAccessor, class Array>
        void
        separableMultiDistance( SrcIterator s, SrcShape const & shape, SrcAccessor src,
                                DestIterator d, DestAccessor dest,
                                bool background,
                                Array const & pixelPitch);

        // use default pixel pitch = 1.0 for each coordinate
        template <class SrcIterator, class SrcShape, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        separableMultiDistance(SrcIterator siter, SrcShape const & shape, SrcAccessor src,
                               DestIterator diter, DestAccessor dest,
                               bool background);

    }
    \endcode
    use argument objects in conjunction with \ref ArgumentObjectFactories :
    \code
    namespace vigra {
        // explicitly specify pixel pitch for each coordinate
        template <class SrcIterator, class SrcShape, class SrcAccessor,
                  class DestIterator, class DestAccessor, class Array>
        void
        separableMultiDistance( triple<SrcIterator, SrcShape, SrcAccessor> const & source,
                                pair<DestIterator, DestAccessor> const & dest,
                                bool background,
                                Array const & pixelPitch);

        // use default pixel pitch = 1.0 for each coordinate
        template <class SrcIterator, class SrcShape, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        separableMultiDistance(triple<SrcIterator, SrcShape, SrcAccessor> const & source,
                               pair<DestIterator, DestAccessor> const & dest,
                               bool background);

    }
    \endcode
    \deprecatedEnd

    This function performs a Euclidean distance transform on the given
    multi-dimensional array. It simply calls \ref separableMultiDistSquared()
    and takes the pixel-wise square root of the result. See \ref separableMultiDistSquared()
    for more documentation.

    <b> Usage:</b>

    <b>\#include</b> \<vigra/multi_distance.hxx\><br/>
    Namespace: vigra

    \code
    Shape3 shape(width, height, depth);
    MultiArray<3, unsigned char> source(shape);
    MultiArray<3, float> dest(shape);
    ...

    // Calculate Euclidean distance for all background pixels
    separableMultiDistance(source, dest, true);
    \endcode

    \see vigra::distanceTransform(), vigra::separableMultiDistSquared()
*/
doxygen_overloaded_function(template <...> void separableMultiDistance)

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor, class Array>
void separableMultiDistance( SrcIterator s, SrcShape const & shape, SrcAccessor src,
                             DestIterator d, DestAccessor dest, bool background,
                             Array const & pixelPitch)
{
    separableMultiDistSquared( s, shape, src, d, dest, background, pixelPitch);

    // Finally, calculate the square root of the distances
    using namespace vigra::functor;

    transformMultiArray( d, shape, dest, d, dest, sqrt(Arg1()) );
}

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor>
void separableMultiDistance( SrcIterator s, SrcShape const & shape, SrcAccessor src,
                             DestIterator d, DestAccessor dest, bool background)
{
    separableMultiDistSquared( s, shape, src, d, dest, background);

    // Finally, calculate the square root of the distances
    using namespace vigra::functor;

    transformMultiArray( d, shape, dest, d, dest, sqrt(Arg1()) );
}

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor, class Array>
inline void separableMultiDistance( triple<SrcIterator, SrcShape, SrcAccessor> const & source,
                                    pair<DestIterator, DestAccessor> const & dest, bool background,
                                    Array const & pixelPitch)
{
    separableMultiDistance( source.first, source.second, source.third,
                            dest.first, dest.second, background, pixelPitch );
}

template <class SrcIterator, class SrcShape, class SrcAccessor,
          class DestIterator, class DestAccessor>
inline void separableMultiDistance( triple<SrcIterator, SrcShape, SrcAccessor> const & source,
                                    pair<DestIterator, DestAccessor> const & dest, bool background)
{
    separableMultiDistance( source.first, source.second, source.third,
                            dest.first, dest.second, background );
}

template <unsigned int N, class T1, class S1,
          class T2, class S2, class Array>
inline void
separableMultiDistance(MultiArrayView<N, T1, S1> const & source,
                       MultiArrayView<N, T2, S2> dest,
                       bool background,
                       Array const & pixelPitch)
{
    vigra_precondition(source.shape() == dest.shape(),
        "separableMultiDistance(): shape mismatch between input and output.");
    separableMultiDistance( srcMultiArrayRange(source),
                            destMultiArray(dest), background, pixelPitch );
}

template <unsigned int N, class T1, class S1,
          class T2, class S2>
inline void
separableMultiDistance(MultiArrayView<N, T1, S1> const & source,
                       MultiArrayView<N, T2, S2> dest,
                       bool background)
{
    vigra_precondition(source.shape() == dest.shape(),
        "separableMultiDistance(): shape mismatch between input and output.");
    separableMultiDistance( srcMultiArrayRange(source),
                            destMultiArray(dest), background );
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% BoundaryDistanceTransform %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//rewrite labeled data and work with separableMultiDist
namespace lemon_graph {

template <class Graph, class T1Map, class T2Map>
void
markRegionBoundaries(Graph const & g,
                     T1Map const & labels,
                     T2Map & out)
{
    typedef typename Graph::NodeIt        graph_scanner;
    typedef typename Graph::OutBackArcIt  neighbor_iterator;

    //find faces
    for (graph_scanner node(g); node != INVALID; ++node)
    {
        typename T1Map::value_type center = labels[*node];

        for (neighbor_iterator arc(g, node); arc != INVALID; ++arc)
        {
            // set adjacent nodes with different labels to 1
            if(center != labels[g.target(*arc)])
            {
                out[*node] = 1;
                out[g.target(*arc)] = 1;
            }
        }
    }
}

} //-- namspace lemon_graph

doxygen_overloaded_function(template <...> unsigned int markRegionBoundaries)

template <unsigned int N, class T1, class S1,
                          class T2, class S2>
inline void
markRegionBoundaries(MultiArrayView<N, T1, S1> const & labels,
                     MultiArrayView<N, T2, S2> out,
                     NeighborhoodType neighborhood=DirectNeighborhood)
{
    vigra_precondition(labels.shape() == out.shape(),
        "markRegionBoundaries(): shape mismatch between input and output.");

    GridGraph<N, undirected_tag> graph(labels.shape(), neighborhood);

    lemon_graph::markRegionBoundaries(graph, labels, out);
}

//MultiDistance which works directly on labeled data

namespace detail
{

/********************************************************/
/*                                                      */
/*                boundaryDistParabola                  */
/*                                                      */
/********************************************************/

template <class DestIterator, class LabelIterator>
void
boundaryDistParabola(DestIterator is, DestIterator iend,
                     LabelIterator ilabels,
                     double dmax,
                     bool array_border_is_active=false)
{
    // We assume that the data in the input is distance squared and treat it as such
    double w = iend - is;
    if(w <= 0)
        return;

    DestIterator id = is;
    typedef typename LabelIterator::value_type LabelType;
    typedef typename DestIterator::value_type DestType;
    typedef detail::DistParabolaStackEntry<DestType> Influence;
    typedef std::vector<Influence> Stack;

    double apex_height = array_border_is_active
                             ? 0.0
                             : dmax;
    Stack _stack(1, Influence(apex_height, 0.0, -1.0, w));
    LabelType current_label = *ilabels;
    for(double begin = 0.0, current = 0.0; current <= w; ++ilabels, ++is, ++current)
    {
        apex_height = (current < w)
                          ? (current_label == *ilabels)
                               ? *is
                               : 0.0
                          : array_border_is_active
                                ? 0.0
                                : dmax;
        while(true)
        {
            Influence & s = _stack.back();
            double diff = current - s.center;
            double intersection = current + (apex_height - s.apex_height - sq(diff)) / (2.0 * diff);

            if(intersection < s.left) // previous parabola has no influence
            {
                _stack.pop_back();
                if(_stack.empty())
                    intersection = begin; // new parabola is valid for entire present segment
                else
                    continue;  // try new top of stack without advancing to next pixel
            }
            else if(intersection < s.right)
            {
                s.right = intersection;
            }
            if(intersection < w)
                _stack.push_back(Influence(apex_height, intersection, current, w));
            if(current < w && current_label == *ilabels)
                break; // finished present pixel, advance to next one

            // label changed => finalize the current segment
            typename Stack::iterator it = _stack.begin();
            for(double c = begin; c < current; ++c, ++id)
            {
                while(c >= it->right)
                    ++it;
                *id = sq(c - it->center) + it->apex_height;
            }
            if(current == w)
                break;  // stop when this was the last segment

            // initialize the new segment
            begin = current;
            current_label = *ilabels;
            apex_height = *is;
            Stack(1, Influence(0.0, begin-1.0, begin-1.0, w)).swap(_stack);
            // don't advance to next pixel here, because the present pixel must also
            // be analysed in the context of the new segment
        }
    }
}

/********************************************************/
/*                                                      */
/*           internalBoundaryMultiArrayDist             */
/*                                                      */
/********************************************************/

template <unsigned int N, class T1, class S1,
                          class T2, class S2>
void
internalBoundaryMultiArrayDist(
                      MultiArrayView<N, T1, S1> const & labels,
                      MultiArrayView<N, T2, S2> dest,
                      double dmax, bool array_border_is_active=false)
{
    typedef typename MultiArrayView<N, T1, S1>::const_traverser LabelIterator;
    typedef typename MultiArrayView<N, T2, S2>::traverser DestIterator;
    typedef MultiArrayNavigator<LabelIterator, N> LabelNavigator;
    typedef MultiArrayNavigator<DestIterator, N> DNavigator;

    dest = dmax;
    for( unsigned d = 0; d < N; ++d )
    {
        LabelNavigator lnav( labels.traverser_begin(), labels.shape(), d );
        DNavigator dnav( dest.traverser_begin(), dest.shape(), d );

        for( ; dnav.hasMore(); dnav++, lnav++ )
        {
            boundaryDistParabola(dnav.begin(), dnav.end(),
                                 lnav.begin(),
                                 dmax, array_border_is_active);
        }
    }
}

} // namespace detail

    /** \brief Specify which boundary is used for boundaryMultiDistance().

    */
enum BoundaryDistanceTag {
    OuterBoundary,      ///< Pixels just outside of each region
    InterpixelBoundary, ///< Half-integer points between pixels of different labels
    InnerBoundary       ///< Pixels just inside of each region
};

/********************************************************/
/*                                                      */
/*             boundaryMultiDistance                    */
/*                                                      */
/********************************************************/

/** \brief Euclidean distance to the implicit boundaries of a multi-dimensional label array.


    <b> Declarations:</b>

    pass arbitrary-dimensional array views:
    \code
    namespace vigra {
        template <unsigned int N, class T1, class S1,
                  class T2, class S2>
        void
        boundaryMultiDistance(MultiArrayView<N, T1, S1> const & labels,
                              MultiArrayView<N, T2, S2> dest,
                              bool array_border_is_active=false,
                              BoundaryDistanceTag boundary=InterpixelBoundary);
    }
    \endcode

    This function computes the distance transform of a labeled image <i>simultaneously</i>
    for all regions. Depending on the requested type of \a boundary, three modes
    are supported:
    <ul>
    <li><tt>OuterBoundary</tt>: In each region, compute the distance to the nearest pixel not
               belonging to that regions. This is the same as if a normal distance transform
               where applied to a binary image containing just this region.</li>
    <li><tt>InterpixelBoundary</tt> (default): Like <tt>OuterBoundary</tt>, but shift the distance
               to the interpixel boundary by subtractiong 1/2. This make the distences consistent
               accross boundaries.</li>
    <li><tt>InnerBoundary</tt>: In each region, compute the distance to the nearest pixel in the
               region which is adjacent to the boundary. </li>
    </ul>
    If <tt>array_border_is_active=true</tt>, the
    outer border of the array (i.e. the interpixel boundary between the array
    and the infinite region) is also used. Otherwise (the default), regions
    touching the array border are treated as if they extended to infinity.

    <b> Usage:</b>

    <b>\#include</b> \<vigra/multi_distance.hxx\><br/>
    Namespace: vigra

    \code
    Shape3 shape(width, height, depth);
    MultiArray<3, unsigned char> source(shape);
    MultiArray<3, UInt32> labels(shape);
    MultiArray<3, float> dest(shape);
    ...

    // find regions (interpixel boundaries are implied)
    labelMultiArray(source, labels);

    // Calculate Euclidean distance to interpixel boundary for all pixels
    boundaryMultiDistance(labels, dest);
    \endcode

    \see vigra::distanceTransform(), vigra::separableMultiDistance()
*/
doxygen_overloaded_function(template <...> void boundaryMultiDistance)

template <unsigned int N, class T1, class S1,
                          class T2, class S2>
void
boundaryMultiDistance(MultiArrayView<N, T1, S1> const & labels,
                      MultiArrayView<N, T2, S2> dest,
                      bool array_border_is_active=false,
                      BoundaryDistanceTag boundary=InterpixelBoundary)
{
    vigra_precondition(labels.shape() == dest.shape(),
        "boundaryMultiDistance(): shape mismatch between input and output.");

    using namespace vigra::functor;

    if(boundary == InnerBoundary)
    {
        MultiArray<N, unsigned char> boundaries(labels.shape());

        markRegionBoundaries(labels, boundaries, IndirectNeighborhood);
        if(array_border_is_active)
            initMultiArrayBorder(boundaries, 1, 1);
        separableMultiDistance(boundaries, dest, true);
    }
    else
    {
        T2 offset = 0.0;

        if(boundary == InterpixelBoundary)
        {
            vigra_precondition(!NumericTraits<T2>::isIntegral::value,
                "boundaryMultiDistance(..., InterpixelBoundary): output pixel type must be float or double.");
            offset = T2(0.5);
        }
        double dmax = squaredNorm(labels.shape()) + N;
        if(dmax > double(NumericTraits<T2>::max()))
        {
            // need a temporary array to avoid overflows
            typedef typename NumericTraits<T2>::RealPromote Real;
            MultiArray<N, Real> tmpArray(labels.shape());
            detail::internalBoundaryMultiArrayDist(labels, tmpArray,
                                                      dmax, array_border_is_active);
            transformMultiArray(tmpArray, dest, sqrt(Arg1()) - Param(offset) );
        }
        else
        {
            // can work directly on the destination array
            detail::internalBoundaryMultiArrayDist(labels, dest, dmax, array_border_is_active);
            transformMultiArray(dest, dest, sqrt(Arg1()) - Param(offset) );
        }
    }
}

//@}

} //-- namespace vigra


#endif        //-- VIGRA_MULTI_DISTANCE_HXX
