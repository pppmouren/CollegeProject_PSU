function outarray = apply_softmax(inarray)
% 
% Input:  inarray, array of size 1x1x10
%        
% Output: outarray, array of size 1x1x10

alpha = max(inarray);
outarray = zeros(1,1,10);
sum = 0;

% get the Denominador
for i = 1:size(inarray,3)
    sum = sum + exp(inarray(1,1,i)-alpha);
end

%calculate probability
for j = 1:size(inarray,3)
    outarray(1,1,j) = exp(inarray(1,1,j)-alpha)/sum;
end
end