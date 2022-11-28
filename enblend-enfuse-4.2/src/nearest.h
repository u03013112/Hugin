/*
 * Copyright (C) 2009-2016 Christoph L. Spiel
 *
 * This file is part of Enblend.
 *
 * Enblend is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Enblend is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Enblend; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __NEAREST_H__
#define __NEAREST_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#ifdef _WIN32
#include <cmath>
#else
#include <math.h>
#endif
#include <stdlib.h>
#include <utility>

#include <vigra/functorexpression.hxx>
#include <vigra/inspectimage.hxx>
#include <vigra/numerictraits.hxx>

#include "timer.h"
#include "opencl_vigra.h"


#ifdef OPENCL
namespace GPU
{
    extern std::unique_ptr<vigra::ocl::DistanceTransformFH> DistanceTransform;
}
#endif


namespace vigra
{
    namespace ocl
    {
        template <class SrcImageIterator, class SrcAccessor,
                  class DestImageIterator, class DestAccessor,
                  class ValueType>
        inline void
        distanceTransform(SrcImageIterator src_upperleft, SrcImageIterator src_lowerright, SrcAccessor src_acc,
                          DestImageIterator dest_upperleft, DestAccessor dest_acc,
                          ValueType background, int norm)
        {
            timer::WallClock wall_clock;

            wall_clock.start();

#ifdef OPENCL
            const bool enable_kernel = parameter::as_boolean("gpu-kernel-dt", true);

            if (GPUContext && GPU::DistanceTransform && enable_kernel)
            {
#ifdef DEBUG
                std::cerr <<
                    command << ": info: choose OpenCL accelaration for Distance Transform" << std::endl;
#endif // DEBUG

                GPU::DistanceTransform->run(src_upperleft, src_lowerright, src_acc,
                                            dest_upperleft, dest_acc,
                                            background, norm);
            }
            else
            {
                if (UseGPU && enable_kernel)
                {
                    std::cerr <<
                        command << ": warning: missing GPUContext or OpenCL DistanceTransform\n" <<
                        command << ": warning: falling back to CPU path" << std::endl;
                }

                vigra::omp::distanceTransform(src_upperleft, src_lowerright, src_acc,
                                              dest_upperleft, dest_acc,
                                              background, norm);
            }
#else
            vigra::omp::distanceTransform(src_upperleft, src_lowerright, src_acc,
                                          dest_upperleft, dest_acc,
                                          background, norm);
#endif // OPENCL

            wall_clock.stop();
            if (parameter::as_boolean("time-distance-transform", false))
            {
                const std::ios::fmtflags flags(std::cerr.flags());
                vigra::Size2D size(src_lowerright - src_upperleft);
                std::cerr <<
                    "\n" <<
                    command << ": timing: wall-clock runtime of `Distance Transform': " <<
                    std::setprecision(3) << 1000.0 * wall_clock.value() << " ms\n" <<
                    command << ": timing: speed according to wall-clock: " <<
                    size.area() / (1048576.0 * wall_clock.value()) << " MPixel/s\n" <<
                    std::endl;
                std::cerr.flags(flags);
            }
        }


        template <class SrcImageIterator, class SrcAccessor,
                  class DestImageIterator, class DestAccessor,
                  class ValueType>
        inline static void
        distanceTransform(vigra::triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                          vigra::pair<DestImageIterator, DestAccessor> dest,
                          ValueType background, int norm)
        {
            vigra::ocl::distanceTransform(src.first, src.second, src.third,
                                          dest.first, dest.second,
                                          background, norm);
        }
    } // namespace ocl
} // namespace vigra


namespace enblend {

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
void
quadruple_image(SrcImageIterator src_upperleft,
                SrcImageIterator src_lowerright, SrcAccessor sa,
                DestImageIterator dest_upperleft, DestAccessor da,
                boundary_t boundary)
{
    const vigra::Diff2D size_x(src_lowerright.x - src_upperleft.x, 0);
    const vigra::Diff2D size_y(0, src_lowerright.y - src_upperleft.y);

    switch (boundary)
    {
    case OpenBoundaries:
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft, da);
        break;

    case HorizontalStrip:
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft, da); // 11
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft + size_x, da); // 12
        break;

    case VerticalStrip:
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft, da); // 11
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft + size_y, da); // 21
        break;

    case DoubleStrip:
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft, da); // 11
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft + size_x, da); // 12
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft + size_y, da); // 21
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                  dest_upperleft + size_x + size_y, da); // 22
        break;

    default:
        NEVER_REACHED("switch control expression \"boundary\" out of range");
    }
}


template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline void
quadruple_image(vigra::triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                vigra::pair<DestImageIterator, DestAccessor> dest,
                boundary_t boundary)
{
    quadruple_image(src.first, src.second, src.third,
                    dest.first, dest.second,
                    boundary);
}


template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
void
quater_image(SrcImageIterator src_upperleft,
             SrcImageIterator src_lowerright, SrcAccessor sa,
             DestImageIterator dest_upperleft, DestAccessor da,
             boundary_t boundary)
{
    const vigra::Diff2D size_x(src_lowerright.x - src_upperleft.x, 0);
    const vigra::Diff2D size_y(0, src_lowerright.y - src_upperleft.y);
    const vigra::Diff2D size_x2(size_x / 2);
    const vigra::Diff2D size_y2(size_y / 2);
    const vigra::Diff2D size_x4(size_x2 / 2);
    const vigra::Diff2D size_y4(size_y2 / 2);

    // destination image
    //  | 11  12 |
    //  |        |
    //  | 21  22 |

    switch (boundary)
    {
    case OpenBoundaries:
        vigra::copyImage(src_upperleft, src_lowerright, sa,
                         dest_upperleft, da);
        break;

    case HorizontalStrip:
        vigra::copyImage(src_upperleft + size_x2,
                         src_upperleft + size_x2 + size_x4 + size_y,
                         sa,
                         dest_upperleft,
                         da); // 11
        vigra::copyImage(src_upperleft + size_x4,
                         src_upperleft + size_x2 + size_y,
                         sa,
                         dest_upperleft + size_x4,
                         da); // 12
        break;

    case VerticalStrip:
        vigra::copyImage(src_upperleft + size_y2,
                         src_upperleft + size_y2 + size_y4 + size_x,
                         sa,
                         dest_upperleft,
                         da); // 21
        vigra::copyImage(src_upperleft + size_y4,
                         src_upperleft + size_y2 + size_x,
                         sa,
                         dest_upperleft + size_y4,
                         da); // 22
        break;

    case DoubleStrip:
        vigra::copyImage(src_upperleft + size_x2 + size_y2,
                         src_upperleft + size_x2 + size_y2 + size_x4 + size_y4,
                         sa,
                         dest_upperleft,
                         da); // 11
        vigra::copyImage(src_upperleft + size_x4 + size_y2,
                         src_upperleft + size_x2 + size_y2 + size_y4,
                         sa,
                         dest_upperleft + size_x4,
                         da); // 12
        vigra::copyImage(src_upperleft + size_x2 + size_y4,
                         src_upperleft + size_x2 + size_x4 + size_y2,
                         sa,
                         dest_upperleft + size_y4,
                         da); // 21
        vigra::copyImage(src_upperleft + size_x4 + size_y4,
                         src_upperleft + size_x2 + size_y2,
                         sa,
                         dest_upperleft + size_x4 + size_y4,
                         da); // 22
        break;

    default:
        NEVER_REACHED("switch control expression \"boundary\" out of range");
    }
}


template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline void
quater_image(vigra::triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
             vigra::pair<DestImageIterator, DestAccessor> dest,
             boundary_t boundary)
{
    quater_image(src.first, src.second, src.third,
                 dest.first, dest.second,
                 boundary);
}


template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor,
          class ValueType>
void
periodicDistanceTransform(SrcImageIterator src_upperleft, SrcImageIterator src_lowerright, SrcAccessor sa,
                          DestImageIterator dest_upperleft, DestAccessor da,
                          ValueType background, int norm, boundary_t boundary)
{
    typedef typename SrcImageIterator::value_type SrcValueType;
    typedef typename DestImageIterator::value_type DestValueType;

    const vigra::Diff2D size(src_lowerright.x - src_upperleft.x,
                             src_lowerright.y - src_upperleft.y);
    int size_x;
    int size_y;

    switch (boundary)
    {
    case OpenBoundaries:
        size_x = size.x;
        size_y = size.y;
        break;

    case HorizontalStrip:
        size_x = 2 * size.x;
        size_y = size.y;
        break;

    case VerticalStrip:
        size_x = size.x;
        size_y = 2 * size.y;
        break;

    case DoubleStrip:
        size_x = 2 * size.x;
        size_y = 2 * size.y;
        break;

    default:
        NEVER_REACHED("switch control expression \"boundary\" out of range");
    }

    vigra::BasicImage<SrcValueType> periodic(size_x, size_y);
    vigra::BasicImage<DestValueType> distance(periodic.size());

    quadruple_image(src_upperleft, src_lowerright, sa, periodic.upperLeft(), periodic.accessor(), boundary);
    vigra::ocl::distanceTransform(srcImageRange(periodic), destImage(distance), background, norm);
    quater_image(srcImageRange(distance), destIter(dest_upperleft, da), boundary);
}


template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor,
          class ValueType>
inline void
periodicDistanceTransform(vigra::triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
                          vigra::pair<DestImageIterator, DestAccessor> dest,
                          ValueType background, int norm, boundary_t boundary)
{
    periodicDistanceTransform(src.first, src.second, src.third,
                              dest.first, dest.second,
                              background, norm, boundary);
}


template <class ValueType>
struct saturating_subtract
{
    typedef ValueType first_argument_type;
    typedef ValueType second_argument_type;
    typedef ValueType result_type;

    result_type operator()(const first_argument_type& v1,
                           const second_argument_type& v2) const
    {
        typedef vigra::NumericTraits<result_type> traits;

        return v2 < v1 ? v1 - v2 : traits::zero();
    }
};


template <class SrcImageIterator, class SrcAccessor>
inline unsigned
quick_tally(SrcImageIterator begin, SrcImageIterator end, SrcAccessor acc,
            unsigned threshold)
{
    typedef typename SrcAccessor::value_type SrcValueType;

    unsigned count = 0U;
    SrcImageIterator i = begin;
    while (count < threshold && i != end)
    {
        if (acc(i) != SrcValueType())
        {
            ++count;
        }
        ++i;
    }

    return count;
}


template <class SrcImageIterator, class SrcAccessor>
inline unsigned
quick_tally(vigra::triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src,
            unsigned threshold)
{
    return quick_tally(src.first, src.second, src.third, threshold);
}


// Compute a mask (dest) that defines the seam line given the
// blackmask (src1) and the whitemask (src2) of the overlapping
// images.
//
// The idea of the algorithm is from
//     Yalin Xiong, Ken Turkowski
//     "Registration, Calibration and Blending in Creating High Quality Panoramas"
//     Proceedings of the 4th IEEE Workshop on Applications of Computer Vision (WACV'98)
// where we find:
//     "To locate the mask boundary, we perform the grassfire
//      transform on two images individually.  The resulting distance
//      maps represent how far away each pixel is from its nearest
//      boundary.  The pixel values of the blend mask is then set to
//      either 0 or 1 by comparing the distance values at each pixel
//      in the two distance maps."
//
// Though we prefer the Distance Transform to the Grassfire Transform.

template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
void
nearestFeatureTransform(SrcImageIterator src1_upperleft, SrcImageIterator src1_lowerright, SrcAccessor sa1,
                        SrcImageIterator src2_upperleft, SrcAccessor sa2,
                        DestImageIterator dest_upperleft, DestAccessor da,
                        nearest_neighbor_metric_t norm, boundary_t boundary)
{
    typedef typename SrcAccessor::value_type SrcPixelType;
    typedef vigra::NumericTraits<SrcPixelType> SrcPixelTraits;

    typedef typename DestAccessor::value_type DestPixelType;
    typedef vigra::NumericTraits<DestPixelType> DestPixelTraits;

    const SrcPixelType background = SrcPixelTraits::zero();
    const vigra::Size2D size(src1_lowerright.x - src1_upperleft.x, src1_lowerright.y - src1_upperleft.y);

    IMAGETYPE<float> dist12(size);
    IMAGETYPE<float> dist21(size);
    if (Verbose >= VERBOSE_NFT_MESSAGES)
    {
        std::cerr << command << ": info: creating ";
        if (CoarseMask) {
            std::cerr << "coarse/" << CoarsenessFactor;
        } else {
            std::cerr << "fine";
        }
        std::cerr << " blend mask: 1/3";
        std::cerr.flush();
    }

    const unsigned overlap_threshold =
        // Arbitrary threshold of overlapping pixels below which we
        // consider the overlap of the masks to be complete, i.e. the
        // image pair as useless.
        //
        // The current parameter default is two times the
        // circumference of the overlap rectangle.
        parameter::as_unsigned("overlap-check-threshold", 2U) * //< overlap-check-threshold 2
        2U * (static_cast<unsigned>(size.height()) + static_cast<unsigned>(size.width()));
    IMAGETYPE<SrcPixelType> diff12(size);
    vigra::omp::combineTwoImages(src1_upperleft, src1_lowerright, sa1,
                                 src2_upperleft, sa2,
                                 diff12.upperLeft(), diff12.accessor(),
                                 saturating_subtract<SrcPixelType>());

    const unsigned tally12 = quick_tally(diff12.begin(), diff12.end(), diff12.accessor(), overlap_threshold);

    switch (boundary)
    {
    case OpenBoundaries:
        vigra::ocl::distanceTransform(srcImageRange(diff12), destImage(dist12), background, norm);
        break;

    case HorizontalStrip: // FALLTHROUGH
    case VerticalStrip:   // FALLTHROUGH
    case DoubleStrip:
        periodicDistanceTransform(srcImageRange(diff12), destImage(dist12),
                                  background, norm, boundary);
        break;

    default:
        NEVER_REACHED("switch control expression \"boundary\" out of range");
    }

    if (Verbose >= VERBOSE_NFT_MESSAGES)
    {
        std::cerr << " 2/3";
        std::cerr.flush();
    }
    IMAGETYPE<SrcPixelType> diff21(size);
    vigra::omp::combineTwoImages(src2_upperleft, src2_upperleft + size, sa2,
                                 src1_upperleft, sa1,
                                 diff21.upperLeft(), diff21.accessor(),
                                 saturating_subtract<SrcPixelType>());

    const unsigned tally21 = quick_tally(diff21.begin(), diff21.end(), diff21.accessor(), overlap_threshold);

    switch (boundary)
    {
    case OpenBoundaries:
        vigra::ocl::distanceTransform(srcImageRange(diff21), destImage(dist21), background, norm);
        break;

    case HorizontalStrip: // FALLTHROUGH
    case VerticalStrip:   // FALLTHROUGH
    case DoubleStrip:
        periodicDistanceTransform(srcImageRange(diff21), destImage(dist21),
                                  background, norm, boundary);
        break;

    default:
        NEVER_REACHED("switch control expression \"boundary\" out of range");
    }

    if (Verbose >= VERBOSE_NFT_MESSAGES)
    {
        std::cerr << " 3/3";
        std::cerr.flush();
    }

    const unsigned overlap_tally = std::max(tally12, tally21);
    if (overlap_tally < overlap_threshold)
    {
        std::cerr << "\n" <<
            command << ": excessive image overlap detected; too high risk of defective seam line\n" <<
            command << ": note: remove at least one of the images" << std::endl;
#ifdef DEBUG
        std::cerr <<
            command << ": note: overlap area size is " << size << " > " << overlap_threshold << ", but only\n" <<
            command << ": note: " << overlap_tally << " of " << size.x * size.y << " pixels do not overlap" <<
            std::endl;
#endif
        exit(1);
    }

    vigra::omp::combineTwoImages(dist12.upperLeft(), dist12.lowerRight(), dist12.accessor(),
                                 dist21.upperLeft(), dist21.accessor(),
                                 dest_upperleft, da,
                                 ifThenElse(vigra::functor::Arg1() < vigra::functor::Arg2(),
                                            vigra::functor::Param(DestPixelTraits::max()),
                                            vigra::functor::Param(DestPixelTraits::zero())));

    if (Verbose >= VERBOSE_NFT_MESSAGES)
    {
        std::cerr << std::endl;
    }
}


template <class SrcImageIterator, class SrcAccessor,
          class DestImageIterator, class DestAccessor>
inline void
nearestFeatureTransform(vigra::triple<SrcImageIterator, SrcImageIterator, SrcAccessor> src1,
                        vigra::pair<SrcImageIterator, SrcAccessor> src2,
                        vigra::pair<DestImageIterator, DestAccessor> dest,
                        nearest_neighbor_metric_t norm, boundary_t boundary)
{
    nearestFeatureTransform(src1.first, src1.second, src1.third,
                            src2.first, src2.second,
                            dest.first, dest.second,
                            norm, boundary);
}

} // namespace enblend

#endif /* __NEAREST_H__ */

// Local Variables:
// mode: c++
// End:
