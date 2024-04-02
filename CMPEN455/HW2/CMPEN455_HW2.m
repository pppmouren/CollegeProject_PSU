% HOMEWORK 2
% Name: Xuhong Lin
% Email: xql5448@psu.edu
% Instructor: Prof. Vishal Monga 
% Date:

%%
% Question 1

% (a) Figure 1 and the caption:
% Image sharpening using the 3x3 Laplacian filter with +8 at the center
% given by: [-1 -1 -1; -1 8 -1; -1 -1 -1]; c = +1
I = imread("cameraman.tif");
I = im2double(I);
figure("Name","original image"); imshow(I);
Lap_filter = [-1 -1 -1; -1 8 -1; -1 -1 -1];
temp_img = conv2(I, Lap_filter, "same");
c = 1;
I_1 = abs(I + c*temp_img);
%I_1 = abs(I + c*uint8(temp_img));
figure("Name", "Img Sharping Using 3x3 Laplacian Filter with +8"); 
imshow(I_1);

% (b) Figure 2 and the caption: Image processing in two steps. 
% The input image is first smoothed with the 3×3, σ2 = 0.5 Gaussian filter. 
% The resulting image is then sharpened using the 3×3 Laplacian filter
% with -4 at the center, givn by: [0 1 0; 1 -4 1; 0 1 0]. c = -1
Lap_filter = [0 1 0; 1 -4 1; 0 1 0]; 
sigma = 0.5;
c = 1;
% Gaussain smooth with 3x3 size and sigma = 0.5
h = fspecial("gaussian",[3,3],sigma);
temp_img = imfilter(I,h);
% Image sharping with center = -4
temp_img = conv2(temp_img, Lap_filter, "same");
I_2 = abs(I - c*temp_img);
%I_2 = abs(I + c*uint8(temp_img));
figure("Name","Img Smoothing with gaussian sigma = 0.5 and Sharping with -4");
imshow(I_2);

% (c) Figure 3 and the caption: Image processing in two steps. 
% The input image is first smoothed with the 3×3, σ2 = 1 Gaussian filter.
% The resulting image is then sharpened using the 3×3 Laplacian filter 
% with -8 at the center, given by: [1 1 1; 1 -8 1; 1 1 1]; c = -1
Lap_Filter = [1 1 1; 1 -8 1; 1 1 1];
sigma = 1;
c = 1;
% Gaussain smooth with 3x3 size and sigma = 1
h = fspecial("gaussian",[3,3],sigma);
temp_img = imfilter(I,h);
% Image sharping with center = -8
temp_img = conv2(temp_img, Lap_Filter, "same");
I_3 = abs(I - c*temp_img);
%I_3 = abs(I + c*uint8(temp_img));
figure("Name", "Img Smoothing with gaussian sigma = 1.0 and Sharping with -8"); 
imshow(I_3);

%%
% Question 2

% (a) 
% Load the two images
image1 = imread('cameraman.tif');
image2 = imread('woman_blonde.tif');

% Convert the images to double for processing
image1 = im2double(image1);
image2 = im2double(image2);

% Resize image2 to match the size of image1
image2 = imresize(image2, size(image1));

%rotate 180 before apply to conv2()
I_4 = conv2(image1, rot90(image2,2),"same");
figure; imshow(I_4);
I_5 = imfilter(image1,image2,'corr');
figure; imshow(I_5);
I_6 = conv2(image1, image2,"same");
% Find the maximum value in the matrix
maxValue = max(I_4(:));
maxValue5 = max(I_5(:));
maxValue6 = max(I_6(:));
% Find the (x, y) coordinates of the maximum value
[x, y] = find(I_4 == maxValue);
[x5,y5] = find(I_5 == maxValue5);
[x6,y6] = find(I_6 == maxValue6);
% Display the result
fprintf('Maximum Value: %.4f\n', maxValue);
fprintf('Coordinates of Maximum Value: (x, y) = (%d, %d)\n', x, y);
fprintf('Maximum Value of I5: %.4f\n', maxValue5);
fprintf('Coordinates of Maximum Value of I5: (x, y) = (%d, %d)\n', x5, y5);
fprintf('Maximum Value of I6: %.4f\n', maxValue6);
fprintf('Coordinates of Maximum Value of I6: (x, y) = (%d, %d)\n', x6, y6);

