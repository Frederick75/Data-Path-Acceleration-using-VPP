**Step-by-Step Compilation & Deployment**

To compile and load either plugin into VPP:

1. Build Step:  

mkdir build && cd build 
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make 
sudo make install  

2. Execution: 

When VPP launches, the shared object binary (.so) is dynamically loaded into memory. You can verify its registration via the VPP CLI:  

vppctl show vlib graph 
