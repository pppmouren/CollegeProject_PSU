% Task 2 
% 2D to 3D triangulation reconstruction and error calculation
load("mocapPoints3D.mat");
PA = load("Parameters_V1_1.mat");
PB = load("Parameters_V2_1.mat");

% variable declear 
AtempPixel = [ApixelCoord; temp];
AnewWorldCoord = zeros(3,39);
BtempPixel = [BpixelCoord; temp];
BnewWorldCoord = zeros(3,39);
ResultWorldCoord = zeros(3,39);
T = (PB.Parameters.position - PA.Parameters.position)';
factor = zeros(3,39);
PLeft = zeros(3,39);
PRight = zeros(3,39);
W = zeros(3,39);
Triang_max = zeros(3,3);

% construct all 39 PL and PR vector
for i = 1:39
    PLeft(:,i) = ARmax' * (inv(AKmax)) * AtempPixel(:,i); % Pw = R^T * T^-1 * Pu
    PRight(:,i) = BRmax' * (inv(BKmax)) * BtempPixel(:,i); % Pw = R^T * T^-1 * Pu
    W(:,i) = cross(PLeft(:,i), PRight(:,i));
    % construct the matrix of PLeft, PRight, and W 
    Triang_max(:,1) = PLeft(:,i);   % Triang_max = [PLeft ; PRight; W]
    Triang_max(:,2) = PRight(:,i);
    Triang_max(:,3) = W(:,i);
    factor(:,i) = (inv(Triang_max)) * T;
end

for i = 1:39
    % get the world coordinate
    AnewWorldCoord(:,i) = (PA.Parameters.position)' + factor(1,i) * PLeft(:,i);
    BnewWorldCoord(:,i) = (PB.Parameters.position)' - factor(2,i) * PRight(:,i);

    % get the 3D point
    ResultWorldCoord(1,i) = (AnewWorldCoord(1,i) + BnewWorldCoord(1,i))/2;
    ResultWorldCoord(2,i) = (AnewWorldCoord(2,i) + BnewWorldCoord(2,i))/2;
    ResultWorldCoord(3,i) = (AnewWorldCoord(3,i) + BnewWorldCoord(3,i))/2;
end

% compute the MSE
G = ResultWorldCoord-pts3D;
G1 = G.^2;
Squared_diff = mean(mean(G1));
disp(["the mean square error is:" Squared_diff]);