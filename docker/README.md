# 做这个docker的目的还是更有效的在mac下直接调用api
尝试了hsi，用python调用，实在是不会啊，虽然看起来很美，但是文档太少，花时间去研究它还不如直接用c++写好完整的需要功能，然后封装出来，还能在mac用。

# 记录一下制作编译环境的docker步骤

1、底包还是用centos，之前的编译就是在centos上完成的。
2、

# cmake
下面是之前真机编译的cmake命令：
cmake -DCMAKE_C_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gcc -DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/g++ -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_HSI=on -DPYTHON_INCLUDE_DIR=/usr/include/python3.6m -DPYTHON_LIBRARY=/usr/lib64/libpython3.6m.so -DCPACK_BINARY_DEB:BOOL=ON -DwxWidgets_LIBRARIES=/home/wxWidgets-3.2.1/lib -DVIGRA_CONFIG_VERSION_HXX=/usr/local/include/vigra/config_version.hxx -DVIGRA_LIBRARIES=/usr/local/lib/libvigraimpex.so -DSWIG_ADD_LIBRARY=/usr/local/share/swig/4.1.0 ../

cmake -DCMAKE_C_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gcc -DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/g++ -DCMAKE_INSTALL_PREFIX=/usr/local -DPYTHON_INCLUDE_DIR=/usr/include/python3.6m -DPYTHON_LIBRARY=/usr/lib64/libpython3.6m.so -DCPACK_BINARY_DEB:BOOL=ON -DwxWidgets_LIBRARIES=/home/wxWidgets-3.2.1/lib -DVIGRA_CONFIG_VERSION_HXX=/usr/local/include/vigra/config_version.hxx -DVIGRA_LIBRARIES=/usr/local/lib/libvigraimpex.so -DSWIG_ADD_LIBRARY=/usr/local/share/swig/4.1.0 ../

## 用这个
cmake -DCMAKE_C_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gcc -DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/g++ -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_HSI=on -DPYTHON_INCLUDE_DIR=/usr/include/python3.6m -DPYTHON_LIBRARY=/usr/lib64/libpython3.6m.so -DCPACK_BINARY_DEB:BOOL=ON -DVIGRA_CONFIG_VERSION_HXX=/usr/local/include/vigra/config_version.hxx -DVIGRA_LIBRARIES=/usr/local/lib/libvigraimpex.so -DSWIG_ADD_LIBRARY=/usr/local/share/swig/4.1.0 ../