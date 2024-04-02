clc; clear; close all;

% read the image and set as double type
I = imread("cameraman.tif");
I = im2double(I);
figure; imshow(I);
% Add motion blur to the image
gaussianPSF = fspecial("motion", 25, 11);
gaussianPSF = gaussianPSF(:,10:16);
I_gaussian_blurred = imfilter(I,gaussianPSF,'conv','circular');
figure; imshow(I_gaussian_blurred); title('Motion Blurred Image')
PSNR1 = psnr(I_gaussian_blurred, I);
disp(PSNR1);
% Apply Landweber algorithm to restore the image
iteration = 100;
stepSize = 40;
I_restored = ALW(I_gaussian_blurred, gaussianPSF, stepSize,iteration);

scale = max(max(I_restored));
I_ad = I_restored/scale;
figure; imshow(I_restored); title('Restored image using Landweber Algorithm');
PSNR2 = psnr(I_restored, I);
disp(PSNR2);
