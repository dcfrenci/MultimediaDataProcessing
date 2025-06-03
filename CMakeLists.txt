cmake_minimum_required(VERSION 3.30)
project(Lab)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(Lab main.cpp
        Homeworks/Homework_2/Exercise_1/Exercise_1.cpp
        Homeworks/Homework_2/Exercise_1/Lab_version_1.cpp
        Homeworks/Homework_2/Exercise_2/Lab_version_2.cpp
        Homeworks/Homework_2/Exercise_3/Lab_version_3.cpp
        Homeworks/Homework_2/Exercise_4/Lab_version_4.cpp
        Homeworks/Homework_2/Exercise_5/Lab_version_5.cpp
        Homeworks/Homework_2/Exercise_1/Lab_version_2.cpp
        Homeworks/Homework_3/Exercise_1/Exercise_1.cpp
        Homeworks/Homework_3/Exercise_1/Lab_version_1.cpp
        Homeworks/Homework_4/Exercise_1/Exercise_1.cpp
        Homeworks/Homework_4/Exercise_1/Exercise_1_v2.cpp
        Homeworks/Homework_4/Exercise_1/Exercise_1_v1.cpp
        Homeworks/Homework_4/Exercise_1/Exercise_1_v0.cpp
        Homeworks/Homework_4/Exercise_2/Exercise_2.cpp
        Homeworks/Homework_4/Exercise_2/packbits_extension.cpp
        Homeworks/Homework_4/Exercise_3/huffman1.cpp
        Homeworks/Homework_4/Exercise_4/huffman2.cpp
        Homeworks/Homework_4/Exercise_3/Lab_version_v1.cpp
        Homeworks/Homework_5/Exercise_1/gradient.cpp
        Homeworks/Homework_5/Exercise_1/main.cpp
        Homeworks/Homework_5/Exercise_2/main.cpp
        Homeworks/Homework_5/Exercise_3/main.cpp
        Homeworks/Homework_6/Exercise_1/split.cpp
        Homeworks/Homework_6/Exercise_2/combine.cpp
        Homeworks/Homework_6/Exercise_3/huffdiff.cpp
        Homeworks/Homework_7/Inheritance/canvas_c_version.h
        Homeworks/Homework_7/Inheritance/canvas_c_version.c
        Homeworks/Homework_7/Inheritance/shape_c_version_1.c
        Homeworks/Homework_7/Inheritance/shape_c_version_2.c
        Exams/Bit_torrent/torrent.cpp
        Exams/18_Decode_HDR_image/main.cpp
        Exams/18_Decode_HDR_image/main_chat.cpp
        Exams/19_BMP/main.cpp
        Exams/19_BMP/bmp2pam_8bit.cpp
        Exams/19_BMP/bmp2pam_24bit.cpp
        Exams/19_BMP/bmp2pam_4.cpp
        Exams/19_BMP/bmp2pam_1bit.cpp
        Exams/14_Z85rot/z85rot.cpp
        Exams/TIFF/main.cpp
        Exams/YUV4MPEG2/pgm.cpp
        Exams/YUV4MPEG2/ppm.cpp
        Exams/YUV4MPEG2/y4m_gray.cpp
        Exams/YUV4MPEG2/y4m_color.cpp
        Exams/MDCT/mdct.cpp
        Exams/MKW/main.cpp
)

target_link_libraries(Lab PRIVATE stdc++exp)