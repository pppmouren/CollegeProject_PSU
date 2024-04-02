% CMPEN455 HW4
clc; close all; clear;
%% Q1 Image Denoising 
% read the image and make it double type
I = imread("cameraman.tif");
I = im2double(I);

% add the guassian noise with mean = 0 and variance = 0.02
I_noised = imnoise(I, "gaussian", 0, 0.02);
figure(Name="Noised Image with Gaussian Noise of mean: 0, variance:0.02"); 
imshow(I_noised);

% Square Average Filter
box_filter = 1/9 * [1 1 1; 1 1 1; 1 1 1];
% print image after applying Square Average Filter
I_Square = imfilter(I_noised, box_filter);
figure(Name="Square Average Filter applied"); imshow(I_Square);

% Linear Adaptive Filter 
% Variable declearation
m = size(I_noised,1);
n = size(I_noised,2);
gaussVar = 0.02;
% try to apply the linear adaptive filter to get the output image with
% windoe size of 3x3
I_Linear = zeros(m, n);
for i = 1:m-2
    for j = 1:n-2
        window = I_noised(i:i+2, j:j+2);
        arithMean = mean(mean(window));
        arithVar = sum(sum(((window - arithMean).^2)/9));
        I_Linear(i:i+2,j:j+2) = I_noised(i:i+2,j:j+2) - (gaussVar/arithVar)*(I_noised(i:i+2,j:j+2)-arithMean);
    end
end
% plot the image
figure(Name="Linear Adaptive Filter applied"); imshow(I_Linear);

% Bilateral Filter
% Inspect a patch of the image from the sky region. Compute the variance of
% the patch, which approximates the variance of the noise.
patch = imcrop(I_noised,[170, 35, 50 50]);
patchVar = std2(patch)^2;
DoS = 2*patchVar;
I_bilateral = imbilatfilt(I_noised,DoS,3);
figure(Name="Bilateral Filter Applied"); imshow(I_bilateral);


%% Question 2
B1 = [0 0 0 0 0 0 0 0 0;
      0 0 0 0 0 0 0 0 0;
      0 0 0 0 0 0 0 0 0;
      0 0 0 0.0113 0.0837 0.0113 0 0 0;
      0 0 0 0.0837 0.6187 0.0837 0 0 0;
      0 0 0 0.0113 0.0837 0.0113 0 0 0;
      0 0 0 0 0 0 0 0 0;
      0 0 0 0 0 0 0 0 0;
      0 0 0 0 0 0 0 0 0];
B2 = [0 0 0 0 0 0 0 0.0145 0;
      0 0 0 0 0 0 0.0262 0.0896 0.0145;
      0 0 0 0 0 0.0262 0.0896 0.0262 0;
      0 0 0 0 0.0262 0.0896 0.0262 0 0;
      0 0 0 0.0262 0.0896 0.0262 0 0 0;
      0 0 0.0262 0.0896 0.0262 0 0 0 0;
      0 0.0262 0.0896 0.0262 0 0 0 0 0;
      0.0145 0.0896 0.0262 0 0 0 0 0 0;
      0 0.0145 0 0 0 0 0 0 0];
% show the B1 and B2 filter
%B1_norm = B1./norm(B1);
%B2_norm = B2./norm(B2);
B1_norm = (B1 - min(B1(:)))./(max(B1(:)) - min(B1(:)));
B2_norm = (B2 - min(B2(:)))./(max(B2(:)) - min(B2(:)));
Scale_B1_norm = imresize(B1_norm, 10,"nearest");
Scale_B2_norm = imresize(B2_norm, 10,"nearest");
figure(Name="B1 blur filter"); imshow(Scale_B1_norm);
figure(Name="B2 blur filter"); imshow(Scale_B2_norm);

% Display the 2D Discrete Space Fourier Transform using freqz2
figure(Name="2D Discrete Space Fourier Transform of B1");
freqz2(B1_norm);
figure(Name="2D Discrete Space Fourier Transform of B2");
freqz2(B2_norm);

% Display the blurred version of cameraman
I_B1 = imfilter(I, B1, "conv");
I_B2 = imfilter(I, B2, "conv");
figure(Name="B1 blur filter version of cameraman.tif");
imshow(I_B1);
figure(Name="B2 blur filter verison of cameraman.tif");
imshow(I_B2);

% Adding Gaussian white noise to Blurred image
Noised_I_B1 = imnoise(I_B1, "gaussian", 0, 0.02);
Noised_I_B2 = imnoise(I_B2, "gaussian", 0, 0.02);
figure(Name="Gaussian Noised B1 blur fliter of cameraman.tif");
imshow(Noised_I_B1);
figure(Name="Gaussian Noised B2 blur filter of cameraman.tif");
imshow(Noised_I_B2);

% Apply inverse filter and weiner filter to B1 blurred image
% inverse filter of B1
Invfilter_I_B1 = deconvreg(Noised_I_B1, B1);
figure(Name="Inverse Filter applied to Noised B1-blurred cameraman"); imshow(Invfilter_I_B1);
% weiner filter of B1
nsr = gaussVar/var(I(:));
weiner_I_B1 = deconvwnr(Noised_I_B1,B1,nsr);
figure(Name="Weiner filter applied to Noised B1-blurred cameraman"); imshow(weiner_I_B1);
% inverse filter of B2
Invfilter_I_B2 = deconvreg(Noised_I_B2, B2);
figure(Name="Inverse filter applied to Noised B2-blurred cameraman"); imshow(Invfilter_I_B2);
% weiner filter of B2
weiner_I_B2 = deconvwnr(Noised_I_B2,B2,nsr);
figure(Name="Weiner filter applied to Noised B2-blurred cameraman"); imshow(weiner_I_B2);

