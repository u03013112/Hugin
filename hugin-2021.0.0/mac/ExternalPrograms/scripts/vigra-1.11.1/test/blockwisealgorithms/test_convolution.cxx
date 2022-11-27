#include <vigra/blockwise_convolution.hxx>
#include <vigra/blockwise_convolution.hxx>

#include <vigra/multi_convolution.hxx>
#include <vigra/unittest.hxx>
#include <vigra/multi_blocking.hxx>
#include <vigra/multi_blockwise.hxx>

#include <iostream>
#include "utils.hxx"

using namespace std;
using namespace vigra;

struct BlockwiseConvolutionTest
{
    void simpleTest()
    {
        typedef MultiArray<2, double> Array;
        typedef Array::difference_type Shape;
 
        Shape shape(40);
        Shape block_shape(2);

        Array data(shape);
        fillRandom(data.begin(), data.end(), 2000);

        Kernel1D<double> kernel;
        kernel.initAveraging(3, 2);

        Array correct_output(shape);
        separableConvolveMultiArray(data, correct_output, kernel);
        
        Array tested_output(shape);
        separableConvolveBlockwise(data, tested_output, kernel, block_shape);
        
        shouldEqualSequenceTolerance(correct_output.begin(), correct_output.end(), tested_output.begin(), 1e-14);
    }

    void chunkedTest()
    {
        static const int N = 3;

        typedef MultiArray<3, int> NormalArray;
        typedef ChunkedArrayLazy<3, int> ChunkedArray;

        typedef NormalArray::difference_type Shape;
        
        Shape shape(40);
        
        NormalArray data(shape);
        fillRandom(data.begin(), data.end(), 2000);
        ChunkedArray chunked_data(shape);
        chunked_data.commitSubarray(Shape(0), data);

        Kernel1D<double> kernel;
        kernel.initAveraging(3, 2);
        vector<Kernel1D<double> > kernels(N, kernel);
        
        separableConvolveMultiArray(data, data, kernels.begin()); // data now contains output
        
        separableConvolveBlockwise(chunked_data, chunked_data, kernels.begin());
        
        NormalArray checked_out_data(shape);
        chunked_data.checkoutSubarray(Shape(0), checked_out_data);
        for(int i = 0; i != data.size(); ++i)
        {
            shouldEqual(data[i], checked_out_data[i]);
        }
    }

    void testParallel()
    {
        double sigma = 1.0;
        BlockwiseConvolutionOptions<2>   opt;
        TinyVector<double, 2> sigmaV(sigma, sigma);

        opt.setStdDev(sigmaV);
        opt.blockShape(TinyVector<int, 2>(5,7));
        opt.numThreads(ParallelOptions::Nice);
        std::cout << "running testParallel() with " << opt.getNumThreads() << " threads." << std::endl;

        typedef MultiArray<2, double> Array;
        typedef Array::difference_type Shape;
        
        // random array
        Shape shape(200,200);
        Array data(shape);
        fillRandom(data.begin(), data.end(), 2000);

        // blockwise
        Array resB(shape);
        gaussianSmoothMultiArray(data, resB, opt);

        // sequential
        Array res(shape);
        gaussianSmoothMultiArray(data, res, sigma);

        shouldEqualSequenceTolerance(
            res.begin(), 
            res.end(), 
            resB.begin(), 
            1e-14
        );

    }
};

struct BlockwiseConvolutionTestSuite
: public test_suite
{
    BlockwiseConvolutionTestSuite()
    : test_suite("blockwise convolution test")
    {
        add(testCase(&BlockwiseConvolutionTest::simpleTest));
        add(testCase(&BlockwiseConvolutionTest::chunkedTest));
        add(testCase(&BlockwiseConvolutionTest::testParallel));
    }
};

int main(int argc, char** argv)
{
    BlockwiseConvolutionTestSuite test;
    int failed = test.run(testsToBeExecuted(argc, argv));

    cout << test.report() << endl;

    return failed != 0;
}
