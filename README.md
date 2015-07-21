itec-scenarios

Install Guide (testet on Ubuntu 12.04 64bit)

# Dependencies:
    * NS3
    * ndnSim
    * Boost C++
		* libcurl
		* cmake
		+ BRITE

# Building:

    # fetch NS-3 + ndnSIM
    * mkdir ndnSIM
    * cd ndnSIM
    * git clone git://github.com/cawka/ns-3-dev-ndnSIM.git ns-3
    * git clone git://github.com/cawka/pybindgen.git pybindgen
    * git clone git://github.com/NDN-Routing/ndnSIM.git ns-3/src/ndnSIM

		# Building Brite
		* hg clone http://code.nsnam.org/BRITE
		* cd BRITE
		* make
		* sudo cp *.h /usr/local/include/ns3-dev/ns3
		* sudo cp libbrite.so /usr/lib/
		* sudo mkdir /usr/local/include/ns3-dev/ns3/Models
		* cd Models/
		* sudo cp *.h /usr/local/include/ns3-dev/ns3/Models
		* cd ../..

    # Build and install NS-3 and ndnSIM
    * cd ns-3
    * CXXFLAGS="-std=c++11" ./waf configure -d optimized --with-brite=../BRITE --boost-libs=/usr/lib/x86_64-linux-gnu
    * ./waf
    * sudo ./waf install
		* cd ./build
		* sudo cp ./libns3-dev-brite-optimized.so /usr/local/lib/
    * cd ..

    # Build itec-scenarios
    * git clone https://github.com/danposch/itec-scenarios.git
    * cd itec-scenarios\
    * ./bootstrap.sh
    * sudo cp ./libdash/libdash/build/bin/libdash.so  /usr/local/lib/
    * ./waf configure
    * ./waf 
    * ./waf --run example --vis
    
# Logging:
    # Logging Parameters
    # --v2: Enable very verpose output (everything) for all components
    # --v1: Enable verbose output (>= LOG_INFO) for all components
    # --v0: Enable debug output (>= LOG_DEBUG) for all components
    # --vN: Do not disable or enable any logging, use NS_LOG console parameter instead
    # Example given for NS_LOG for DashPlayer with all output of DashPlayer being logged:
    * NS_LOG=DashPlayer=level_all ./waf --run example

#some examples
		* git clone https://github.com/cawka/ndnSIM-ddos-interest-flooding.git

==============
