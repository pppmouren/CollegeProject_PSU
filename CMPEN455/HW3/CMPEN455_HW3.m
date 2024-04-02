% HW3
% Student: Xuhong Lin

%%
% Question 1
% get h(x,y)
% H = [1 1/4 1/4, 1/4 -1/2 -1/2, 1/4 -1/2 -1/2];
% h = ifft2(h, 3,3);
h = [0 1/4 0, 1/4 0 1/4, 0 1/4 0];
I = imread("cameraman.tif");
I_noise = imnoise(I,"gaussian",0,0.02);
I_filtered = imfilter(I_noise, h, "conv", "same");
figure(Name="Noised cameraman"); imshow(I_noise);
figure(Name="Applied h(x,y)"); imshow(I_filtered);

%%
%Question 2

% (1) Average Filter 
fa = 1/9*[1 1 1; 1 1 1; 1 1 1];
freqz2(fa);
axis([-1 1 -1 1]);

%%
%(2) first Central-Difference Filters
fc1 = [0 -1 0, 0 0 0, 0 1 0];
freqz2(fc1);
axis([-1 1 -1 1]);

%%
%(2) second Central-Difference Filters
fc2 = [0 0 0, -1 0 1, 0 0 0];
freqz2(fc2);
axis([-1 1 -1 1]);

%%
%(3) first Prewitt Filters
fp1 = [-1 -1 -1, 0 0 0, 1 1 1];
freqz2(fp1);
axis([-1 1 -1 1]);
%%
%(3) second Prewitt Filters
fp2 = [-1 0 1, -1 0 1, -1 0 1];
freqz2(fp2);
axis([-1 1 -1 1]);

%%
% (4) first Sober Filter
fs1 = [-1 -2 -1; 0 0 0; 1 2 1];
freqz2(fs1);
axis([-1 1 -1 1]);

%%
% (4) second Sober Filter
fs2 = [-1 0 1, -2 0 2, -1 0 1];
freqz2(fs2);
axis([-1 1 -1 1]);



