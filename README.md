itec-scenarios

Install Guide (testet on Ubuntu 12.04 64bit)

# Dependencies:
    * NS3
    * ndnSim
    * Boost C++

# Building:

    # fetch NS-3 + ndnSIM
    * mkdir ndnSIM
    * cd ndnSIM
    * git clone git://github.com/cawka/ns-3-dev-ndnSIM.git ns-3
    * git clone git://github.com/cawka/pybindgen.git pybindgen
    * git clone git://github.com/NDN-Routing/ndnSIM.git ns-3/src/ndnSIM

    # Build and install NS-3 and ndnSIM
    * cd ns-3
    * ./waf configure -d optimized
    * ./waf
    * sudo ./waf install
    * cd ..

    # Build itec-scenarios
    * git clone https://github.com/danposch/itec-scenarios.git
    * cd itec-scenarios\
    * ./bootstrap.sh
    * sudo cp ./libdash/libdash/build/bin/libdash.so  /usr/local/lib/
    * ./waf configure
    * ./waf 
    * ./waf --run example --vis

==============
