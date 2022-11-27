/************************************************************************/
/*                                                                      */
/*     Copyright 2011-2012 Stefan Schmidt and Ullrich Koethe            */
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

/**
 * This header provides definitions of graph-related types
 * and optionally provides a gateway to popular graph libraries
 * (for now, BGL is supported).
 */

#ifndef VIGRA_GRAPH_GENERALIZATION_HXX
#define VIGRA_GRAPH_GENERALIZATION_HXX


#include "graphs.hxx"
#include "multi_gridgraph.hxx"
namespace vigra{



    template<class MAP>
    struct GraphMapTypeTraits{
        typedef typename MAP::value_type      Value;
        typedef typename MAP::reference       Reference;
        typedef typename MAP::const_reference ConstReference;
    };

    // generalizes the iterator begin end accessed
    // since grid graph has no constructor
    // for INVALID
    template<class GRAPH>
    struct GraphIteratorAccessor{
        typedef GRAPH Graph;
        typedef typename Graph::Node        Node;
        typedef typename Graph::NodeIt      NodeIt;
        typedef typename Graph::EdgeIt      EdgeIt;
        typedef typename Graph::ArcIt       ArcIt;
        typedef typename Graph::OutArcIt    OutArcIt;
        typedef typename Graph::IncEdgeIt   IncEdgeIt;

        static NodeIt nodesBegin(const Graph & g){ return NodeIt(g);}
        static EdgeIt edgesBegin(const Graph & g){ return EdgeIt(g);}
        static ArcIt  arcsBegin( const Graph & g){ return ArcIt( g);}
        static OutArcIt outArcBegin(const Graph & g,const Node & node){
            return OutArcIt(g,node);
        }
        static IncEdgeIt incEdgeBegin(const Graph & g,const Node & node){
            return IncEdgeIt(g,node);
        }


        static NodeIt nodesEnd(const Graph &){ return NodeIt(lemon::INVALID);}
        static EdgeIt edgesEnd(const Graph &){ return EdgeIt(lemon::INVALID);}
        static ArcIt  arcsEnd( const Graph &){ return ArcIt( lemon::INVALID);}
        static OutArcIt outArcEnd(const Graph &,const Node &){
            return OutArcIt(lemon::INVALID);
        }
        static IncEdgeIt incEdgeEnd(const Graph &,const Node &){
            return IncEdgeIt(lemon::INVALID);
        }
    };

    // generalizes the iterator begin end accessed
    // since grid graph has no constructor
    // for INVALID
    template<unsigned int DIM,class DIRECTED_TAG>
    struct GraphIteratorAccessor<GridGraph<DIM,DIRECTED_TAG> >{
        typedef GridGraph<DIM,DIRECTED_TAG> Graph;
        typedef typename Graph::Node Node;
        typedef typename Graph::NodeIt NodeIt;
        typedef typename Graph::EdgeIt EdgeIt;
        typedef typename Graph::ArcIt  ArcIt;
        typedef typename Graph::OutArcIt OutArcIt;
        typedef typename Graph::IncEdgeIt   IncEdgeIt;

        static NodeIt   nodesBegin(const Graph & g){ return NodeIt(g);}
        static EdgeIt   edgesBegin(const Graph & g){ return g.get_edge_iterator();}
        static ArcIt    arcsBegin( const Graph & g){ return ArcIt( g);}
        static OutArcIt outArcBegin(const Graph & g,const Node & node){
            return g.get_out_edge_iterator(node);
        }
         static IncEdgeIt incEdgeBegin(const Graph & g,const Node & node){
            return IncEdgeIt(g,node);
        }

        static NodeIt   nodesEnd(const Graph & g){ return g.get_vertex_end_iterator();}
        static EdgeIt   edgesEnd(const Graph & g){ return g.get_edge_end_iterator();}
        static ArcIt    arcsEnd( const Graph & g){ return g.get_arc_end_iterator(); }
        static OutArcIt outArcEnd(const Graph & g,const Node & node){
            return g.get_out_edge_end_iterator(node);
        }
        static IncEdgeIt incEdgeEnd(const Graph &,const Node &){
            return IncEdgeIt(lemon::INVALID);
        }
    };


    // shape of graphs node,edge,arc-maps
    template<class GRAPH>
    class IntrinsicGraphShape{
    private:
        typedef GRAPH Graph;
        typedef typename vigra::MultiArray<1,int>::difference_type DiffType1d;
        typedef typename Graph::index_type  index_type;
    public:
        typedef typename Graph::Node Node ;
        typedef typename Graph::Edge Edge ;
        typedef typename  Graph::Arc  Arc ;

        typedef DiffType1d IntrinsicNodeMapShape;
        typedef DiffType1d IntrinsicEdgeMapShape;
        typedef DiffType1d  IntrinsicArcMapShape;

        static IntrinsicNodeMapShape intrinsicNodeMapShape(const Graph & g){
            return IntrinsicNodeMapShape(g.maxNodeId()+1);
        }
        static IntrinsicEdgeMapShape intrinsicEdgeMapShape(const Graph & g){
            return IntrinsicEdgeMapShape(g.maxEdgeId()+1);
        }
        static IntrinsicArcMapShape intrinsicArcMapShape(const Graph & g){
            return  IntrinsicArcMapShape(g.maxArcId()+1);
        }


        static const unsigned int IntrinsicNodeMapDimension=1;
        static const unsigned int IntrinsicEdgeMapDimension=1;
        static const unsigned int IntrinsicArcMapDimension=1;
    };
    // shape of graphs node,edge,arc-maps
    template<unsigned int DIM,class DIRECTED_TAG>
    class IntrinsicGraphShape<GridGraph<DIM,DIRECTED_TAG> >{
    private:
        typedef GridGraph<DIM,DIRECTED_TAG> Graph;
    typedef typename Graph::index_type  index_type;
    public:
        typedef typename Graph::Node Node ;
        typedef typename Graph::Edge Edge ;
        typedef typename  Graph::Arc  Arc ;

        typedef typename Graph::shape_type              IntrinsicNodeMapShape;
        typedef typename Graph::edge_propmap_shape_type IntrinsicEdgeMapShape;
        typedef typename Graph::edge_propmap_shape_type IntrinsicArcMapShape;

        static IntrinsicNodeMapShape intrinsicNodeMapShape(const Graph & g){
            return  g.shape();
        }
        static IntrinsicEdgeMapShape intrinsicEdgeMapShape(const Graph & g){
            return  g.edge_propmap_shape();
        }
        static IntrinsicArcMapShape intrinsicArcMapShape(const Graph & g){
            return  g.arc_propmap_shape();
        }
        static const unsigned int IntrinsicNodeMapDimension=DIM;
        static const unsigned int IntrinsicEdgeMapDimension=DIM+1;
        static const unsigned int IntrinsicArcMapDimension=DIM+1;
    };

    // convert a descriptor to an multi_array index w.r.t. 
    // an node/edge/arc map
    template<class GRAPH>
    class GraphDescriptorToMultiArrayIndex{
    private:
        typedef GRAPH Graph;
    typedef typename Graph::index_type  index_type;
    public:
        typedef typename Graph::Node Node ;
        typedef typename Graph::Edge Edge ;
        typedef typename  Graph::Arc  Arc ;

        typedef typename IntrinsicGraphShape<Graph>::IntrinsicNodeMapShape IntrinsicNodeMapShape;
        typedef typename IntrinsicGraphShape<Graph>::IntrinsicEdgeMapShape IntrinsicEdgeMapShape;
        typedef typename IntrinsicGraphShape<Graph>::IntrinsicArcMapShape  IntrinsicArcMapShape;

        static IntrinsicNodeMapShape intrinsicNodeCoordinate(const Graph & g,const Node & node){
            return IntrinsicNodeMapShape(g.id(node));
        }
        static IntrinsicEdgeMapShape intrinsicEdgeCoordinate(const Graph & g,const Edge & edge){
            return IntrinsicEdgeMapShape(g.id(edge));
        }
        static IntrinsicArcMapShape intrinsicArcCoordinate(const Graph & g,const Arc & arc){
            return  IntrinsicArcMapShape(g.id(arc));
        }

    };

    // convert a descriptor to an multi_array index w.r.t. 
    // an node/edge/arc map
    template<unsigned int DIM,class DIRECTED_TAG>
    class GraphDescriptorToMultiArrayIndex<GridGraph<DIM,DIRECTED_TAG> >{
    private:
        typedef GridGraph<DIM,DIRECTED_TAG> Graph;
        typedef typename Graph::index_type  index_type;
    public:
        typedef typename Graph::Node Node ;
        typedef typename Graph::Edge Edge ;
        typedef typename  Graph::Arc  Arc ;

        typedef typename IntrinsicGraphShape<Graph>::IntrinsicNodeMapShape IntrinsicNodeMapShape;
        typedef typename IntrinsicGraphShape<Graph>::IntrinsicEdgeMapShape IntrinsicEdgeMapShape;
        typedef typename IntrinsicGraphShape<Graph>::IntrinsicArcMapShape  IntrinsicArcMapShape;


        static Node intrinsicNodeCoordinate(const Graph &,const Node & node){
            return node;
        }
        static Edge intrinsicEdgeCoordinate(const Graph &,const Edge & edge){
            return edge;
        }
        static Arc  intrinsicArcCoordinate (const Graph &,const Arc & arc){
            return arc;
        }

    };





} // namespace vigra

#endif // VIGRA_GRAPH_GENERALIZATION_HXX
