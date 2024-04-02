function outarray = apply_relu(inarray)
% 
% Input:  inarray, array of size 32x32x10, 16x16x10, or 8x8x10
%        
% Output: outarray, array of size 32x32x10, 16x16x10, or 8x8x10

% Init outarray
outarray = zeros(size(inarray,1),size(inarray,2),size(inarray,3));
% Implement RELU
for k = 1:size(inarray,3)
    for i = 1:size(inarray,1)
        for j = 1:size(inarray,2)
            if inarray(i, j, k)< 0
                inarray(i, j, k)=0;
            end
            outarray(i, j, k) = inarray(i, j, k);
           
        end
    end
end 