function outarray = apply_maxpool(inarray)
% 
% Input:  inarray, array of size 32x32x10, 16x16x10, or 8x8x10
%        
% Output: outarray, array of size 16x16x10, 8x8x10, or 4x4x10

% Init outarray
outarray = zeros(size(inarray, 1)/2,size(inarray, 2)/2,size(inarray,3));

% Implment maxpool algorithm
for k = 1:size(inarray,3)
    for i = 1: size(inarray, 1)/2
        for j = 1:size(inarray, 2)/2
            indexed_i = (i-1)*2+1;
            indexed_j = (j-1)*2+1;
            % Get 2x2 matrix for maxpool
            temp_max = inarray(indexed_i:indexed_i+1, indexed_j:indexed_j+1,k);
            % Get the max value 
            maxval = max(max(temp_max));
            % Assign it to outarray
            outarray(i,j,k) = maxval;
        end
    end
end