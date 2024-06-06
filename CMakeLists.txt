cmake_minimum_required(VERSION 3.18)		# cmake 最低版本，3.18具有新特性

project(SF_DML_Project) 	# 项目名称

# -- 改成自己的路径
set(OpenCV_DIR "C:\\Program Files\\opencv\\build")   # 你自己的opecv根路径

# -- 设置标准
add_definitions(-DAPI_EXPORTS)			# 这行解决警告
add_definitions(-DUNICODE -D_UNICODE)		# UNICODE字符集
option(CUDA_USE_STATIC_CUDA_RUNTIME OFF)		# 设置可分离编译，在指定生成cu文件之前
set(CMAKE_CXX_STANDARD 17)			# 设置编译器版本
set(CMAKE_BUILD_TYPE Release)			# 设置debug or Release

# -- 设置多线程
set(THREADS_PREFER_PTHREAD_FLAG ON)		# 设置线程首选标志
find_package(Threads)				# 引入线程库

# -- 设置 opencv
find_package(OpenCV QUIET)				# 引入opecv库
include_directories(${OpenCV_INCLUDE_DIRS})		#  opencv  的 include目录

# -- 项目设置
include_directories(${PROJECT_SOURCE_DIR}/include)	# 指定项目的include目录
include_directories(${PROJECT_SOURCE_DIR})		# 指定项目的include目录
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)		# 指定项目的lib路径
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL" )	# 增量编译

# -- 设置imgui筛选器
file(GLOB_RECURSE IMGUI_RES ./imgui/*.*)
source_group(Imgui FILES ${IMGUI_RES})

# -- spdlog
file(GLOB_RECURSE SPDLOG_RES ./spdlog/*.*)
source_group(Spdlog FILES ${SPDLOG_RES})

# -- 设置项目头文件筛选器
file(GLOB_RECURSE CURRENT_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)
source_group("Header Files" FILES ${CURRENT_HEADERS}) 

# -- 设置项目cpp文件筛选器
file(GLOB_RECURSE CPP_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# -- 设置SF_DML项目属性
add_executable(SF_DML ${CPP_FILES} ${CURRENT_HEADERS} ${NETVER_H} ${NETVER_H} ${NETVER_S} ${NETVER_M} ${IMGUI_RES} ${SPDLOG_RES})
SET(SF_DML "${CMAKE_EXE_LINKER_FLAGS} /level='requireAdministrator' /uiAccess='false'")
Set_Target_Properties(SF_DML PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SF_DML)


# -- 链接依赖
target_link_libraries(SF_DML  "d3d11" "dxgi" "runtimeobject")  	# 链接d3d11和Toast库
target_link_libraries(SF_DML ${OpenCV_LIBS})     		# 链接opencv依赖
target_link_libraries(SF_DML "msdk")     			# 链接易键鼠
target_link_libraries(SF_DML "ntdll")     			# net驱动
target_link_libraries(SF_DML "SFDXGI")     			# dx截图
target_link_libraries(SF_DML "onnxruntime" "DirectML")     	