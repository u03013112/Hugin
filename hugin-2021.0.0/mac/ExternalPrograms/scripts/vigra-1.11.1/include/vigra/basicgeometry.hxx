/************************************************************************/
/*                                                                      */
/*               Copyright 1998-2002 by Ullrich Koethe                  */
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

#ifndef VIGRA_BASICGEOMETRY_HXX
#define VIGRA_BASICGEOMETRY_HXX

#include "error.hxx"
#include "stdimage.hxx"
#include "copyimage.hxx"
#include "multi_shape.hxx"

#include <cmath>

namespace vigra {

/** \addtogroup GeometricTransformations
*/
//@{

/********************************************************/
/*                                                      */
/*                      rotateImage                     */
/*                                                      */
/********************************************************/

/** \brief Rotate an image by a multiple of 90 degrees or by an arbitrary angle.

    If you specify the angle as an integer which is a multiple of 90 degrees, rotateImage()
    just copies the pixels in the appropriate new order. It expects the destination image to
    have the correct shape for the desired rotation. That is, when the rotation is a multiple
    of 180 degrees, source and destination must have the same shape, otherwise destination
    must have the transposed shape of the source.

    If you want to rotate by an arbitrary angle and around an arbitrary center point,
    you must specify the source image as a \ref vigra::SplineImageView, which is used for
    interpolation at the required subpixel positions. If no center point is provided, the image
    center is used by default. The destination image must have the same size
    as the source SplineImageView.

    Positive angles refer to counter-clockwise rotation, negative ones to clockwise rotation.
    All angles must be given in degrees.

    <b> Declarations:</b>

    pass 2D array views:
    \code
    namespace vigra {
        // rotate by a multiple of 90 degrees
        template <class T1, class S1,
                  class T2, class S2>
        void
        rotateImage(MultiArrayView<2, T1, S1> const & src,
                    MultiArrayView<2, T2, S2> dest,
                    int rotation);

        // rotate by an arbitrary angle around the given center point
        template <int ORDER, class T,
                  class T2, class S2>
        void
        rotateImage(SplineImageView<ORDER, T> const & src,
                    MultiArrayView<2, T2, S2> dest,
                    double angleInDegree,
                    TinyVector<double, 2> const & center = (src.shape() - Shape2(1)) / 2.0);
    }
    \endcode

    \deprecatedAPI{rotateImage}
    pass \ref ImageIterators and \ref DataAccessors :
    \code
    namespace vigra {
        // rotate by a multiple of 90 degrees
        template <class SrcIterator, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        rotateImage(SrcIterator is, SrcIterator end, SrcAccessor as,
                    DestIterator id, DestAccessor ad, int rotation);

        // rotate by an arbitrary angle around the given center point
        template <int ORDER, class T,
                  class DestIterator, class DestAccessor>
        void rotateImage(SplineImageView<ORDER, T> const & src,
                         DestIterator id, DestAccessor dest,
                         double angleInDegree, TinyVector<double, 2> const & center = (src.shape() - Shape2(1)) / 2.0);
    }
    \endcode
    use argument objects in conjunction with \ref ArgumentObjectFactories :
    \code
    namespace vigra {
        // rotate by a multiple of 90 degrees
        template <class SrcImageIterator, class SrcAccessor,
              class DestImageIterator, class DestAccessor>
        void
        rotateImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                    pair<DestImageIterator, DestAccessor> dest, int rotation);

        // rotate by an arbitrary angle around the given center point
        template <int ORDER, class T,
                  class DestIterator, class DestAccessor>
        void
        rotateImage(SplineImageView<ORDER, T> const & src,
                    pair<DestImageIterator, DestAccessor> dest,
                    double angleInDegree, TinyVector<double, 2> const & center = (src.shape() - Shape2(1)) / 2.0);
    }
    \endcode
    \deprecatedEnd

    <b> Usage:</b>

    <b>\#include</b> \<vigra/basicgeometry.hxx\><br>
    Namespace: vigra

    \code
    // rotate counter-clockwise by 90 degrees (no interpolation required)
    MultiArray<2, float> src(width, height),
                         dest(height, width); // note that width and height are exchanged
    ... // fill src
    rotateImage(src, dest, 90);

    // rotate clockwise by 38.5 degrees, using a SplieImageView for cubic interpolation
    SplineImageView<3, float> spline(srcImageRange(src));
    MultiArray<2, float>  dest2(src.shape());

    vigra::rotateImage(spline, dest2, -38.5);
    \endcode

    \deprecatedUsage{rotateImage}
    \code
    // rotate counter-clockwise by 90 degrees (no interpolation required)
    BImage src(width, height),
           dest(height, width);    // note that width and height are exchanged
    ... // fill src

    rotateImage(srcImageRange(src), destImage(dest), 90);

    // rotate clockwise by 38.5 degrees, using a SplieImageView for cubic interpolation
    SplineImageView<3, float> spline(srcImageRange(src));
    FImage dest2(width, height);

    rotateImage(spline, destImage(dest), -38.5);
    \endcode
    <b> Required Interface:</b>
    \code
    SrcImageIterator src_upperleft, src_lowerright;
    DestImageIterator dest_upperleft;

    SrcAccessor src_accessor;

    dest_accessor.set(src_accessor(src_upperleft), dest_upperleft);
    \endcode
    \deprecatedEnd

    <b> Preconditions:</b>
    \code
    src.shape(0) > 1  &&  src.shape(1) > 1
    \endcode
*/
doxygen_overloaded_function(template <...> void rotateImage)

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
void rotateImage(SrcIterator is, SrcIterator end, SrcAccessor as,
                           DestIterator id, DestAccessor ad, int rotation)
{
    int x, y;
    int ws = end.x - is.x;
    int hs = end.y - is.y;

    vigra_precondition(rotation % 90 == 0,
                "rotateImage(): "
                "This function rotates images only about multiples of 90 degree");

    rotation = rotation%360;
    if (rotation < 0)
        rotation += 360;

    switch(rotation)
    {
        case 0:
            copyImage(is, end, as, id, ad);
            break;
        case 90:
            is.x += (ws-1);
            for(x=0; x != ws; x++, is.x--, id.y++)
            {
                typename SrcIterator::column_iterator cs = is.columnIterator();
                typename DestIterator::row_iterator rd = id.rowIterator();
                for(y=0; y != hs; y++, cs++, rd++)
                {
                    ad.set(as(cs), rd);
                }

            }
            break;

        case 180:
            end.x--;
            end.y--;
            for(x=0; x != ws; x++, end.x--, id.x++)
            {
                typename SrcIterator::column_iterator cs = end.columnIterator();
                typename DestIterator::column_iterator cd = id.columnIterator();
                for(y=0; y != hs; y++, cs--, cd++)
                {
                    ad.set(as(cs), cd);
                }

            }
            break;

        case 270:
            is.y += (hs-1);
            for(x=0; x != ws; x++, is.x++, id.y++)
            {
                typename SrcIterator::column_iterator cs = is.columnIterator();
                typename DestIterator::row_iterator rd = id.rowIterator();
                for(y=0; y != hs; y++, cs--, rd++)
                {
                    ad.set(as(cs), rd);
                }

            }
            break;
        default: //not needful, because of the exception handig in if-statement
            vigra_fail("internal error");
    }
}

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline void
rotateImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
              pair<DestImageIterator, DestAccessor> dest, int rotation)
{
    rotateImage(src.first, src.second, src.third, dest.first, dest.second, rotation);
}

template <class T1, class S1,
          class T2, class S2>
inline void
rotateImage(MultiArrayView<2, T1, S1> const & src,
            MultiArrayView<2, T2, S2> dest,
            int rotation)
{
    if(rotation % 180 == 0)
        vigra_precondition(src.shape() == dest.shape(),
            "rotateImage(): shape mismatch between input and output.");
    else
        vigra_precondition(src.shape() == reverse(dest.shape()),
            "rotateImage(): shape mismatch between input and output.");
    rotateImage(srcImageRange(src), destImage(dest), rotation);
}

/********************************************************/
/*                                                      */
/*                     reflectImage                     */
/*                                                      */
/********************************************************/

enum Reflect {horizontal = 1, vertical = 2};

inline
Reflect operator|(Reflect l, Reflect r)
{
    return Reflect((unsigned int)l | (unsigned int)r);
}

/** \brief Reflect image horizontally or vertically.

    The reflection direction refers to the reflection axis, i.e.
    horizontal reflection turns the image upside down, vertical reflection
    changes left for right. The directions are selected by the enum values
    <tt>vigra::horizontal</tt> and <tt>vigra::vertical</tt>. The two directions
    can also be "or"ed together to perform both reflections simultaneously
    (see example below) -- this is the same as a 180 degree rotation.

    <b> Declarations:</b>

    pass 2D array views:
    \code
    namespace vigra {
        template <class T1, class S1,
                  class T2, class S2>
        void
        reflectImage(MultiArrayView<2, T1, S1> const & src,
                     MultiArrayView<2, T2, S2> dest, Reflect reflect);
    }
    \endcode

    \deprecatedAPI{reflectImage}
    pass \ref ImageIterators and \ref DataAccessors :
    \code
    namespace vigra {
        template <class SrcIterator, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        reflectImage(SrcIterator is, SrcIterator end, SrcAccessor as,
                     DestIterator id, DestAccessor ad, Reflect axis);
    }
    \endcode
    use argument objects in conjunction with \ref ArgumentObjectFactories :
    \code
    namespace vigra {
        template <class SrcImageIterator, class SrcAccessor,
              class DestImageIterator, class DestAccessor>
        void
        reflectImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                     pair<DestImageIterator, DestAccessor> dest, Reflect axis);
    }
    \endcode
    \deprecatedEnd

    <b> Usage:</b>

    <b>\#include</b> \<vigra/basicgeometry.hxx\><br>
    Namespace: vigra

    \code
    MultiArray<2, float> src(width, height),
                         dest(width, height);
    ... // fill src
    // reflect about both dimensions
    vigra::reflectImage(src, dest, vigra::horizontal | vigra::vertical);

    \endcode

    \deprecatedUsage{reflectImage}
    \code
    BImage src(width, height),
           dest(width, height);
    ... // fill src

    vigra::reflectImage(srcImageRange(src), destImage(dest), vigra::horizontal | vigra::vertical);
    \endcode
    <b> Required Interface:</b>
    \code
    SrcImageIterator src_upperleft, src_lowerright;
    DestImageIterator dest_upperleft;

    SrcAccessor src_accessor;

    dest_accessor.set(src_accessor(src_upperleft), dest_upperleft);
    \endcode
    \deprecatedEnd

    <b> Preconditions:</b>
    \code
    src.shape(0) > 1  &&  src.shape(1) > 1
    \endcode
*/
doxygen_overloaded_function(template <...> void reflectImage)

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
void reflectImage(SrcIterator is, SrcIterator end, SrcAccessor as,
                  DestIterator id, DestAccessor ad, Reflect reflect)
{

    int ws = end.x - is.x;
    int hs = end.y - is.y;

    int x, y;

    if(reflect == horizontal)
    {//flipImage
        is.y += (hs-1);
        for(x=0; x<ws; ++x, ++is.x, ++id.x)
        {
            typename SrcIterator::column_iterator  cs = is.columnIterator();
            typename DestIterator::column_iterator cd = id.columnIterator();
            for(y=0; y!=hs;y++, cs--, cd++)
            {
                ad.set(as(cs), cd);
            }
        }
    }
    else if(reflect == vertical)
    {//flopImage
        is.x += (ws-1);
        for(x=0; x < ws; ++x, --is.x, ++id.x)
        {

            typename SrcIterator::column_iterator cs = is.columnIterator();
            typename DestIterator::column_iterator cd = id.columnIterator();
            for(y=0; y!=hs;y++, cs++, cd++)
            {
                ad.set(as(cs), cd);
            }
        }
    }
    else if(reflect == (horizontal | vertical))
    {//flipFlopImage   //???
        end.x--;
        end.y--;
        for(x=0; x != ws; x++, end.x--, id.x++)
        {
            typename SrcIterator::column_iterator cs = end.columnIterator();
            typename DestIterator::column_iterator cd = id.columnIterator();
            for(y=0; y != hs; y++, cs--, cd++)
            {
                ad.set(as(cs), cd);
            }
        }
    }
    else
        vigra_fail("reflectImage(): "
                   "This function reflects horizontal or vertical,"
                   "   'and' is included");
}

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline void
reflectImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
              pair<DestImageIterator, DestAccessor> dest, Reflect reflect)
{
    reflectImage(src.first, src.second, src.third, dest.first, dest.second, reflect);
}

template <class T1, class S1,
          class T2, class S2>
inline void
reflectImage(MultiArrayView<2, T1, S1> const & src,
             MultiArrayView<2, T2, S2> dest, Reflect reflect)
{
    vigra_precondition(src.shape() == dest.shape(),
        "reflectImage(): shape mismatch between input and output.");
    reflectImage(srcImageRange(src), destImage(dest), reflect);
}

/********************************************************/
/*                                                      */
/*                    transposeImage                   */
/*                                                      */
/********************************************************/

// names clash with sys/types.h on Mac OS / Darwin, see docs below
enum Transpose{major = 1, minor = 2};

/** \brief Transpose an image over the major or minor diagonal.

    The transposition direction refers to the axis, i.e.
    major transposition turns the upper right corner into the lower left one,
    whereas minor transposition changes the upper left corner into the lower right one.
    The directions are selected by the enum values
    <tt>vigra::major</tt> and <tt>vigra::minor</tt>. The two directions
    can also be "or"ed together to perform both reflections simultaneously
    (see example below) -- this is the same as a 180 degree rotation.
    (Caution: When doing multi-platform development, you should be
    aware that some <sys/types.h> define major/minor, too.  Do not omit
    the vigra namespace prefix.)

    Note that a similar effect can be chieved by MultiArrayView::transpose(). However,
    the latter can only transpose about the major diagonal, and it doesn't rearrange the data
    -  it just creates a view with transposed axis ordering. It depends on the context
    which function is more appropriate.

    <b> Declarations:</b>

    pass 2D array views:
    \code
    namespace vigra {
        template <class T1, class S1,
                  class T2, class S2>
        void
        transposeImage(MultiArrayView<2, T1, S1> const & src,
                       MultiArrayView<2, T2, S2> dest, Transpose axis);
    }
    \endcode

    \deprecatedAPI{transposeImage}
    pass \ref ImageIterators and \ref DataAccessors :
    \code
    namespace vigra {
        template <class SrcIterator, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        transposeImage(SrcIterator is, SrcIterator end, SrcAccessor as,
                       DestIterator id, DestAccessor ad, Transpose axis);
    }
    \endcode
    use argument objects in conjunction with \ref ArgumentObjectFactories :
    \code
    namespace vigra {
        template <class SrcImageIterator, class SrcAccessor,
              class DestImageIterator, class DestAccessor>
        void
        transposeImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                       pair<DestImageIterator, DestAccessor> dest, Transpose axis);
    }
    \endcode
    \deprecatedEnd

    <b> Usage:</b>

    <b>\#include</b> \<vigra/basicgeometry.hxx\><br>
    Namespace: vigra

    \code
    MultiArray<2, float> src(width, height),
                         dest(height, width);   // note that dimensions are transposed
    ... // fill src

    // transpose about the major diagonal
    vigra::transposeImage(src, dest, vigra::major);

    // this produces the same data as transposing the view
    assert(dest == src.transpose());

    // transposition about the minor diagonal has no correspondence in MultiArrayView
    vigra::transposeImage(src, dest, vigra::minor);
    \endcode

    \deprecatedUsage{transposeImage}
    \code
    BImage src(width, height),
           dest(width, height);
    ... // fill src

    // transpose about both diagonals simultaneously
    vigra::transposeImage(srcImageRange(src), destImage(dest), vigra::major | vigra::minor);
    \endcode
    <b> Required Interface:</b>
    \code
    SrcImageIterator src_upperleft, src_lowerright;
    DestImageIterator dest_upperleft;

    SrcAccessor src_accessor;

    dest_accessor.set(src_accessor(src_upperleft), dest_upperleft);
    \endcode
    \deprecatedEnd

    <b> Preconditions:</b>
    \code
    src.shape(0) > 1  &&  src.shape(1) > 1
    \endcode
*/
doxygen_overloaded_function(template <...> void transposeImage)

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
void transposeImage(SrcIterator is, SrcIterator end, SrcAccessor as,
                    DestIterator id, DestAccessor ad, Transpose transpose)
{
    int ws = end.x - is.x;
    int hs = end.y - is.y;

    int x, y;

    if(transpose == major)
    {//Die Funktion spiegelt das Bild um (0,0) (1,1) Diagonale
        for(x=0; x != ws; x++, is.x++, id.y++)
        {

            typename SrcIterator::column_iterator cs = is.columnIterator();
            typename DestIterator::row_iterator rd = id.rowIterator();
            for(y=0; y != hs; y++, cs++, rd++)
            {
                ad.set(as(cs), rd);
            }
        }
    }
    else if(transpose == minor)
    {//Die Funktion spiegelt das Bild (1,0) (0,1) Diagonale
        end.x--;
        end.y--;
        for(x=0; x != ws; x++, --end.x, ++id.y)
        {

            typename SrcIterator::column_iterator cs = end.columnIterator();
            typename DestIterator::row_iterator rd = id.rowIterator();
            for(y=0; y != hs; y++, --cs, ++rd)
            {
                ad.set(as(cs), rd);
            }
        }
    }
    else if(transpose == (major | minor))
    {//flipFlopImage  //???
        end.x--;
        end.y--;
        for(x=0; x != ws; x++, end.x--, id.x++)
        {
            typename SrcIterator::column_iterator cs = end.columnIterator();
            typename DestIterator::column_iterator cd = id.columnIterator();
            for(y=0; y != hs; y++, cs--, cd++)
            {
                ad.set(as(cs), cd);
            }
        }

    }
    else
        vigra_fail("transposeImage(): "
                   "This function transposes major or minor,"
                   "   'and' is included");

}

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline void
transposeImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
               pair<DestImageIterator, DestAccessor> dest, Transpose transpose)
{
    transposeImage(src.first, src.second, src.third, dest.first, dest.second, transpose);
}

template <class T1, class S1,
          class T2, class S2>
inline void
transposeImage(MultiArrayView<2, T1, S1> const & src,
               MultiArrayView<2, T2, S2> dest, Transpose transpose)
{
    vigra_precondition(src.shape() == reverse(dest.shape()),
        "transposeImage(): shape mismatch between input and output.");
    transposeImage(srcImageRange(src), destImage(dest), transpose);
}

/********************************************************/
/*                                                      */
/*                        resampleLine                  */
/*                                                      */
/********************************************************/

/*
* Vergroessert eine Linie um einen Faktor.
* Ist z.B. der Faktor = 4 so werden in der
* neuen Linie(Destination) jedes Pixel genau 4 mal
* vorkommen, also es findet auch keine Glaetung
* statt (NoInterpolation). Als Parameter sollen
* der Anfangs-, der Enditerator und der Accessor
* der Ausgangslinie (Source line), der Anfangsiterator
* und Accessor der Ziellinie (destination line) und
* anschliessend der Faktor um den die Linie (Zeile)
* vergroessert bzw. verkleinert werden soll.
*/
template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
void resampleLine(SrcIterator src_iter, SrcIterator src_iter_end, SrcAccessor src_acc,
                  DestIterator dest_iter, DestAccessor dest_acc, double factor)
{
    // The width of the src line.
    int src_width = src_iter_end - src_iter;

    vigra_precondition(src_width > 0,
                       "resampleLine(): input image too small.");
    vigra_precondition(factor > 0.0,
                       "resampleLine(): factor must be positive.");

    if (factor >= 1.0)
    {
        int int_factor = (int)factor;
        double dx = factor - int_factor;
        double saver = dx;
        for ( ; src_iter != src_iter_end ; ++src_iter, saver += dx)
        {
            if (saver >= 1.0)
            {
                saver = saver - (int)saver;
                dest_acc.set(src_acc(src_iter), dest_iter);
                ++dest_iter;
            }
            for(int i = 0 ; i < int_factor ; i++, ++dest_iter)
            {
                dest_acc.set(src_acc(src_iter), dest_iter);
            }
        }
    }
    else
    {
        DestIterator dest_end = dest_iter + (int)VIGRA_CSTD::ceil(src_width*factor);
        factor = 1.0/factor;
        int int_factor = (int)factor;
        double dx = factor - int_factor;
        double saver = dx;
        src_iter_end -= 1;
        for ( ; src_iter != src_iter_end && dest_iter != dest_end ;
              ++dest_iter, src_iter += int_factor, saver += dx)
        {
            if (saver >= 1.0)
            {
                saver = saver - (int)saver;
                ++src_iter;
            }
            dest_acc.set(src_acc(src_iter), dest_iter);
        }
        if (dest_iter != dest_end)
        {
            dest_acc.set(src_acc(src_iter_end), dest_iter);
        }
    }
}

inline int sizeForResamplingFactor(int oldsize, double factor)
{
    return (factor < 1.0)
        ? (int)VIGRA_CSTD::ceil(oldsize * factor)
        : (int)(oldsize * factor);
}


/********************************************************/
/*                                                      */
/*                     resampleImage                    */
/*                                                      */
/********************************************************/

/** \brief Resample image by a given factor.

    This algorithm is very fast and does not require any arithmetic on the pixel types.
    The input image must have a size of at
    least 2x2. Destiniation pixels are directly copied from the appropriate
    source pixels. The size of the result image is the product of <tt>factor</tt>
    and the original size, where we round up if <tt>factor < 1.0</tt> and down otherwise.
    This size calculation is the main difference to the convention used in the similar
    function \ref resizeImageNoInterpolation():
    there, the result size is calculated as <tt>n*(old_width-1)+1</tt> and
    <tt>n*(old_height-1)+1</tt>. This is because \ref resizeImageNoInterpolation()
    does not replicate the last pixel of every row/column in order to make it compatible
    with the other functions of the <tt>resizeImage...</tt> family.

    The function can be called with different resampling factors for x and y, or
    with a single factor to be used for both directions.

    It should also be noted that resampleImage() is implemented so that an enlargement followed
    by the corresponding shrinking reproduces the original image.

    <b> Declarations:</b>

    pass 2D array views:
    \code
    namespace vigra {
        template <class T1, class S1,
                  class T2, class S2>
        void
        resampleImage(MultiArrayView<2, T1, S1> const & src,
                      MultiArrayView<2, T2, S2> dest, double factor);

        template <class T1, class S1,
                  class T2, class S2>
        void
        resampleImage(MultiArrayView<2, T1, S1> const & src,
                      MultiArrayView<2, T2, S2> dest, double xfactor, double yfactor);
    }
    \endcode

    \deprecatedAPI{resampleImage}
    pass \ref ImageIterators and \ref DataAccessors :
    \code
    namespace vigra {
        template <class SrcIterator, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        resampleImage(SrcIterator is, SrcIterator iend, SrcAccessor sa,
                      DestIterator id, DestAccessor ad, double factor);

        template <class SrcIterator, class SrcAccessor,
                  class DestIterator, class DestAccessor>
        void
        resampleImage(SrcIterator is, SrcIterator iend, SrcAccessor sa,
                      DestIterator id, DestAccessor ad, double xfactor, double yfactor);
    }
    \endcode
    use argument objects in conjunction with \ref ArgumentObjectFactories :
    \code
    namespace vigra {
        template <class SrcImageIterator, class SrcAccessor,
              class DestImageIterator, class DestAccessor>
        void
        resampleImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                      pair<DestImageIterator, DestAccessor> dest, double factor);

        template <class SrcImageIterator, class SrcAccessor,
              class DestImageIterator, class DestAccessor>
        void
        resampleImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                      pair<DestImageIterator, DestAccessor> dest, double xfactor, double yfactor);
    }
    \endcode
    \deprecatedEnd

    <b> Usage:</b>

    <b>\#include</b> \<vigra/basicgeometry.hxx\><br>
    Namespace: vigra

    \code
    double factor = 2.0;
    MultiArray<2, float> src(width, height),
                         dest((int)(factor*width), (int)(factor*height));   // enlarge image by factor
    ... // fill src

    resampleImage(src, dest, factor);
    \endcode

    \deprecatedUsage{resampleImage}
    \code
    // use old API
    vigra::resampleImage(srcImageRange(src), destImage(dest), factor);
    \endcode
    <b> Required Interface:</b>
    \code
    SrcImageIterator src_upperleft, src_lowerright;
    DestImageIterator dest_upperleft;

    SrcAccessor src_accessor;

    dest_accessor.set(src_accessor(src_upperleft), dest_upperleft);
    \endcode
    \deprecatedEnd

    <b> Preconditions:</b>
    \code
    src.shape(0) > 1  &&  src.shape(1) > 1
    \endcode
*/
doxygen_overloaded_function(template <...> void resampleImage)

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
void
resampleImage(SrcIterator is, SrcIterator iend, SrcAccessor sa,
              DestIterator id, DestAccessor ad, double xfactor, double yfactor)
{
    int width_old = iend.x - is.x;
    int height_old = iend.y - is.y;

    //Bei Verkleinerung muss das dest-Bild ceiling(src*factor), da z.B.
    //aus 6x6 grossem Bild wird eins 18x18 grosses gemacht bei Vergroesserungsfaktor 3.1
    //umgekehrt damit wir vom 18x18 zu 6x6 (und nicht 5x5) bei Vergroesserung von 1/3.1
    //muss das kleinste Integer das groesser als 18/3.1 ist genommen werden.
    int height_new = sizeForResamplingFactor(height_old, yfactor);
    int width_new = sizeForResamplingFactor(width_old, xfactor);

    vigra_precondition((width_old > 1) && (height_old > 1),
                 "resampleImage(): "
                 "Source image too small.\n");
    vigra_precondition((width_new > 1) && (height_new > 1),
                 "resampleImage(): "
                 "Destination image too small.\n");

    typedef typename SrcAccessor::value_type SRCVT;
    typedef BasicImage<SRCVT> TmpImage;
    typedef typename TmpImage::traverser TmpImageIterator;

    BasicImage<SRCVT> tmp(width_old, height_new);

    int x,y;

    typename BasicImage<SRCVT>::Iterator yt = tmp.upperLeft();

    for(x=0; x<width_old; ++x, ++is.x, ++yt.x)
    {
        typename SrcIterator::column_iterator c1 = is.columnIterator();
        typename TmpImageIterator::column_iterator ct = yt.columnIterator();
        resampleLine(c1, c1 + height_old, sa, ct, tmp.accessor(), yfactor);
    }

    yt = tmp.upperLeft();

    for(y=0; y < height_new; ++y, ++yt.y, ++id.y)
    {
        typename DestIterator::row_iterator rd = id.rowIterator();
        typename TmpImageIterator::row_iterator rt = yt.rowIterator();
        resampleLine(rt, rt + width_old, tmp.accessor(), rd, ad, xfactor);
    }

}

template <class SrcIterator, class SrcAccessor,
          class DestIterator, class DestAccessor>
void
resampleImage(SrcIterator is, SrcIterator iend, SrcAccessor sa,
              DestIterator id, DestAccessor ad, double factor)
{
  resampleImage(is, iend, sa, id, ad, factor, factor);
}

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline void
resampleImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
              pair<DestImageIterator, DestAccessor> dest, double factor)
{
  resampleImage(src.first, src.second, src.third, dest.first, dest.second, factor);
}

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline void
resampleImage(triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
              pair<DestImageIterator, DestAccessor> dest, double xfactor, double yfactor)
{
  resampleImage(src.first, src.second, src.third, dest.first, dest.second, xfactor, yfactor);
}

template <class T1, class S1,
          class T2, class S2>
inline void
resampleImage(MultiArrayView<2, T1, S1> const & src,
              MultiArrayView<2, T2, S2> dest, double factor)
{
    if(factor > 1.0)
        vigra_precondition(floor(factor*src.shape()) == dest.shape(),
            "resampleImage(): shape mismatch between input and output.");
    else
        vigra_precondition(ceil(factor*src.shape()) == dest.shape(),
            "resampleImage(): shape mismatch between input and output.");

    resampleImage(srcImageRange(src), destImage(dest), factor);
}

template <class T1, class S1,
          class T2, class S2>
inline void
resampleImage(MultiArrayView<2, T1, S1> const & src,
              MultiArrayView<2, T2, S2> dest, double xfactor, double yfactor)
{
    if(xfactor > 1.0)
        vigra_precondition(floor(xfactor*src.shape(0)) == dest.shape(0),
            "resampleImage(): shape mismatch between input and output.");
    else
        vigra_precondition(ceil(xfactor*src.shape(0)) == dest.shape(0),
            "resampleImage(): shape mismatch between input and output.");
    if(yfactor > 1.0)
        vigra_precondition(floor(yfactor*src.shape(1)) == dest.shape(1),
            "resampleImage(): shape mismatch between input and output.");
    else
        vigra_precondition(ceil(yfactor*src.shape(1)) == dest.shape(1),
            "resampleImage(): shape mismatch between input and output.");

    resampleImage(srcImageRange(src), destImage(dest), xfactor, yfactor);
}

//@}

} // namespace vigra


#endif /* VIGRA_BASICGEOMETRY_HXX */
