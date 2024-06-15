cmake_minimum_required(VERSION 2.6)

project (Calc24_Client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -O0 -Wno-unused-variable -pthread")

#link_directories(
   # ${PROJECT_SOURCE_DIR}/lib
	#/usr/lib64/mysql/
#)

# 推荐使用 CentOS 7.0 及以上系统
# CentOS 7.0 及以上系统默认安装的是mariadb，安装方法
# yum install mariadb-server mariadb mariadb-devel
# CentOS 7.0 以下系统默认安装mysql，安装方法：
# yum install mysql-server mysql mysql-devel

#find_library(MYSQL_LIB libmysqlclient.so /usr/lib64/mysql/)
#如果你的数据库是mariadb，你还需要安装mariadb-devel库；如果你的数据库是mysql，你需要安装mysql-devel，否则会出现如下错误提示。
#IF (NOT MYSQL_LIB)
#    MESSAGE(FATAL_ERROR "mysqlclient not found. If your mysql is mariadb, please install mariadb-devel otherwise install mysql-devel.")
#ENDIF(NOT MYSQL_LIB)

set(srcs
TCPClient.cpp
main.cpp
)

add_executable(Calc24Client ${srcs})
#光包含库目录是没用的，还必须使用TARGET_LINK_LIBRARIES链接该库
#TARGET_LINK_LIBRARIES(chatserver mysqlclient)
