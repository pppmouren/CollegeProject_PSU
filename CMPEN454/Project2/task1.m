%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Project:     stereo vision project (CMPEN454 Project 1)
%
% Overview:    The goal will to be to use the tools we have developed 
%              through the last few lectures by considering stereo image 
%              pairs coupled with motion capture data. You%ll be given a 
%              pair of images taken from different cameras simultaneously.
%              The person in the image is tagged with motion capture markers
%              which were used to get accurate 3D point measurements. 
%              In this scenario you have all of the intrinsic and extrinsic 
%              camera information, so you ll be able to explore the 3D 
%              scene using the methods we have developed and use the motion 
%              capture data as a ground truth.
%
% Authors:    Jianwu Tan, Jie Chen, Qiaoxu Cui, Xuhong Lin
% Instructor: Prof. Kenneth D. Czuprynski
% Date:       11/06/2023
% Attribute: 
%             Every contribute their toughts on each task, and below is
%             the coding distribution.
%             task1.m:   Jianwu Tan
%             task2.m:   Xuhong Lin     
%             task3a.m:  Xuhong Lin    
%             task4.m:   Qiaoxu Cui  
%             task5.m:   Jie Chen
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



clear all; close all; clc;
load("mocapPoints3D.mat");
PA = load("Parameters_V1_1.mat");
PB = load("Parameters_V2_1.mat");

 
% Data Section
ARmax = PA.Parameters.Rmat;
APmax = PA.Parameters.Pmat;
A_t = APmax(:,4);
AKmax = PA.Parameters.Kmat;
AtempWorld = zeros(4,1);
AtempPixel = zeros(3,39);
ApixelCoord = zeros(2,39);
BRmax = PB.Parameters.Rmat;
BPmax = PB.Parameters.Pmat;
B_t = BPmax(:,4);
BKmax = PB.Parameters.Kmat;
BtempWorld = zeros(4,1);
BtempPixel = zeros(3,39);
BpixelCoord = zeros(2,39);

% Task 1
% Transfrom 3D to 2D pixel point
temp = ones(1,39);
AworldCoord = [pts3D;temp];

% image 1 pinhold camera model
for i = 1:39
    AtempWorld = AworldCoord(:,i);
    AtempPixel(:,i) = AKmax*APmax*AtempWorld; % pinhole camera matrix

    % Divide by Z value and store it 
    ApixelCoord(1,i) = AtempPixel(1,i)/AtempPixel(3,i);
    ApixelCoord(2,i) = AtempPixel(2,i)/AtempPixel(3,i);
end
% Lie the points on the image
figure(1)
I1 = imread("im1corrected.jpg");
imagesc(I1);
hold on;
plot(ApixelCoord(1,:),ApixelCoord(2,:),'m*')
title("image1-3D to 2D pixel locations")

% PB
temp = ones(1,39);
BworldCoord = [pts3D;temp];

for i = 1:39
    BtempWorld = BworldCoord(:,i);
    BtempPixel(:,i) = BKmax*BPmax*BtempWorld;
    BpixelCoord(1,i) = BtempPixel(1,i)/BtempPixel(3,i);
    BpixelCoord(2,i) = BtempPixel(2,i)/BtempPixel(3,i);
end

% Lie the points on the image
figure(2)
I2 = imread("im2corrected.jpg");
imagesc(I2);
hold on;
plot(BpixelCoord(1,:),BpixelCoord(2,:),'m*')
title("image2-3D to 2D pixel locations")












