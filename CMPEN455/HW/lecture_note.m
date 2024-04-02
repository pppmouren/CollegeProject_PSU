%%
I =imread("cameraman.tif");
BW1 = edge(I, "prewitt");
BW2 = edge(I, "sobel");
BW3 = edge(I, "roberts");
figure; imshow(I); figure; imshow(BW1);figure; imshow(BW2);figure; imshow(BW3);
%%
%run with different thresh hole value
%default thresh hole is about 0.2
I =imread("cameraman.tif");
BW1 = edge(I, "prewitt",0.1);
BW2 = edge(I, "sobel",0.1);
BW3 = edge(I, "roberts",0.1);
figure; imshow(I); figure; imshow(BW1);figure; imshow(BW2);figure; imshow(BW3);

%%
% LAplacian zero_crossing 
% Laplacian of Guassian
I = imread("cameraman.tif");
BW4 = edge(I,"log");
BW5 = edge(I,"zerocross");
figure; imshow(I); figure; imshow(BW4); figure; imshow(BW5);

%%
%low pass
avFilt1 = 1/9*[1 1 1; 1 1 1; 1 1 1];
freqz2(avFilt1);

%%
% low pass
avFilt2 = 1/16*[1 2 1; 2 4 2; 1 2 1];
freqz2(avFilt2);

%%
% prewitt, band pass in horizontal and low pass in vertical
filter1 = [-1, 0, 1; -1 0 1; -1 0 1];
freqz2(filter1);

%%
% Sobel, band pass in horizontal and low pass in vertical, but slightly
% difference than prewitt in y direction
filter2 = [-1 0 1; -2 0 2; -1 0 1];
freqz2(filter2);