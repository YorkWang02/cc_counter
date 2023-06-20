# Cuckoo_Counter and CMSketch realization using BESS

## Clone BESS

Once you have your host configured, go ahead and clone the BESS codebase and cd to the new directory:

    git clone https://github.com/NetSys/bess.git
    cd bess/
## Install dependencies
On Ubuntu 18.04 you can install all of the BESS build dependencies with the following commmand.

    sudo apt install make apt-transport-https ca-certificates g++ make pkg-config libunwind8-dev liblzma-dev zlib1g-dev libpcap-dev libssl-dev libnuma-dev git python python-pip python-scapy libgflags-dev libgoogle-glog-dev libgraph-easy-perl libgtest-dev libgrpc++-dev libprotobuf-dev libc-ares-dev libbenchmark-dev libgtest-dev protobuf-compiler-grpc
    ## The following packages are needed to run bessctl
    pip install --user protobuf grpcio scapy
For older releases, BESS conveniently comes with all of its dependencies bundled in an Ansible script -- all you have to do is run the script and it will set everything up. A warning: the script will aggressively replace any conflicting packages (this is why we recommend a fresh install). Here's what to run in your shell:


    sudo apt-get install -y software-properties-common
    sudo apt-add-repository -y ppa:ansible/ansible
    sudo apt-get update
    sudo apt-get install -y ansible
    ansible-playbook -i localhost, -c local env/build-dep.yml
    ansible-playbook -i localhost, -c local env/runtime.yml  # if you want to run BESS on the same machine.
    sudo reboot
Return to the CM shell and reload vagrant in order to remount the `/opt/bess` directory: vagrant up vagrant reload vagrant ssh
## Move files to right locations

Move files under the folder `modulefiles` (`CMSketch.h`, `CMSmodule.h`, `CMSmodule.cc`, `cuckoo_counter.h`, `cuckoo_counter.cc` and `cuckoo_counter_c.h`) to `bess/core/modules`. Only after this operation can you build bess correctly later with CMSketch module and Cuckoo_counter module in.

 Then move files under the folder `bessfiles` (`cmsketch.bess` and `cuckoo.bess`) to `bess/bessctl/conf`.

## Start hugepages

Every time you reboot the system, you have to set up hugepages. Note that this step is not just for installation! Every time you reboot and want to use bess, you need to run the following:

    # For single-node systems
    sudo sysctl vm.nr_hugepages=1024`

    # For multi-node (NUMA) systems
    echo 1024 | sudo tee /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
    echo 1024 | sudo tee /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
## And build!

Return to your `bess/` directory (the downloaded, cloned repository) and run `./build.py`. This should build BESS for you.

## Start up BESS and run a sample configuration

Assuming building BESS was successful, you should now be ready to use bessctl to start up a bessd switch and configure it. Here's what to do from your `bess/` directory:

    $ cd bessctl/
    $ ./bessctl
    Type "help" for more information.
    Connection to localhost:10514 failed
    Perhaps bessd daemon is not running locally? Try "daemon start".
    <disconnected> $
You will now enter a command line interface. Don't worry about the Connection to `localhost:10514 failed` warning; it just tells you that the BESS daemon (bessd) is not running. Here's how to start bessd:

    > daemon start
You should now see a command prompt that says `localhost:10514`. This means that you have successfully started a bessd instance (which is currently forwarding no packets) and that your bessctl instance is communicating with bessd over a local socket. To make your bessd instance forward some packets, try running a sample script!

    > run samples/acl
    > monitor pipeline
`run samples/acl` (located at `bessctl/conf/samples/acl.bess` file) starts up a sample configuration of modules. bessd is not connected to any network interface cards or VMs so it creates its own packets to forward using a Source module; it then forwards them through an ACL module which filters out half of the packets (those that match a blacklisted term). monitor pipeline is a quick way to see how packets are flowing in your bessd configuration -- you can always type `monitor pipeline` to see all of the modules and ports in the system and how many packets per second are flowing through them.
## Try Cuckoo_Counter and CMSketch!
Simply use `run cuckoo` to see how Cuckoo_Counter works on BESS and `run cmsketch` to see CMSketch. These will actually run files located in `bess/bessctl/conf` with a suffix named "bess"(`cuckoo.bess` and `cmsketch.bess`). Don't forget to use `monitor tc` or `monitor pipeline` for more information during the running if needed.

Also remember you can always use `quit` to get out.