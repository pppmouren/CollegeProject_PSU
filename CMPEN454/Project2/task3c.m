% task 3c

% IMPORTANT: Pick 5 points in each images. The order of the points have to
% picked in order. The first point should located in the head of the human, the second
% point should located in the feet of the human. The third point should
% located in the top of the doorway, the forth point should located in the
% floor of the doorway. And the fifth point should located at the center of
% the camera that can be saw in both image.

%data section 
im1PixelCoord = ones(3,5);
im2PixelCoord = ones(3,5);
im1WorldCoord = zeros(3,5);
im2WorldCoord = zeros(3,5);
humanWorldCoord = zeros(3,2);
doorWorldCoord = zeros(3,2);
cameraWorldCoord = zeros(3,1);
T = (PB.Parameters.position - PA.Parameters.position)';
T3_PLeft = zeros(3,5);
T3_PRight = zeros(3,5);
T3_W = zeros(3,5);
T3_Triang_max = zeros(3,3);
T3_Factor = zeros(3,5);


im1 = imread('im1corrected.jpg');
im2 = imread('im2corrected.jpg');


figure(1); imagesc(im1); axis image; drawnow;
figure(2); imagesc(im2); axis image; drawnow;


figure(1); [x1,y1] = getpts;
figure(1); imagesc(im1); axis image; hold on
for i=1:5
   h=plot(x1(i),y1(i),'*'); set(h,'Color','g','LineWidth',2);
   text(x1(i),y1(i),sprintf('%d',i));
end
hold off
drawnow;


figure(2); imagesc(im2); axis image; drawnow;
[x2,y2] = getpts;
figure(2); imagesc(im2); axis image; hold on
for i=1:5
   h=plot(x2(i),y2(i),'*'); set(h,'Color','g','LineWidth',2);
   text(x2(i),y2(i),sprintf('%d',i));
end
hold off
drawnow;

% Save the points
savx1 = x1; savy1 = y1; savx2 = x2; savy2 = y2;


for i = 1:5
    im1PixelCoord(1,i) = savx1(i,1);
    im1PixelCoord(2,i) = savy1(i,1);
    im2PixelCoord(1,i) = savx2(i,1);
    im2PixelCoord(2,i) = savy2(i,1);
end

% construct all 3 floor PL and PR vector
for i = 1:5
    T3_PLeft(:,i) = ARmax' * (inv(AKmax)) * im1PixelCoord(:,i);
    T3_PRight(:,i) = BRmax' * (inv(BKmax)) * im2PixelCoord(:,i);
    T3_W(:,i) = cross(T3_PLeft(:,i), T3_PRight(:,i));
    % construct the matrix of PLeft, PRight, and W
    T3_Triang_max(:,1) = T3_PLeft(:,i);
    T3_Triang_max(:,2) = T3_PRight(:,i);
    T3_Triang_max(:,3) = T3_W(:,i);
    T3_Factor(:,i) = (inv(T3_Triang_max)) * T;
end

for i = 1:5
    % get the world coordinate
    im1WorldCoord(:,i) = (PA.Parameters.position)' + T3_Factor(1,i) * T3_PLeft(:,i);
    im2WorldCoord(:,i) = (PB.Parameters.position)' - T3_Factor(2,i) * T3_PRight(:,i);

    % get the 3D point
    if i <= 2
        humanWorldCoord(1,i) = (im1WorldCoord(1,i) + im2WorldCoord(1,i))/2;
        humanWorldCoord(2,i) = (im1WorldCoord(2,i) + im2WorldCoord(2,i))/2;
        humanWorldCoord(3,i) = (im1WorldCoord(3,i) + im2WorldCoord(3,i))/2;
    elseif i <=4
        doorWorldCoord(1,i-2) = (im1WorldCoord(1,i) + im2WorldCoord(1,i))/2;
        doorWorldCoord(2,i-2) = (im1WorldCoord(2,i) + im2WorldCoord(2,i))/2;
        doorWorldCoord(3,i-2) = (im1WorldCoord(3,i) + im2WorldCoord(3,i))/2;
    else
        cameraWorldCoord(1,i-4) = (im1WorldCoord(1,i) + im2WorldCoord(1,i))/2;
        cameraWorldCoord(2,i-4) = (im1WorldCoord(2,i) + im2WorldCoord(2,i))/2;
        cameraWorldCoord(3,i-4) = (im1WorldCoord(3,i) + im2WorldCoord(3,i))/2;
    end 
end


humanheight = humanWorldCoord(3,1) - humanWorldCoord(3,2);
doorheight = doorWorldCoord(3,1) - doorWorldCoord(3,2);
fprintf("the human height is %.4f mm.\n", humanheight);
fprintf("the doorway height is %.4f mm.\n", doorheight);
fprintf("the world coordinate of the center of the camera is (%.4f, %.4f, %.4f)\n", cameraWorldCoord(1,1), cameraWorldCoord(2,1), cameraWorldCoord(3,1));



