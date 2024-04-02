%%
% HW1
%Q1
%img read
I = imread("cameraman.tif");

% variables and macros
scale_x = 2.4;
scale_y = 1/1.4;
ang = (360-33.5)*pi/180;
scale_mat = [scale_x 0 0;0 scale_y 0;0 0 1];
rotate_mat = [cos(ang) -sin(ang) 0;sin(ang) cos(ang) 0;0 0 1];

% figure_1 showing the result when the input image is enlarged by 2.4 along
% rows, shrunk by 1.4 along columns, and rotated by 33.5 degrees clockwise
trans_mat = scale_mat * rotate_mat;
t_form_1 = maketform("AFFINE", trans_mat);
I_trans_1 = imtransform(I, t_form_1, "bicubic");
figure("name", "transformed_image"); imshow(I_trans_1);

% figure 2 howing the inverse transform of the resulting image in figure 1
trans_mat_inv = inv(trans_mat);
t_form_2 = maketform("AFFINE", trans_mat_inv);
I_trans_2 = imtransform(I_trans_1, t_form_2, "bicubic");
figure("name", "inverse spatial transform of figure 1");imshow(I_trans_2);

% figure 3 showing the difference between the original (input) image 
% and figure 2
padding_x = round((size(I_trans_2,1)-size(I,1))/2);
padding_y = round((size(I_trans_2,2)-size(I,2))/2);
temp_img = uint8(zeros(size(I_trans_2,1), size(I_trans_2, 2)));
temp_img(padding_x:padding_x+size(I,1),padding_y:padding_y+size(I,2)-1) = I;
I_diff_3 = abs(temp_img - I_trans_2);
figure("name", "difference image of figure 2 and input image"); imshow(I_diff_3);

% comment of bright and dark pixels in figure 3:
% Since figure 3 is comparing the differece between two images,
% Dark pixcel means the pixcel in the same position have similar values,
% the darker the pixcel is, the closer the values will be. Black pixcel
% means that the pixcels in the same position of two images are the same.
% if we see whole black in difference image, it means that two image are
% identical, or the pixcels/image do not change in transformation.
% bright pixcel means the pixcels in the same position of two image are
% different, the more bright it is, the more different of the two pixcels.
% Bright pixcel also means that the pixcels/image are changed during
% transformation.


%Q2
% figure 4 showing the result when the input image is spatially transformed
% using the matrix T and the bilinear interpolation, where T is specified as
% T = [0.3 0.1 0; 0.5 1.9 1; 0 0 1]
T = [0.3 0.1 0; 0.5 1.9 1; 0 0 1];
t_form_4 = maketform("AFFINE", T');
I_trans_4 = imtransform(I, t_form_4, "bilinear");
figure("name", "Transfrom by T matrix"); imshow(I_trans_4);

% inverse spatial transform to figure 4
T_inv = inv(T);
t_form_5 = maketform("AFFINE", T_inv');
I_trans_5 = imtransform(I_trans_4, t_form_5, "bilinear");
%figure("name","Inverse Spatial Transfrom by T matrix"); imshow(I_trans_5);

% difference image 
padding_x_5 = round((size(I_trans_5,1) - size(I,1))/2);
padding_y_5 = round((size(I_trans_5,2) - size(I,2))/2);
temp_img_5 = uint8(zeros(size(I_trans_5,1), size(I_trans_5,2)));
temp_img_5(padding_x_5:padding_x_5+size(I,1)-1, padding_y_5:padding_y_5+size(I,2)-1) = I;
I_diff_5 = abs(temp_img_5 - I_trans_5);
figure("name", "Difference Image of inverse spatial transformed image and input"); imshow(I_diff_5);

% comment of bright and dark pixels in figure 5:
% Since figure 5 is comparing the differece between two images,
% Dark pixel means the pixel in the same position have similar values,
% the darker the pixel is, the closer the values will be. Black pixel
% means that the pixel in the same position of two images are the same.
% if we see whole black in difference image, it means that two image are
% identical, or the pixel/image do not change in transformation.
% Bright pixel means the pixel in the same position of two image are
% different, the more bright it is, the more difference of intensity of the
% two pixels. Bright pixel also means that the pixel/image are changed during
% transformation.

%%
%Q3
I = imread("cameraman.tif");
I_1 = histeq(I);
I_2 = histeq(I_1);
figure("name","Original image");imshow(I);
figure("name","image after one time of histogram equalization"); imshow(I_1);
figure("name","image after two times of histogram equalization"); imshow(I_2);

% Histogram equalization is achieved by having a transformation function which
% can be defined to be the Cumulative Distribution Function (CDF) of a given
% Probability Density Function (PDF) of a gray-levels in a given image. 
% Then, we map pixel value to the CDF we got to redistribute the whole
% pixels. After we do this, the CDF of the transformed image will be a
% straight line.
% Then, if we do the histogram equalization again, we will also calculate
% the CDF of the transfored image which is already been a straight line.
% Then we map the pixel values to this CDF, we will get the same image as
% the transformed image.
