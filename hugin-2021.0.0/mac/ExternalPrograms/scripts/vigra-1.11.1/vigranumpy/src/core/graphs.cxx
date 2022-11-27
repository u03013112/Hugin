/************************************************************************/
/*                                                                      */
/*                 Copyright 2011 by Ullrich Koethe                     */
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

#define PY_ARRAY_UNIQUE_SYMBOL vigranumpygraphs_PyArray_API
//#define NO_IMPORT_ARRAY

#include <vigra/numpy_array.hxx>
#include <vigra/numpy_array_converters.hxx>
#include <vigra/graphs.hxx>
#include <vigra/metrics.hxx>

namespace python = boost::python;

namespace vigra{





	void defineInvalid(){
        python::class_<lemon::Invalid>("Invalid",python::init<>());
    }

	void defineAdjacencyListGraph();
	void defineGridGraph2d();
    void defineGridGraph3d();
    void defineGridGraphImplicitEdgeMap();
    template<unsigned int DIM>
    void defineGridRag();


    //void defineEccentricity();
} // namespace vigra

using namespace vigra;
using namespace boost::python;



BOOST_PYTHON_MODULE_INIT(graphs)
{
    import_vigranumpy();

    python::docstring_options doc_options(true, true, false);

    // all exporters needed for graph exporters (like lemon::INVALID)
    defineInvalid();

    enum_<metrics::MetricType>("MetricType")
        .value("chiSquared", metrics::ChiSquaredMetric)
        .value("hellinger", metrics::HellingerMetric)
        .value("squaredNorm", metrics::SquaredNormMetric)
        .value("norm", metrics::NormMetric)
        .value("manhattan", metrics::ManhattanMetric)
        .value("symetricKl", metrics::SymetricKlMetric)
        .value("bhattacharya", metrics::BhattacharyaMetric)
        ;
    


    // all graph classes itself (GridGraph , AdjacencyListGraph)
    defineAdjacencyListGraph();
    defineGridGraph2d();
    defineGridGraph3d();

    // implicit edge maps
    defineGridGraphImplicitEdgeMap();

}
