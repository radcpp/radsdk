#include <iostream>

#include <boost/compute/core.hpp>

namespace compute = boost::compute;

int sample_Boost_Compute_HelloWorld()
{
    // get the default device
    compute::device device = compute::system::default_device();

    // print the device's name
    std::cout << "hello from " << device.name() << std::endl;

    return 0;
}

#include <vector>

#include <boost/compute/algorithm/copy.hpp>
#include <boost/compute/container/vector.hpp>

int sample_Boost_Compute_TransferringData()
{
    // get default device and setup context
    compute::device device = compute::system::default_device();
    compute::context context(device);
    compute::command_queue queue(context, device);

    // create data array on host
    int host_data[] = { 1, 3, 5, 7, 9 };

    // create vector on device
    compute::vector<int> device_vector(5, context);

    // copy from host to device
    compute::copy(
        host_data, host_data + 5, device_vector.begin(), queue
    );

    // create vector on host
    std::vector<int> host_vector(5);

    // copy data back to host
    compute::copy(
        device_vector.begin(), device_vector.end(), host_vector.begin(), queue
    );

    return 0;
}

#include <vector>
#include <algorithm>

#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/functional/math.hpp>

#include <random>

int sample_Boost_Compute_TransformingData()
{
    // get default device and setup context
    compute::device device = compute::system::default_device();
    compute::context context(device);
    compute::command_queue queue(context, device);

    std::default_random_engine randEngine;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // generate random data on the host
    std::vector<float> host_vector(1000000);
    std::generate(host_vector.begin(), host_vector.end(), [&]() {return dist(randEngine); });

    // create a vector on the device
    compute::vector<float> device_vector(host_vector.size(), context);

    // transfer data from the host to the device
    compute::copy(
        host_vector.begin(), host_vector.end(), device_vector.begin(), queue
    );

    // calculate the square-root of each element in-place
    compute::transform(
        device_vector.begin(),
        device_vector.end(),
        device_vector.begin(),
        compute::sqrt<float>(),
        queue
    );

    // copy values back to the host
    compute::copy(
        device_vector.begin(), device_vector.end(), host_vector.begin(), queue
    );

    return 0;
}
