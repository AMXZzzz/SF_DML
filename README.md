# SF_DML源码-c++
## ->编译文档

### 1. 安装Visual Studio 2022
- vs2022负载选择 **使用c++的桌面开发**
- 安装完成后才能进入下一步骤

### 2. 安装CUDA,cudann,TensorRt, [安装视频](https://www.bilibili.com/video/BV1pG411h743/?spm_id_from=333.999.0.0&vd_source=48769c3445e4933d438612c7cb69d59c)
- 推荐使用cuda12, cunda8.9, TensorRt 8.6 GA
- 切记，必须先安装VS的c++负载，才能安装CUDA，因为CUDA需要向VS安装一些插件

### 3. 安装[opencv](https://github.com/opencv/opencv/releases)
- 双击安装后,在exe同级目录下解压出一个opencv文件夹,记住他，后面要用
  
### 4. 安装[cmake](https://github.com/Kitware/CMake/releases)
 - 版本要求>3.18即可

### 5.修改SF_TRT_62源码下的CMakeLists.txt
- 打开CMakeLists.txt
- 修改opencv的路径,就是上面记住的路径
- 路径定位到build,即能看到include文件夹
- 检查分隔符是不是2个  **\\\\**
- ctrl + s 保存

### 6. 打开cmake,按照[视频](https://www.bilibili.com/video/BV1pG411h743?p=3&vd_source=48769c3445e4933d438612c7cb69d59c)编译
- 记得将Debug改为Release
- 运行库选择MT
