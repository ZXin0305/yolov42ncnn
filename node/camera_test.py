#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import cv2
# 1.打开摄像头，读取摄像头拍摄到的角度
vedio_capter = cv2.VideoCapture(0)
# 2.循环不停的获取摄像头的画面，并做进一步的处理
while True:
    # 2.1获取摄像头拍摄到的画面，并做进一步的处理
    ret, frame = vedio_capter.read() # ret是指是否返回画面true和false，frame是指读取到的视频的帧
    #  2.2 通过opencv把拍摄到的画面展示出来
    frame=cv2.GaussianBlur(frame,(5,5),0) 
    cv2.imshow('Vedio', frame)
    #  2.3 按q退出while循环，即退出程序
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

vedio_capter.realease()
