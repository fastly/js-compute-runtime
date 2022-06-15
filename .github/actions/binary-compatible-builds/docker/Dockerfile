FROM centos:7

# See https://edwards.sdsu.edu/research/c11-on-centos-6/ for where these
RUN yum install -y centos-release-scl cmake xz epel-release python3 unzip git
RUN yum install -y patchelf devtoolset-8-gcc devtoolset-8-binutils devtoolset-8-gcc-c++

# Delete `libstdc++.so` to force gcc to link against `libstdc++.a` instead.
# This is a hack and not the right way to do this, but it ends up doing the
# right thing for now.
RUN rm -f /opt/rh/devtoolset-8/root/usr/lib/gcc/x86_64-redhat-linux/8/libstdc++.so
