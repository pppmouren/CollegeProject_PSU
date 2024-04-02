% Task 3b 
% Use triangulation to measure objects in the scene.
% Measure the 3D locations of 3 points on the wall that has white vertical stripes painted on
% it and fit a plane
load("mocapPoints3D.mat");
PA = load("Parameters_V1_1.mat");
PB = load("Parameters_V2_1.mat");

% variable section
T = (PB.Parameters.position - PA.Parameters.position)';
wallPLeft = zeros(3,3);
wallPRight = zeros(3,3);
wallW = zeros(3,3);
wallTriang_max = zeros(3,3);
wallFactor = zeros(3,3);
im1WorldCoord = zeros(3,3);
im2WorldCoord = zeros(3,3);
wallWorldCoord = zeros(3,3);

% read the image
im1 = imread('im1corrected.jpg');
im2 = imread('im2corrected.jpg');


figure(1); imagesc(im1); axis image; drawnow;
figure(2); imagesc(im2); axis image; drawnow;


figure(1); [x1,y1] = getpts;
figure(1); imagesc(im1); axis image; hold on
for i=1:3
   h=plot(x1(i),y1(i),'*'); set(h,'Color','g','LineWidth',2);
   text(x1(i),y1(i),sprintf('%d',i));
end
hold off
drawnow;


figure(2); imagesc(im2); axis image; drawnow;
[x2,y2] = getpts;
figure(2); imagesc(im2); axis image; hold on
for i=1:3
   h=plot(x2(i),y2(i),'*'); set(h,'Color','g','LineWidth',2);
   text(x2(i),y2(i),sprintf('%d',i));
end
hold off
drawnow;

% Save the points
savx1 = x1; savy1 = y1; savx2 = x2; savy2 = y2;

% constract the pixel coordiniate matrix
im1PixelCoord = ones(3,3);
im2PixelCoord = ones(3,3);

for i = 1:3
    im1PixelCoord(1,i) = savx1(i,1);
    im1PixelCoord(2,i) = savy1(i,1);
    im2PixelCoord(1,i) = savx2(i,1);
    im2PixelCoord(2,i) = savy2(i,1);
end

% construct all 3 floor PL and PR vector
for i = 1:3
    wallPLeft(:,i) = ARmax' * (inv(AKmax)) * im1PixelCoord(:,i);
    wallPRight(:,i) = BRmax' * (inv(BKmax)) * im2PixelCoord(:,i);
    wallW(:,i) = cross(wallPLeft(:,i), wallPRight(:,i));
    % construct the matrix of PLeft, PRight, and W
    wallTriang_max(:,1) = wallPLeft(:,i);
    wallTriang_max(:,2) = wallPRight(:,i);
    wallTriang_max(:,3) = wallW(:,i);
    wallFactor(:,i) = (inv(wallTriang_max)) * T;
end

for i = 1:3
    % get the world coordinate
    im1WorldCoord(:,i) = (PA.Parameters.position)' + wallFactor(1,i) * wallPLeft(:,i);
    im2WorldCoord(:,i) = (PB.Parameters.position)' - wallFactor(2,i) * wallPRight(:,i);

    % get the 3D point
    wallWorldCoord(1,i) = (im1WorldCoord(1,i) + im2WorldCoord(1,i))/2;
    wallWorldCoord(2,i) = (im1WorldCoord(2,i) + im2WorldCoord(2,i))/2;
    wallWorldCoord(3,i) = (im1WorldCoord(3,i) + im2WorldCoord(3,i))/2;
end

V1 = wallWorldCoord(:,1) - wallWorldCoord(:,2);
V2 = wallWorldCoord(:,1) - wallWorldCoord(:,3);
% gte normal vertor
N = cross(V1,V2);
% normalize it
N = N./norm(N);
% get distance
D = N(1)*wallWorldCoord(1,1) + N(2)*wallWorldCoord(2,1)+ N(3)*wallWorldCoord(3,1);
fprintf("The 3D-Plane Equation is (%.4f)x + (%.4f)y + (%.4f)z + (%.4f) = 0\n", N(1),N(2),N(3),D);