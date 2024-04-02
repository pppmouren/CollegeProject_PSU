%EE454 proj2_task5 
%Get the pixel coords of two images from task 1
x1 = ApixelCoord(1,:)';
y1 = ApixelCoord(2,:)';
x2 = BpixelCoord(1,:)';
y2 = BpixelCoord(2,:)';
%Get the epipolar lines and calculate the  mean square distance
L2 = F * [x1' ; y1'; ones(size(x1'))];
for i = 1:size(L2)
    a = L2(1,i); b = L2(2,i); c=L2(3,i);
    DerrL2(i) = (a*x2(i) + b*y2(i) + c)^2/(a^2+b^2);
end

L1 = ([x2' ; y2'; ones(size(x2'))]' * F)' ;
for i = 1:size(L1)
    a = L1(1,i); b = L1(2,i); c=L1(3,i);
    DerrL1(i) = (a*x1(i) + b*y1(i) + c)^2/(a^2+b^2);
end

Derror_comb = [DerrL2,DerrL1];
Derro_final = mean(Derror_comb);
fprintf('The mean of the square geometric distances is %.1f\n',Derro_final);