This repositories contains implementations for the following concepts and features for NDN:
- Persistent Interests (PI) including a sample push-consumer and push-producer
- An adaptive forwarding strategy for PIs
- QoS Class Identifier (QCI), enabling QoS features in NDN

Available simulations
=====================

PIPS-scenario-simple
---------------

A simple scenario used to showcase the forwarding strategy in action.

PIPS-scenario
---------------

An advanced scenario used for evaluations from http://dx.doi.org/10.1145/3125719.3132091

Installing and patching ndnSIM
==============================

The code was tested with ndnSIM version 2.3. on Ubuntu 16.04.1 LTS

    # Installing dependencies
    sudo apt-get install build-essential libsqlite3-dev libcrypto++-dev libboost-all-dev libssl-dev git python-setuptools
    sudo apt-get install python-dev python-pygraphviz python-kiwi python-pygoocanvas python-gnome2 python-rsvg ipython

    # Checkout latest version of ndnSIM
    mkdir ndnSIM_2.3_with_PIPS
    cd ndnSIM_2.3_with_PIPS
    git clone https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3
    git clone https://github.com/named-data-ndnSIM/pybindgen.git pybindgen
    git clone --recursive https://github.com/named-data-ndnSIM/ndnSIM.git ns-3/src/ndnSIM

    # Set correct version for ndnSIM and compile it
    cd ns-3
    git checkout 333e6b052c101625199af40107edd6e379a36119
    cd src/ndnSIM/
    git checkout 0970340dd68742e5433f237c0a48de35986cd597
    cd NFD/
    git checkout 38111cde9bab698f6eaf1a9d430130c2cbb3eca4
    cd ../ndn-cxx/
    git checkout 4692ba80cf1dcf07acbbaba8a134ea22481dd457
    cd ../../../
    ./waf configure -d optimized
    # PyViz visualizer should be enabled
    ./waf
    sudo ./waf install

    # Checkout pips-scenario
    cd ..
    git clone https://github.com/phylib/pips-scenario.git
    cd pips-scenario

    # Patch ndn-cxx
    cp extern/data.cpp ../ns-3/src/ndnSIM/ndn-cxx/src/
    cp extern/data.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/
    cp extern/interest.cpp ../ns-3/src/ndnSIM/ndn-cxx/src/
    cp extern/interest.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/
    cp extern/tag-host.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/
    cp extern/qci.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/encoding/
    cp extern/tlv.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/encoding/
    cp extern/nack-header.cpp ../ns-3/src/ndnSIM/ndn-cxx/src/lp/
    cp extern/nack-header.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/lp/

    # Patch NFD
    cp extern/bandwidth-estimator.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/bandwidth-estimator.hpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/forwarder.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/interface-estimation.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/interface-estimation.hpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/loss-estimator-time-window.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/loss-estimator-time-window.hpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/loss-estimator.hpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/measurement-info.hpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/retx-suppression-exponential.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/rtt-estimator2.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/rtt-estimator2.hpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/strategy-helper.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/strategy-helper.hpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/strategy-requirements.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/strategy-requirements.hpp ../ns-3/src/ndnSIM/NFD/daemon/fw/


    # Recompile ndnSIM
    cd ../ns-3/
    ./waf
    sudo ./waf install
    
    # Compile and run scenario
    cd ../pips-scenario
    ./waf configure
    ./waf
    sudo ldconfig
    ./waf --run=PIPS-scenario-simple --vis

Compiling
=========

To configure in optimized mode without logging **(default)**:

    ./waf configure

To configure in optimized mode with scenario logging enabled (logging in NS-3 and ndnSIM modules will still be disabled,
but you can see output from NS_LOG* calls from your scenarios and extensions):

    ./waf configure --logging

To configure in debug mode with all logging enabled

    ./waf configure --debug

If you have installed NS-3 in a non-standard location, you may need to set up ``PKG_CONFIG_PATH`` variable.

Running
=======

Normally, you can run scenarios either directly

    ./build/<scenario_name>

or using waf

    ./waf --run <scenario_name>

If NS-3 is installed in a non-standard location, on some platforms (e.g., Linux) you need to specify ``LD_LIBRARY_PATH`` variable:

    LD_LIBRARY_PATH=/usr/local/lib ./build/<scenario_name>

or

    LD_LIBRARY_PATH=/usr/local/lib ./waf --run <scenario_name>

To run scenario using debugger, use the following command:

    gdb --args ./build/<scenario_name>


Running with visualizer
-----------------------

The following will run scenario with visualizer:

    ./waf --run <scenario_name> --vis

or

    PKG_LIBRARY_PATH=/usr/local/lib ./waf --run <scenario_name> --vis

If you want to request automatic node placement, set up additional environment variable:

    NS_VIS_ASSIGN=1 ./waf --run <scenario_name> --vis

or

    PKG_LIBRARY_PATH=/usr/local/lib NS_VIS_ASSIGN=1 ./waf --run <scenario_name> --vis
